/**
 * @file update_ctrl.cpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @date 2024-09-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <fstream>
#include <nlohmann/json.hpp>

#include <update_ctrl.hpp>
#include <pcie_slot.hpp>
#include <log.h>
#include <mcs_parser.hpp>
#include <tools.hpp>

using namespace update_np;
using namespace xdma_np;
using namespace nlohmann;
using namespace interface;
using namespace std;
using namespace tools_np;
using namespace micronnp;

update_ctrl::update_ctrl() {}

update_ctrl::~update_ctrl() {
    this->xdma.deinit();
}

bool update_ctrl::init(std::string slot_alias, int timeout) {
    try {
        // 读取插槽，扫描设备
        auto slots = scan_slots();
        auto devices = scan_devices();

        // 读取预定义设备映射
        json slot_config;
        ifstream ifs("/env/slot.json");
        slot_config = json::parse(ifs);
        ifs.close();

        // 读取设备id和bus名称
        if (!slot_config.contains(slot_alias)) {
            logf_err("invalid alias %s\n", slot_alias.c_str());
            return false;
        }

        auto slot_iter = slots.find(slot_config[slot_alias].get<int>());
        if (slot_iter == slots.end()) {
            logf_err("can not find slot id %d\n", slot_config[slot_alias].get<int>());
            return false;
        }
        string bus = slot_iter->second;

        auto device_iter = devices.find(bus);
        if (device_iter == devices.end()) {
            logf_err("can not find devices %s\n", bus.c_str());
            return false;
        }
        int device_id = device_iter->second;

        if (this->xdma.init(device_id, bus) != 0) {
            logf_err("xdma init failed.\n");
            return false;
        }

        auto addr = this->xdma.register_map(0x80090000L, 4 * KiB, device_id);
        if (addr == nullptr) {
            this->xdma.deinit();
            return false;
        }
        if (!this->spi_flash.init(addr, timeout)) {
            this->xdma.deinit();
            return false;
        }
        return true;
    } catch (exception &e) {
        logf_err("%s\n", e.what());
        return false;
    }
}

bool update_ctrl::update(std::string mcs_file_path, std::string prm_file_path) {
    auto mcs_result = check_valid(mcs_file_path);
    logf_info("check result: %s, bytes: %ld\n", mcs_result.valid ? "true" : "false", mcs_result.bytes);
    if (!mcs_result.valid)
        return false;

    auto prm_result = parse_prm(prm_file_path);
    if (prm_result.size() == 0) {
        logf_err("partition is zero or parse prm file failed.\n");
        return false;
    }

    MCSFragment fragment;
    uint8_t verify_buffer[READ_SIZE] = {0};
    ifstream file(mcs_file_path);
    if (!file.is_open()) {
        logf_err("open %s failed.\n", mcs_file_path.c_str());
        return false;
    }

    bool write_verify_fail = false;
    size_t index = 0;

    try {
        auto id = this->spi_flash.read_id();
        logf_info("\n%s\n", id.display().c_str());
    } catch (const std::exception &e) {
        logf_err("read flash device id info failed. %s\n", e.what());
        return false;
    }

    try {
        this->spi_flash.enter_4byte_mode();
    } catch (const std::exception &e) {
        logf_err("flash enter 4 byte mode failed. %s\n", e.what());
        return false;
    }

    while (index < prm_result.size()) {
        printf("================ start program partition %ld.\n", index);
        auto cur_partition = prm_result[index];
        cur_partition.print();
        uint32_t partition_size = cur_partition.size;
        uint32_t tot_size = partition_size;
        uint32_t partition_addr = cur_partition.addr_start;
        int cnt = 0;
        while (!file.eof() && partition_size > 0) {
            size_t fragment_size = partition_size > sizeof(fragment.buffer) ? sizeof(fragment.buffer) : partition_size;
            auto ret = get_mcs_fragment(file, fragment.buffer, sizeof(fragment.buffer), fragment_size);
            if (ret.second != -1) {
                fragment.addr = ret.first;
                fragment.bytes = ret.second;
                logf_info("start program fragment: %d addr: %08x, size: %ld\n", cnt + 1, partition_addr, fragment.bytes);

                // erase
                try {
                    this->spi_flash.erase_sector_block(partition_addr);
                    logf_info("erase addr %08x succeed.\n", partition_addr);
                } catch (const std::exception &e) {
                    write_verify_fail = true;
                    logf_err("erase addr %08x failed. %s\n", partition_addr, e.what());
                    break;
                }

                // write and verify
                uint32_t target_addr = partition_addr;
                uint32_t target_tot_len = fragment.bytes;
                uint32_t target_len = target_tot_len > WRITE_SIZE ? WRITE_SIZE : target_tot_len;
                uint32_t target_offset = 0;

                while (target_tot_len > 0) {
                    try {
                        this->spi_flash.write_data(target_addr, fragment.buffer + target_offset, target_len);
                    } catch (const std::exception &e) {
                        write_verify_fail = true;
                        logf_err("write target addr: %u, target len: %u, target offset: %u failed. %s\n", target_addr, target_len, target_offset, e.what());
                        break;
                    }

                    try {
                        this->spi_flash.read_data(target_addr, verify_buffer, target_len);
                    } catch (const std::exception &e) {
                        write_verify_fail = true;
                        logf_err("read target addr: %u, target len: %u, target offset: %u failed. %s\n", target_addr, target_len, target_offset, e.what());
                        break;
                    }

                    auto cmpres = memcmp(verify_buffer, fragment.buffer + target_offset, target_len);

                    if (cmpres != 0) {
                        write_verify_fail = true;
                        logf_err("verify target addr: %08x, target len: %u, target offset: %u failed, result: %d.\n", target_addr, target_len, target_offset, cmpres);
                        logf_info("read data\n");
                        display_hex(verify_buffer, target_len);
                        printf("\n");
                        logf_info("write data\n");
                        display_hex(fragment.buffer + target_offset, target_len);
                        break;
                    }

                    target_tot_len -= target_len;
                    target_addr = target_addr + target_len;
                    target_offset += target_len;
                    target_len = target_tot_len > WRITE_SIZE ? WRITE_SIZE : target_tot_len;
                }

                if (write_verify_fail)
                    break;

                partition_size -= ret.second;
                logf_info("program fragment: %d addr: %08x, size: %ld succeed, left bytes: %u %.2lf%%\n\n", cnt + 1,
                          partition_addr,
                          fragment.bytes,
                          partition_size,
                          static_cast<double>(partition_size) / static_cast<double>(tot_size) * 100.0);
                partition_addr += fragment.bytes;
                cnt++;
            }
        }
        if (write_verify_fail)
            break;
        printf("================ finish program partition %ld.\n", index++);
    }

    try {
        this->spi_flash.quit_4byte_mode();
    } catch (const std::exception &e) {
        logf_err("flash quit 4 byte mode failed. %s\n", e.what());
        return false;
    }

    file.close();

    return !write_verify_fail;
}