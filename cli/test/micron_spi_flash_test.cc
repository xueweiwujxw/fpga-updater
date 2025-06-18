/**
 * @file micron_spi_flash_test.cc
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @date 2025-01-07
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <fstream>
#include <nlohmann/json.hpp>

#include <xdma_engine.hpp>
#include <pcie_slot.hpp>
#include <log.h>
#include <tools.hpp>
#include <micron_spi_flash.hpp>

using namespace interface;
using namespace xdma_np;
using namespace nlohmann;
using namespace std;
using namespace tools_np;
using namespace micronnp;

#define TEST_SLOT_ALIAS "analog"

bool hardware_init(xdma_engine &xdma, micron_spi_flash &spi) {
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
        if (!slot_config.contains(TEST_SLOT_ALIAS)) {
            logf_err("invalid alias %s\n", TEST_SLOT_ALIAS);
            return false;
        }

        auto slot_iter = slots.find(slot_config[TEST_SLOT_ALIAS].get<int>());
        if (slot_iter == slots.end()) {
            logf_err("can not find slot id %d\n", slot_config[TEST_SLOT_ALIAS].get<int>());
            return false;
        }
        string bus = slot_iter->second;

        auto device_iter = devices.find(bus);
        if (device_iter == devices.end()) {
            logf_err("can not find devices %s\n", bus.c_str());
            return false;
        }
        int device_id = device_iter->second;

        if (xdma.init(device_id, bus) != 0) {
            logf_err("xdma init failed.\n");
            return false;
        }

        auto addr = xdma.register_map(0x80090000L, 4 * KiB, device_id);
        if (addr == nullptr) {
            xdma.deinit();
            return false;
        }
        if (!spi.init(addr, 128 / 8, 4)) {
            xdma.deinit();
            return false;
        }
        return true;
    } catch (exception &e) {
        logf_err("%s\n", e.what());
        return false;
    }
}

int main(int argc, char const *argv[]) {
    micron_spi_flash spi_flash;
    xdma_engine xdma;

    if (!hardware_init(xdma, spi_flash))
        return 1;

    if (!spi_flash.initialized())
        return 1;

    try {
        printf("%s\n", spi_flash.read_id().display().c_str());

        spi_flash.enter_4byte_mode();

        printf("%02x\n", (uint8_t)spi_flash.read_status_register());
        printf("%02x\n", (uint8_t)spi_flash.read_flag_status_register());

        uint8_t rdata[256] = {0};
        uint8_t wdata[256] = {0};

        spi_flash.read_data(256, rdata, sizeof(rdata));
        display_hex(rdata, sizeof(rdata));
        printf("\n");

        spi_flash.erase_32k_block(0x07fffd00);
        spi_flash.read_data(0x07fffd00, wdata, sizeof(wdata));
        display_hex(wdata, sizeof(wdata));
        printf("\n");

        spi_flash.write_data(0x07fffd00, rdata, sizeof(rdata));
        spi_flash.read_data(0x07fffd00, wdata, sizeof(wdata));
        display_hex(wdata, sizeof(wdata));
        printf("\n");

        spi_flash.quit_4byte_mode();

        printf("%02x\n", (uint8_t)spi_flash.read_status_register());
        printf("%02x\n", (uint8_t)spi_flash.read_flag_status_register());
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
    }

    return 0;
}
