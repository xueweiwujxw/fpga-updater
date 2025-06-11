/**
 * @file pcie_slot.hpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-11-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <map>
#include <vector>
#include <sstream>
#include <sys/dir.h>
#include <string.h>
#include <types.hpp>
#include <dirent.h>

#include <custom_exception.hpp>
#include <log.h>

namespace interface
{
    /**
     * @brief 字符串切分
     *
     * @param str 输入的字符串
     * @param frags 保存切分结果的向量
     * @param delim 切分标志，目前仅支持char字符
     */
    inline static void split(const std::string &str, std::vector<std::string> &frags, const char delim = ' ') {
        frags.clear();
        std::istringstream iss(str);
        std::string tmp;
        while (std::getline(iss, tmp, delim)) {
            if (tmp != "") {
                frags.emplace_back(std::move(tmp));
            }
        }
    }

    /**
     * @brief 获取插槽号和bus的映射
     * @exception PCIE_SLOT_PARSE_ERR
     * @exception std::exception
     *
     * @return std::map<int, std::string> int: id; string: bus
     */
    inline static std::map<int, std::string> scan_slots() {
        std::map<int, std::string> slots;
        try {
            auto f = popen("dmidecode -t slot | awk '/Designation: .*SLOT[0-9]/{match($0, /SLOT[0-9]+/); slot=substr($0, RSTART, RLENGTH); next} /Bus Address:/{if (slot) {print slot, $3}}'", "r");
            char buffer[2048] = {0};
            while (fgets(buffer, sizeof(buffer), f)) {
                std::string line(buffer);
                int key = 0;
                std::string bus = "";
                std::vector<std::string> frags;
                std::vector<std::string> addrs;
                split(line, frags, ' ');
                if (frags.size() != 2)
                    throw rohm::CustomException(rohm::ErrorTypes::PCIE_SLOT_PARSE_ERR);
                sscanf(frags[0].c_str(), "SLOT%d", &key);
                split(frags[1], addrs, ':');
                if (addrs.size() != 3)
                    throw rohm::CustomException(rohm::ErrorTypes::PCIE_SLOT_PARSE_ERR);
                bus = addrs[1];
                slots.insert(std::make_pair(key, bus));
            }
            pclose(f);
        } catch (const std::exception &e) {
            throw;
        }
        return slots;
    }

    /**
     * @brief 获取bus和设备id的映射关系
     * @exception PCIE_CANNOT_OPEN_DEV
     *
     * @return std::map<std::string, int> string: bus; id: device_id
     */
    inline static std::map<std::string, int> scan_devices() {
        std::map<std::string, int> devices;
        DIR *dir = nullptr;
        struct dirent *entry;
        if ((dir = opendir("/dev")) == nullptr) {
            throw rohm::CustomException(rohm::ErrorTypes::PCIE_CANNOT_OPEN_DEV);
        } else {
            while ((entry = readdir(dir)) != nullptr) {
                if ((strstr(entry->d_name, "xdma") != nullptr) && (strstr(entry->d_name, "control") != nullptr)) {
                    char buffer[3] = "";
                    int device_id = 0;
                    sscanf(entry->d_name, "xdma%d_control_bus0%s", &device_id, buffer);
                    std::string bus(buffer);
                    devices.insert(std::make_pair(bus, device_id));
                }
            }
            closedir(dir);
        }
        return devices;
    }
} // namespace interface
