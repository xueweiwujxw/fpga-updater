/**
 * @file mcs_parser.hpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief 
 * @date 2024-09-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include <numeric>
#include <sstream>
#include <types.hpp>

namespace tools_np
{
    /**
     * @brief MCS校验结果
     * 
     */
    struct MCSParserResult
    {
        bool valid;
        ssize_t bytes;
    };

    /**
     * @brief MCS块数据信息
     * 
     */
    struct MCSFragment
    {
        uint16_t addr = 0;
        ssize_t bytes = 0;
        uint8_t buffer[64 * KiB] = {0};
    };

    /**
     * @brief MCS行起始定义
     * 
     * @details mcs(hex)文件格式行定义
     * :+数据个数(1字节)+起始地址(2字节)+记录类型(1字节)+数据(N字节)+校验和(1字节)
     */
    struct MCSLineHeader
    {
        uint8_t num;       // 行数据个数
        uint16_t addr;     // 行地址
        uint8_t data_type; // 类型
    };

    /**
     * @brief MCS文件行类型
     * 
     */
    enum class MCSLineType
    {
        Data = 0x00,           // 00：数据，示例:0B 0010 00 6164647265737320676170 A7
        FileEof = 0x01,        // 01：文件结束，示例:00 0000 01 FF
        ExtFragAddr = 0x02,    // 02：扩展段地址，示例:02 0000 02 1200 EA
        StartFragAddr = 0x03,  // 03：起始段地址，示例:04 0000 03 0000 3800 C1
        ExtLinearAddr = 0x04,  // 04：扩展线性地址，示例:02 0000 04 FFFF FC
        StartLinearAddr = 0x05 // 05：起始线性地址，示例:04 0000 05 0000 00CD 2A
    };

    struct FlashPartition
    {
        uint32_t addr_start;
        uint32_t addr_end;
        std::string date;
        std::string file;
        uint32_t checksum;
        uint32_t size;

        void print() {
            printf("AddrStart: 0x%08x\n", this->addr_start);
            printf("AddrEnd:   0x%08x\n", this->addr_end);
            printf("Date:      %s\n", this->date.c_str());
            printf("File:      %s\n", this->file.c_str());
            printf("Checksum:  0x%08x\n", this->checksum);
            printf("Size:      %u\n", this->size);
        }
    };

    /**
     * @brief 解析mcs文件，返回文件校验结果和有效字节数
     * 
     * @param check_file 
     * @return MCSParserResult 
     */
    inline MCSParserResult check_valid(const std::string &check_file) {
        ssize_t total_bytes = 0;
        std::ifstream file(check_file);
        if (!file.is_open())
            return {false, -1};

        std::string line;
        bool check_finish = false;
        while (std::getline(file, line)) {
            // 筛除空行和前导非':'行
            if (line.empty() || line[0] != ':')
                continue;

            // 转换为数字
            std::vector<uint8_t> buffer;
            for (size_t i = 1; i < line.size() - 1; i += 2)
                buffer.push_back(static_cast<uint8_t>(std::strtoul(line.substr(i, 2).c_str(), nullptr, 16)));

            // 大小不足header和sum则错误
            if (buffer.size() < 5)
                break;

            // header和sum赋值
            MCSLineHeader header = {
                buffer[0],
                static_cast<uint16_t>((static_cast<uint16_t>(buffer[1]) << 8) | static_cast<uint16_t>(buffer[2])),
                buffer[3],
            };
            uint16_t line_sum = buffer.back();

            // 判断长度是否正确
            if (buffer.size() != static_cast<size_t>(header.num + 5))
                break;

            // 判断校验值是否正确
            uint16_t sum = (0x100 - (std::accumulate(buffer.begin(), buffer.end() - 1, 0) & 0xff)) & 0xff;
            if (sum != line_sum)
                break;

            if (header.data_type == static_cast<uint8_t>(MCSLineType::Data)) {
                total_bytes += buffer[0];
            } else if (header.data_type == static_cast<uint8_t>(MCSLineType::FileEof)) {
                check_finish = true;
                break;
            }
        }

        file.close();
        return {check_finish, total_bytes};
    }

    /**
     * @brief 获取mcs的一块数据，大小为64KB，对应flash的一个扇区大小
     * 
     * @param fd 文件描述符，需要已经打开的文件
     * @param data_buffer 开辟的至少64KB大小的数据缓冲区，不足无法抛出异常
     * @param buffer_size 传输的缓冲区指针的大小
     * @return std::pair<uint16_t, ssize_t> 拓展线性地址和写入缓冲区的数据大小，-1代表写入失败
     */
    inline std::pair<uint16_t, ssize_t> get_mcs_fragment(std::ifstream &file, uint8_t *data_buffer, size_t buffer_size, size_t fragment_size) {
        if (buffer_size < 64 * KiB || data_buffer == nullptr || fragment_size > buffer_size)
            return {0, -1};

        ssize_t read_bytes = 0;
        std::string line;

        uint16_t ext_linear_addr = 0;

        bool find_addr = false;
        while (std::getline(file, line)) {
            // 筛除空行和前导非':'行
            if (line.empty() || line[0] != ':')
                continue;

            // 转换为数字
            std::vector<uint8_t> buffer;
            for (size_t i = 1; i < line.size() - 1; i += 2)
                buffer.push_back(static_cast<uint8_t>(std::strtoul(line.substr(i, 2).c_str(), nullptr, 16)));

            // 大小不足header和sum则错误
            if (buffer.size() < 5)
                break;

            // header赋值
            MCSLineHeader header = {
                buffer[0],
                static_cast<uint16_t>((static_cast<uint16_t>(buffer[1]) << 8) | static_cast<uint16_t>(buffer[2])),
                buffer[3],
            };
            if (header.data_type == static_cast<uint8_t>(MCSLineType::ExtLinearAddr)) {
                ext_linear_addr = static_cast<uint16_t>((static_cast<uint16_t>(buffer[4]) << 8) | static_cast<uint16_t>(buffer[5]));
                find_addr = true;
                break;
            }
        }

        if (!find_addr)
            return {0, -1};

        int data_buffer_offset = 0;
        while (std::getline(file, line)) {
            // 筛除空行和前导非':'行
            if (line.empty() || line[0] != ':')
                continue;

            // 转换为数字
            std::vector<uint8_t> buffer;
            for (size_t i = 1; i < line.size() - 1; i += 2)
                buffer.push_back(static_cast<uint8_t>(std::strtoul(line.substr(i, 2).c_str(), nullptr, 16)));

            // 大小不足header和sum则错误
            if (buffer.size() < 5)
                break;

            // header赋值
            MCSLineHeader header = {
                buffer[0],
                static_cast<uint16_t>((static_cast<uint16_t>(buffer[1]) << 8) | static_cast<uint16_t>(buffer[2])),
                buffer[3],
            };

            if (header.data_type == static_cast<uint8_t>(MCSLineType::Data)) {
                for (size_t i = 4; i < buffer.size() - 1; ++i) {
                    data_buffer[data_buffer_offset++] = buffer[i];
                    read_bytes++;
                }
            }
            if (read_bytes == static_cast<ssize_t>(fragment_size) || header.data_type == static_cast<uint8_t>(MCSLineType::FileEof))
                break;
        }

        return {ext_linear_addr, read_bytes};
    }

    inline std::vector<FlashPartition> parse_prm(std::string &file_path) {
        std::string line = "";
        std::vector<FlashPartition> partitions;

        std::ifstream file(file_path);
        if (!file.is_open())
            return partitions;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '=' || line.find("Addr1") != std::string::npos)
                continue;

            std::istringstream linestream(line);
            std::string addr1, addr2, month, date, time, year, bitfile, checksum;
            if (linestream >> addr1 >> addr2 >> month >> date >> time >> year >> bitfile >> checksum) {
                FlashPartition partition;
                partition.addr_start = std::stoul(addr1, nullptr, 16);
                partition.addr_end = std::stoul(addr2, nullptr, 16);
                partition.date = month + " " + date + " " + time + " " + year;
                partition.file = bitfile;
                partition.checksum = std::stoul(checksum, nullptr, 16);
                partition.size = partition.addr_end - partition.addr_start + 1;

                partitions.push_back(partition);
            }
        }

        file.close();

        return partitions;
    }
} // namespace tools_np
