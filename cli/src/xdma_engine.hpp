/**
 * @file xdma_engine.hpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @version 0.1
 * @date 2021-07-19
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once

#include <aio.h>
#include <map>
#include <memory>
#include <atomic>
#include <string.h>
#include <functional>

namespace xdma_np
{
    struct xdma_share_param
    {
        bool running;                 // 执行中
        bool xdma_ready;              // xdma读取就绪
        int upload_period;            // 上报周期
        unsigned long long trans_len; // 传输长度
    };

    inline const char xdma_shm[] = "demod_xdma_%d_shm.shm";

    typedef std::function<void()> xdma_channel_post_callback;

    /**
     * @brief 寄存器内存映射段
     *
     */
    struct segment
    {
        unsigned long long offset;
        unsigned long long len;
        void *address;
    };

    /**
     * @brief 对应解调板的通道
     *
     */
    class xdma_channel
    {
    private:
        int id;                       // 实际设备bus id
        int order;                    // 用户定义id
        int tx_fd;                    // 发送文件描述符
        int rx_fd;                    // 接收文件描述符
        bool has_tx;                  // 发送通道有效
        bool has_rx;                  // 接收通道有效
        int child_pid;                // 子进程pid
        char shm_file[64];            // 进程共享内存文件名
        std::atomic<bool> is_running; // 标记运行状态
        std::atomic<int> user;        // 当前使用用户数量

        std::map<std::string, xdma_channel_post_callback> start_post_callbacks; // dma通道启动预处理
        std::map<std::string, xdma_channel_post_callback> stop_post_callbacks;  // dma通道停止后处理

    public:
        xdma_channel(int id, int rx_fd, int tx_fd, bool has_rx, bool has_tx);
        ~xdma_channel();
        int start(int order, int upload_period);
        int stop();
        int get_tx_fd() { return this->tx_fd; }
        int get_rx_fd() { return this->rx_fd; }
        bool running_stat() { return this->is_running; }
        void register_start_callback(std::string key, xdma_channel_post_callback callback);
        void unregister_start_callback(std::string key);
        void register_stop_callback(std::string key, xdma_channel_post_callback callback);
        void unregister_stop_callback(std::string key);
    };
    /**
     * @brief 读取驱动设备，安排通道任务
     *
     */
    class xdma_engine
    {
    private:
        std::map<int, std::shared_ptr<xdma_channel>> channels;         // first: channel num, second: xdma channel class shared pointer
        std::map<int, std::map<unsigned long long, segment>> segments; // fisrt: channel num, second: segments of channel
        std::map<int, int> mmapfd;                                     // fisrt: channel num, second: mmapfd of channel

    protected:
    public:
        xdma_engine();
        ~xdma_engine();
        int init(int id, std::string bus);
        void *register_map(unsigned long long offset, unsigned int len, int channel);
        int register_unmap(void *addr, int channel);
        int start_channel(int chid, int order, int upload_period);
        int stop_channel(int chid);
        int get_tx_fd(int chid);
        int get_rx_fd(int chid);
        void deinit();
        bool is_running(int chid);
        bool register_start_callback(int chid, std::string key, xdma_channel_post_callback callback);
        bool unregister_start_callback(int chid, std::string key);
        bool register_stop_callback(int chid, std::string key, xdma_channel_post_callback callback);
        bool unregister_stop_callback(int chid, std::string key);
    };
} // namespace xdma_np
