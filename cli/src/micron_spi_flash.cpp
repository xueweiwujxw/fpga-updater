/**
 * @file micron_spi_flash.cpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @date 2025-01-07
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <unistd.h>
#include <sstream>
#include <iomanip>

#include <micron_spi_flash.hpp>
#include <spi_nor.h>

using namespace micronnp;
using namespace std;

constexpr char WRITE_ENABLE_ERR_MSG[] = "set write enable failed.\t";
constexpr char WRITE_DISABLE_ERR_MSG[] = "set write disable failed.\t";

constexpr char RESET_ENABLE_ERR_MSG[] = "set reset enable failed.\t";
constexpr char RESET_MEM_ERR_MSG[] = "set reset memory failed.\t";

constexpr char EN4B_ERR_MSG[] = "enter 4 bytes mode failed.\t";
constexpr char EX4B_ERR_MSG[] = "quit 4 bytes mode failed.\t";

constexpr char RDSR_OP_ERR_MSG[] = "read status register send op failed.\t";
constexpr char RDSR_RDREQ_ERR_MSG[] = "read status register request failed.\t";
constexpr char RDSR_RD_ERR_MSG[] = "read status register data failed.\t";

constexpr char RDFSR_OP_ERR_MSG[] = "read flag status register send op failed.\t";
constexpr char RDFSR_RDREQ_ERR_MSG[] = "read flag status register request failed.\t";
constexpr char RDFSR_RD_ERR_MSG[] = "read flag status register data failed.\t";

constexpr char RD_OP_ERR_MSG[] = "read send op failed.\t";
constexpr char RD_ADDR_ERR_MSG[] = "read send addr failed.\t";
constexpr char RD_RDREQ_ERR_MSG[] = "read request failed.\t";
constexpr char RD_RD_ERR_MSG[] = "read data failed.\t";

constexpr char RDID_OP_ERR_MSG[] = "read id send op failed.\t";
constexpr char RDID_RDREQ_ERR_MSG[] = "read id request failed.\t";
constexpr char RDID_RD_ERR_MSG[] = "read id data failed.\t";

constexpr char BE_OP_ERR_MSG[] = "block erase send op failed.\t";
constexpr char BE_ADDR_ERR_MSG[] = "block erase addr failed.\t";
constexpr char BE_REFUSE_ERR_MSG[] = "block erase was refused.\t";
constexpr char BE_TIMEOUT_ERR_MSG[] = "block erase operation timeout.\t";
constexpr char BE_RES_ERR_MSG[] = "block erase failure or protecion error.\t";

constexpr char PP_OP_ERR_MSG[] = "program send op failed.\t";
constexpr char PP_ADDR_ERR_MSG[] = "program addr failed.\t";
constexpr char PP_WD_ERR_MSG[] = "program data failed.\t";
constexpr char PP_REFUSE_ERR_MSG[] = "program was refused.\t";
constexpr char PP_TIMEOUT_ERR_MSG[] = "program operation timeout.\t";
constexpr char PP_RES_ERR_MSG[] = "program failure or protecion error.\t";

#ifndef IS_ERR
#define IS_ERR(x) ((x) != 0)
#endif

#ifndef IS_DATA_ERR
#define IS_DATA_ERR(x) ((x) < 0)
#endif

#ifndef THROW_IF_ERR
#define THROW_IF_ERR(code, msg)                                      \
    if (IS_ERR(code)) {                                              \
        throw std::runtime_error(std::string(msg) + strerror(code)); \
    }
#endif

#ifndef THROW_IF_DATA_ERR
#define THROW_IF_DATA_ERR(code, msg)                                 \
    if (IS_DATA_ERR(code)) {                                         \
        throw std::runtime_error(std::string(msg) + strerror(code)); \
    }
#endif

string micron_spi_flash_device_id::display() {
    char output[256] = {0};

    int offset = snprintf(output, sizeof(output), "Manufacture ID: 0x%02x\n", this->manufacture_id);

    offset += snprintf(output + offset, sizeof(output) - offset, "Device ID: 0x");
    for (size_t i = 0; i < sizeof(this->device_id); ++i)
        offset += snprintf(output + offset, sizeof(output) - offset, "%02x", this->device_id[i]);
    offset += snprintf(output + offset, sizeof(output) - offset, "\n");

    offset += snprintf(output + offset, sizeof(output) - offset, "UID: 0x");
    for (size_t i = 0; i < sizeof(this->uid); ++i)
        offset += snprintf(output + offset, sizeof(output) - offset, "%02x", this->uid[i]);

    return string(output);
}

bool micron_spi_flash::check_write_in_progress_with_reg(const uint8_t &reg) {
    return (reg & SR_WIP) != 0;
}

bool micron_spi_flash::check_write_enable_latch_with_reg(const uint8_t &reg) {
    return (reg & SR_WEL) != 0;
}

bool micron_spi_flash::check_device_ready_with_reg(const uint8_t &reg) {
    return (reg & FSR_READY) != 0;
}

bool micron_spi_flash::check_erase_clear_with_reg(const uint8_t &reg) {
    return (reg & FSR_E_ERR) == 0;
}

bool micron_spi_flash::check_program_clear_with_reg(const uint8_t &reg) {
    return (reg & FSR_P_ERR) == 0;
}

bool micron_spi_flash::check_protection_clear_with_reg(const uint8_t &reg) {
    return (reg & FSR_PT_ERR) == 0;
}

bool micron_spi_flash::probe_flash() {
    return axispi_select(this->spi_t, 0) == 0;
}

bool micron_spi_flash::remove_flash() {
    return axispi_diselect(this->spi_t, 0) == 0;
}

void micron_spi_flash::write_enable() {
    try {
        return this->invoke_pre_post([&] {
            auto ret = axispi_write(this->spi_t, SPINOR_OP_WREN);
            THROW_IF_ERR(ret, WRITE_ENABLE_ERR_MSG);
        });
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        throw;
    }
}

void micron_spi_flash::write_disable() {
    try {
        return this->invoke_pre_post([&] {
            auto ret = axispi_write(this->spi_t, SPINOR_OP_WRDI);
            THROW_IF_ERR(ret, WRITE_DISABLE_ERR_MSG);
        });
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        throw;
    }
}

micron_spi_flash::micron_spi_flash() : initialize_stat(false), mode_4b(false), op_timeout_s(0) {}

micron_spi_flash::~micron_spi_flash() {
    axispi_ctrl_free(this->spi_t);
}

bool micron_spi_flash::initialized() {
    return this->initialize_stat.load();
}

bool micron_spi_flash::init(void *addr, uint16_t timeout_ms, uint16_t op_timeout_s) {
    if (addr == nullptr)
        return false;

    axispi_config_t spi_config = {
        .cpol = 0,
        .cpha = 0,
        .sclk_toggle = 7,
        .ss_setup = 7,
        .ss_hold = 7,
        .ss_disable = 7,
        .sample_pos = 2,
        .timout_ms = timeout_ms,
        .continuous = false,
        .ss_width = 1,
    };

    this->spi_t = axispi_ctrl_create(addr);
    if (this->spi_t == nullptr) {
        logf_err("create device failed.\n");
        return false;
    }

    int ret = axispi_ctrl_init(this->spi_t, spi_config);
    if (ret != 0) {
        logf_err("%s\n", strerror(ret));
        axispi_ctrl_free(this->spi_t);
        return false;
    }

    this->initialize_stat = true;
    this->op_timeout_s = op_timeout_s;

    try {
        this->init_flash();
        return true;
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        return false;
    }
}

void micron_spi_flash::init_flash() {
    try {
        return this->invoke_pre_post([&] {
            auto ret = axispi_write(this->spi_t, SPINOR_OP_RST_EN);
            THROW_IF_ERR(ret, RESET_ENABLE_ERR_MSG);
            ret = axispi_write(this->spi_t, SPINOR_OP_RST_MEM);
            THROW_IF_ERR(ret, RESET_MEM_ERR_MSG);
        });
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        throw;
    }
}

void micron_spi_flash::enter_4byte_mode() {
    try {
        return this->invoke_pre_post([&] {
            auto ret = axispi_write(this->spi_t, SPINOR_OP_EN4B);
            THROW_IF_ERR(ret, EN4B_ERR_MSG);
            this->mode_4b = true;
        });
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        throw;
    }
}

void micron_spi_flash::quit_4byte_mode() {
    try {
        return this->invoke_pre_post([&] {
            auto ret = axispi_write(this->spi_t, SPINOR_OP_EX4B);
            THROW_IF_ERR(ret, EX4B_ERR_MSG);
            this->mode_4b = false;
        });
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        throw;
    }
}

uint8_t micron_spi_flash::read_status_register() {
    try {
        return this->invoke_pre_post([&] {
            auto ret = axispi_write(this->spi_t, SPINOR_OP_RDSR);
            THROW_IF_ERR(ret, RDSR_OP_ERR_MSG);

            ret = axispi_read_request(this->spi_t);
            THROW_IF_ERR(ret, RDSR_RDREQ_ERR_MSG);

            ret = axispi_read(this->spi_t);
            THROW_IF_DATA_ERR(ret, RDSR_RD_ERR_MSG);

            return static_cast<uint8_t>(ret & 0xff);
        });
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        throw;
    }
}

uint8_t micron_spi_flash::read_flag_status_register() {
    try {
        return this->invoke_pre_post([&] {
            auto ret = axispi_write(this->spi_t, SPINOR_OP_RDFSR);
            THROW_IF_ERR(ret, RDFSR_OP_ERR_MSG);

            ret = axispi_read_request(this->spi_t);
            THROW_IF_ERR(ret, RDFSR_RDREQ_ERR_MSG);

            ret = axispi_read(this->spi_t);
            THROW_IF_DATA_ERR(ret, RDFSR_RD_ERR_MSG);

            return static_cast<uint8_t>(ret & 0xff);
        });
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        throw;
    }
}

void micron_spi_flash::read_data(uint32_t addr, uint8_t *data, uint16_t length) {
    try {
        return this->invoke_pre_post([&] {
            if (length > READ_SIZE)
                throw runtime_error(string("two much read request length ") + to_string(length));

            uint8_t read_data[READ_SIZE] = {0};
            uint8_t read_op = this->mode_4b ? SPINOR_OP_READ_4B : SPINOR_OP_READ;
            int addr_bytes_num = this->mode_4b ? 4 : 3;
            uint8_t *addr_bytes = reinterpret_cast<uint8_t *>(&addr);

            auto ret = axispi_write(this->spi_t, read_op);
            THROW_IF_ERR(ret, RD_OP_ERR_MSG);

            for (int i = 0; i < addr_bytes_num; ++i) {
                ret = axispi_write(this->spi_t, addr_bytes[addr_bytes_num - i - 1]);
                THROW_IF_ERR(ret, RD_ADDR_ERR_MSG);
            }

            for (int i = 0; i < length; ++i) {
                ret = axispi_read_request(this->spi_t);
                THROW_IF_ERR(ret, RD_RDREQ_ERR_MSG);
            }

            for (int i = 0; i < length; ++i) {
                ret = axispi_read(this->spi_t);

                THROW_IF_DATA_ERR(ret, RD_RD_ERR_MSG);
                read_data[i] = static_cast<uint8_t>(ret);
            }

            memcpy(data, read_data, length);
        });
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        throw;
    }
}

micron_spi_flash_device_id micron_spi_flash::read_id() {
    try {
        return this->invoke_pre_post([&] {
            micron_spi_flash_device_id dev_id;
            auto ret = axispi_write(this->spi_t, SPINOR_OP_RDID);
            THROW_IF_ERR(ret, RDID_OP_ERR_MSG);

            for (size_t i = 0; i < sizeof(dev_id); ++i) {
                ret = axispi_read_request(this->spi_t);
                THROW_IF_ERR(ret, RDID_RDREQ_ERR_MSG);
            }

            uint8_t data_buf[sizeof(micron_spi_flash_device_id)] = {0};

            for (size_t i = 0; i < sizeof(dev_id); ++i) {
                ret = axispi_read(this->spi_t);
                THROW_IF_DATA_ERR(ret, RDID_RD_ERR_MSG);
                data_buf[i] = static_cast<uint8_t>(ret);
            }

            memcpy(&dev_id, data_buf, sizeof(dev_id));
            return dev_id;
        });
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        throw;
    }
}

void micron_spi_flash::write_data(uint32_t addr, uint8_t *data, uint16_t length) {
    try {
        this->write_enable();

        auto status = this->read_status_register();
        auto flag = this->read_flag_status_register();

        if (!this->check_write_enable_latch_with_reg(status))
            THROW_IF_ERR(-EINVAL, PP_REFUSE_ERR_MSG);
        if (!this->check_device_ready_with_reg(flag))
            THROW_IF_ERR(-EBUSY, PP_REFUSE_ERR_MSG);

        this->invoke_pre_post([&] {
            uint8_t erase_op = this->mode_4b ? SPINOR_OP_PP : SPINOR_OP_PP_4B;
            int addr_bytes_num = this->mode_4b ? 4 : 3;
            uint8_t *addr_bytes = reinterpret_cast<uint8_t *>(&addr);

            auto ret = axispi_write(this->spi_t, erase_op);
            THROW_IF_ERR(ret, PP_OP_ERR_MSG);

            for (int i = 0; i < addr_bytes_num; ++i) {
                ret = axispi_write(this->spi_t, addr_bytes[addr_bytes_num - i - 1]);
                THROW_IF_ERR(ret, PP_ADDR_ERR_MSG);
            }

            for (int i = 0; i < length; ++i) {
                ret = axispi_write(this->spi_t, data[i]);
                THROW_IF_ERR(ret, PP_WD_ERR_MSG);
            }
        });

        // wait
        unsigned int wait_cnt = this->op_timeout_s * 1000000 / wait_period_us;
        while (wait_cnt-- > 0) {
            flag = this->read_flag_status_register();
            if (this->check_device_ready_with_reg(flag))
                break;
            usleep(wait_period_us);
        }
        if (wait_cnt == 0)
            THROW_IF_ERR(-ETIMEDOUT, PP_TIMEOUT_ERR_MSG);

        flag = this->read_flag_status_register();
        if (!this->check_program_clear_with_reg(flag) || !this->check_protection_clear_with_reg(flag))
            THROW_IF_ERR(-EINVAL, PP_RES_ERR_MSG);

        this->write_disable();

    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        throw;
    }
}

void micron_spi_flash::erase_4k_block(uint32_t addr) {
    try {
        this->write_enable();

        auto status = this->read_status_register();
        auto flag = this->read_flag_status_register();

        if (!this->check_write_enable_latch_with_reg(status))
            THROW_IF_ERR(-EINVAL, BE_REFUSE_ERR_MSG);
        if (!this->check_device_ready_with_reg(flag))
            THROW_IF_ERR(-EBUSY, BE_REFUSE_ERR_MSG);

        this->invoke_pre_post([&] {
            uint8_t erase_op = this->mode_4b ? SPINOR_OP_BE_4K : SPINOR_OP_BE_4K_4B;
            int addr_bytes_num = this->mode_4b ? 4 : 3;
            uint8_t *addr_bytes = reinterpret_cast<uint8_t *>(&addr);

            auto ret = axispi_write(this->spi_t, erase_op);
            THROW_IF_ERR(ret, BE_OP_ERR_MSG);

            for (int i = 0; i < addr_bytes_num; ++i) {
                ret = axispi_write(this->spi_t, addr_bytes[addr_bytes_num - i - 1]);
                THROW_IF_ERR(ret, BE_ADDR_ERR_MSG);
            }
        });

        // wait
        unsigned int wait_cnt = this->op_timeout_s * 1000000 / wait_period_us;
        while (wait_cnt-- > 0) {
            flag = this->read_flag_status_register();
            if (this->check_device_ready_with_reg(flag))
                break;
            usleep(wait_period_us);
        }
        if (wait_cnt == 0)
            THROW_IF_ERR(-ETIMEDOUT, BE_TIMEOUT_ERR_MSG);

        flag = this->read_flag_status_register();
        if (!this->check_erase_clear_with_reg(flag) || !this->check_protection_clear_with_reg(flag))
            THROW_IF_ERR(-EINVAL, BE_RES_ERR_MSG);

        this->write_disable();

    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        throw;
    }
}

void micron_spi_flash::erase_32k_block(uint32_t addr) {
    try {
        this->write_enable();

        auto status = this->read_status_register();
        auto flag = this->read_flag_status_register();

        if (!this->check_write_enable_latch_with_reg(status))
            THROW_IF_ERR(-EINVAL, BE_REFUSE_ERR_MSG);
        if (!this->check_device_ready_with_reg(flag))
            THROW_IF_ERR(-EBUSY, BE_REFUSE_ERR_MSG);

        this->invoke_pre_post([&] {
            uint8_t erase_op = this->mode_4b ? SPINOR_OP_BE_32K : SPINOR_OP_BE_32K_4B;
            int addr_bytes_num = this->mode_4b ? 4 : 3;
            uint8_t *addr_bytes = reinterpret_cast<uint8_t *>(&addr);

            auto ret = axispi_write(this->spi_t, erase_op);
            THROW_IF_ERR(ret, BE_OP_ERR_MSG);

            for (int i = 0; i < addr_bytes_num; ++i) {
                ret = axispi_write(this->spi_t, addr_bytes[addr_bytes_num - i - 1]);
                THROW_IF_ERR(ret, BE_ADDR_ERR_MSG);
            }
        });

        // wait
        unsigned int wait_cnt = this->op_timeout_s * 1000000 / wait_period_us;
        while (wait_cnt-- > 0) {
            flag = this->read_flag_status_register();
            if (this->check_device_ready_with_reg(flag))
                break;
            usleep(wait_period_us);
        }
        if (wait_cnt == 0)
            THROW_IF_ERR(-ETIMEDOUT, BE_TIMEOUT_ERR_MSG);

        flag = this->read_flag_status_register();
        if (!this->check_erase_clear_with_reg(flag) || !this->check_protection_clear_with_reg(flag))
            THROW_IF_ERR(-EINVAL, BE_RES_ERR_MSG);

        this->write_disable();

    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        throw;
    }
}

void micron_spi_flash::erase_sector_block(uint32_t addr) {
    try {
        this->write_enable();

        auto status = this->read_status_register();
        auto flag = this->read_flag_status_register();

        if (!this->check_write_enable_latch_with_reg(status))
            THROW_IF_ERR(-EINVAL, BE_REFUSE_ERR_MSG);
        if (!this->check_device_ready_with_reg(flag))
            THROW_IF_ERR(-EBUSY, BE_REFUSE_ERR_MSG);

        this->invoke_pre_post([&] {
            uint8_t erase_op = this->mode_4b ? SPINOR_OP_SE : SPINOR_OP_SE_4B;
            int addr_bytes_num = this->mode_4b ? 4 : 3;
            uint8_t *addr_bytes = reinterpret_cast<uint8_t *>(&addr);

            auto ret = axispi_write(this->spi_t, erase_op);
            THROW_IF_ERR(ret, BE_OP_ERR_MSG);

            for (int i = 0; i < addr_bytes_num; ++i) {
                ret = axispi_write(this->spi_t, addr_bytes[addr_bytes_num - i - 1]);
                THROW_IF_ERR(ret, BE_ADDR_ERR_MSG);
            }
        });

        // wait
        unsigned int wait_cnt = this->op_timeout_s * 1000000 / wait_period_us;
        while (wait_cnt-- > 0) {
            flag = this->read_flag_status_register();
            if (this->check_device_ready_with_reg(flag))
                break;
            usleep(wait_period_us);
        }
        if (wait_cnt == 0)
            THROW_IF_ERR(-ETIMEDOUT, BE_TIMEOUT_ERR_MSG);

        flag = this->read_flag_status_register();
        if (!this->check_erase_clear_with_reg(flag) || !this->check_protection_clear_with_reg(flag))
            THROW_IF_ERR(-EINVAL, BE_RES_ERR_MSG);

        this->write_disable();

    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        throw;
    }
}
