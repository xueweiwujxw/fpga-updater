/**
 * @file custom_exception.hpp
 * @author Victor Li (leevick@tsinghua.edu.cn)
 * @brief
 * @version 0.1
 * @date 2022-09-05
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <exception>
#include <string>

#ifndef CUSTOM_EXCEPTION
#define CUSTOM_EXCEPTION
#define STR_PCIE_CANNOT_OPEN_DEV "无法打开/dev目录"
#define STR_PCIE_NO_SLOTS "无法检测到PCIe插槽"
#define STR_PCIE_NO_DEVICES "无法检测到XDMA设备"
#define STR_PCIE_NO_CONFIG "/env/slot.json不存在或无法解析"
#define STR_PCIE_NO_YKYC_CONFIG "/env/slot.json文件不包含设备插槽号"
#define STR_PCIE_NO_YKYC_SLOT "/env/slot.json文件中指定的设备插槽号不存在"
#define STR_PCIE_NO_YKYC_BUS "未能查询到设备对应的总线"
#define STR_PCIE_SLOT_PARSE_ERR "PCIE插槽解析错误"
#define STR_XDMA_BYPASS_OPEN_FAILED "XDMA寄存器文件描述符打开失败"
#define STR_XDMA_BYPASS_MAP_FAILED "XDMA寄存器映射失败"
#define STR_XDMA_CHANNEL_OPEN_FAILED "XDMA通道文件描述符打开失败"
#define STR_XDMA_CHANNEL_START_FAILED "XDMA通道启动失败"
#define STR_XDMA_EVENTS_OPEN_FAILED "XDMA中断文件描述符打开失败"
#define STR_WS_SERVER_REQ_MISSING_KEY "请求数据字段缺失"
#define STR_WS_SERVER_REQ_DATA_TYPE "请求数据类型错误"
#define STR_WS_SERVER_REQ_DATA_RANGE "请求数据范围错误"
#define STR_FILE_NOT_EXIST "文件不存在"
#define STR_FILE_SIZE_ZERO "文件大小为0"
#define STR_FILE_OPEN_FAILED "文件打开失败"
#define STR_FILE_XFER_CANCELLED "文件传输取消"
#define STR_FILE_READ_ERROR "文件数据读取错误"
#define STR_FILE_WRITE_ERROR "文件数据写入错误"
#define STR_PCIE_NO_DEMOD_CONFIG "/env/slot.json文件不包含解调器设备插槽号"
#define STR_PCIE_NO_DEMOD_DEVICE "检测不到解调器设备"
#define STR_CTRL_NO_PERMISSION "no permission"
#define STR_CTRL_START_FAIL "demod channel start failed"
#define STR_CTRL_STOP_FAIL "demod channel stop failed"
#define STR_HOST_CONF_REQUIRED "host conf file not found"
#define STR_CONFIG_NOT_FOUND "config not found"
#endif

namespace rohm
{
    enum class ErrorTypes
    {
        PCIE_CANNOT_OPEN_DEV,
        PCIE_NO_SLOTS,
        PCIE_NO_DEVICES,
        PCIE_NO_CONFIG,
        PCIE_NO_YKYC_CONFIG,
        PCIE_NO_YKYC_SLOT,
        PCIE_NO_YKYC_BUS,
        PCIE_SLOT_PARSE_ERR,
        XDMA_BYPASS_OPEN_FAILED,
        XDMA_BYPASS_MAP_FAILED,
        XDMA_CHANNEL_OPEN_FAILED,
        XDMA_CHANNEL_START_FAILED,
        XDMA_EVENTS_OPEN_FAILED,
        WS_SERVER_REQ_MISSING_KEY,
        WS_SERVER_REQ_DATA_TYPE,
        WS_SERVER_REQ_DATA_RANGE,
        FILE_NOT_EXIST,
        FILE_SIZE_ZERO,
        FILE_OPEN_FAILED,
        FILE_XFER_CANCELLED,
        FILE_READ_ERROR,
        FILE_WRITE_ERROR,
        PCIE_NO_DEMOD_CONFIG,
        PCIE_NO_DEMOD_DEVICE,
        CTRL_NO_PERMISSION,
        CTRL_START_FAIL,
        CTRL_STOP_FAIL,
        HOST_CONF_REQUIRED,
        CONFIG_NOT_FOUND,
    };

    static const char *errorMessages[] = {
        STR_PCIE_CANNOT_OPEN_DEV,
        STR_PCIE_NO_SLOTS,
        STR_PCIE_NO_DEVICES,
        STR_PCIE_NO_CONFIG,
        STR_PCIE_NO_YKYC_CONFIG,
        STR_PCIE_NO_YKYC_SLOT,
        STR_PCIE_NO_YKYC_BUS,
        STR_PCIE_SLOT_PARSE_ERR,
        STR_XDMA_BYPASS_OPEN_FAILED,
        STR_XDMA_BYPASS_MAP_FAILED,
        STR_XDMA_CHANNEL_OPEN_FAILED,
        STR_XDMA_CHANNEL_START_FAILED,
        STR_XDMA_EVENTS_OPEN_FAILED,
        STR_WS_SERVER_REQ_MISSING_KEY,
        STR_WS_SERVER_REQ_DATA_TYPE,
        STR_WS_SERVER_REQ_DATA_RANGE,
        STR_FILE_NOT_EXIST,
        STR_FILE_SIZE_ZERO,
        STR_FILE_OPEN_FAILED,
        STR_FILE_XFER_CANCELLED,
        STR_FILE_READ_ERROR,
        STR_FILE_WRITE_ERROR,
        STR_PCIE_NO_DEMOD_CONFIG,
        STR_PCIE_NO_DEMOD_DEVICE,
        STR_CTRL_NO_PERMISSION,
        STR_CTRL_START_FAIL,
        STR_CTRL_STOP_FAIL,
        STR_HOST_CONF_REQUIRED,
        STR_CONFIG_NOT_FOUND,
    };

    class CustomException : public std::exception
    {
    private:
        ErrorTypes code;
        std::string customized;

    public:
        CustomException(ErrorTypes e) : code(e), customized("") {}
        CustomException(std::string s) : code(ErrorTypes::CONFIG_NOT_FOUND), customized(s) {}
        virtual const char *what() const noexcept override {
            if (this->customized != "")
                return errorMessages[static_cast<int>(code)];
            else
                return this->customized.c_str();
        }
    };
}
