/**
 * @file tools.hpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief 
 * @date 2024-10-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <stdint.h>
#include <stdio.h>

namespace tools_np
{
    inline void display_hex(uint8_t *data, uint32_t len) {
        uint32_t offset = 0;
        while (offset < len) {
            uint32_t right_border = len - offset > 16 ? 16 : len - offset;
            for (uint32_t i = offset; i < offset + right_border; ++i)
                fprintf(stderr, "%02x ", data[i]);
            fprintf(stderr, "\n");
            offset += right_border;
        }
    }
} // namespace tools_np
