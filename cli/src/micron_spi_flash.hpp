/**
 * @file micron_spi_flash.hpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @date 2025-01-07
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <atomic>
#include <functional>
#include <string>

#include <axispi.h>
#include <log.h>

namespace micronnp
{

    constexpr uint16_t READ_SIZE = 256;
    constexpr uint16_t WRITE_SIZE = 256;
    constexpr unsigned wait_period_us = 500;

    struct micron_spi_flash_device_id
    {
        uint8_t manufacture_id = 0;
        uint8_t device_id[2] = {0};
        uint8_t uid[17] = {0};

        std::string display();
    };

    class micron_spi_flash
    {
    private:
        axispi_ctrl_t *spi_t;
        std::atomic<bool> initialize_stat;
        std::atomic<bool> mode_4b;
        uint16_t op_timeout_s;

        template <typename Func, typename... Args>
        auto invoke_pre_post(Func func, Args &&...args) {
            if (!this->probe_flash())
                throw std::runtime_error("Failed to probe flash");

            try {
                if constexpr (std::is_void_v<std::invoke_result_t<Func, Args...>>) {
                    func(std::forward<Args>(args)...);
                    if (!this->remove_flash())
                        throw std::runtime_error("Failed to remove flash");

                    return;
                } else {
                    auto result = func(std::forward<Args>(args)...);
                    if (!this->remove_flash())
                        throw std::runtime_error("Failed to remove flash");

                    return result;
                }

            } catch (const std::exception &e) {
                throw;
            }
        }

        /**
         * @brief 检查是否正在执行下面的操作
         *
         * WRITE STATUS REGISTER, WRITE NONVOLATILE CONFIGURATION REGISTER, PROGRAM, ERASE
         *
         * @param reg 寄存器值
         * @return true 正在执行
         * @return false 设备空闲
         */
        bool check_write_in_progress_with_reg(const uint8_t &reg);

        /**
         * @brief 检查是否允许写入，不允许写入无法执行下面的操作
         *
         * WRITE, PROGRAM, or ERASE operations
         *
         * @param reg 寄存器值
         * @return true 允许写入
         * @return false 不允许写入
         */
        bool check_write_enable_latch_with_reg(const uint8_t &reg);

        /**
         * @brief 检查设备是否就绪
         *
         * @param reg 寄存器值
         * @return true 就绪
         * @return false 忙碌
         */
        bool check_device_ready_with_reg(const uint8_t &reg);

        /**
         * @brief 检查擦除是否成功
         *
         * @param reg 寄存器值
         * @return true 成功
         * @return false 失败
         */
        bool check_erase_clear_with_reg(const uint8_t &reg);

        /**
         * @brief 检查写入是否成功，同时也会标记CRC的状态
         *
         * @param reg 寄存器值
         * @return true 成功
         * @return false 失败
         */
        bool check_program_clear_with_reg(const uint8_t &reg);

        /**
         * @brief 检查写入是否合法
         *
         * @param reg 寄存器值
         * @return true 写入合法
         * @return false 写入保护或者锁定区域
         */
        bool check_protection_clear_with_reg(const uint8_t &reg);

        /**
         * @brief 挂载设备
         *
         * @return true
         * @return false
         */
        bool probe_flash();

        /**
         * @brief 卸载设备
         *
         * @return true
         * @return false
         */
        bool remove_flash();

        /**
         * @brief 允许写入
         *
         */
        void write_enable();

        /**
         * @brief 禁止写入
         *
         */
        void write_disable();

    public:
        micron_spi_flash();
        ~micron_spi_flash();

        /**
         * @brief 返回初始化状态
         *
         * @return true
         * @return false
         */
        bool initialized();

        /**
         * @brief 初始化
         *
         * @param addr 分配的内存映射地址
         * @param timeout_ms spi超时时间
         * @param op_timeou_s spi操作超时时间
         * @return true
         * @return false
         */
        bool init(void *addr, uint16_t timeout_ms = 1000, uint16_t op_timeou_s = 10);

        /**
         * @brief 初始化flash
         *
         */
        void init_flash();

        /**
         * @brief 进入4 byte模式
         *
         */
        void enter_4byte_mode();

        /**
         * @brief 退出4 byte模式
         *
         */
        void quit_4byte_mode();

        /**
         * @brief 读取状态寄存器
         *
         * @return uint8_t
         */
        uint8_t read_status_register();

        /**
         * @brief 读取标志状态寄存器
         *
         * @return uint8_t
         */
        uint8_t read_flag_status_register();

        /**
         * @brief 读取数据
         *
         * @param addr 地址
         * @param data 数据指针
         * @param length 数据长度
         */
        void read_data(uint32_t addr, uint8_t *data, uint16_t length);

        /**
         * @brief 读取id
         *
         * @return  micron_spi_flash_device_id
         */
        micron_spi_flash_device_id read_id();

        /**
         * @brief 写入数据
         *
         * @param addr 地址
         * @param data 数据指针
         * @param length 数据长度
         */
        void write_data(uint32_t addr, uint8_t *data, uint16_t length);

        /**
         * @brief 擦除4K块
         *
         * @param addr
         */
        void erase_4k_block(uint32_t addr);

        /**
         * @brief 擦除块
         *
         * @param addr
         */
        void erase_32k_block(uint32_t addr);

        /**
         * @brief 擦除块
         *
         * @param addr
         */
        void erase_sector_block(uint32_t addr);
    };

} // namespace micronn
