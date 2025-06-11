/**
 * @file xdma_engine.cpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @version 0.1
 * @date 2021-07-19
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zmq.h>
#include <iostream>
#include <wait.h>
#include <sys/time.h>

#include <types.hpp>
#include <log.h>
#include <xdma_engine.hpp>

using namespace std;
using namespace xdma_np;

xdma_channel::xdma_channel(int id, int rx_fd, int tx_fd, bool has_rx, bool has_tx) {
    this->id = id;
    this->rx_fd = rx_fd;
    this->tx_fd = tx_fd;
    this->has_rx = has_rx;
    this->has_tx = has_tx;
    this->order = 0;
    this->is_running = false;
    memset(this->shm_file, 0, sizeof(shm_file));
    this->user.store(0);
    logf_debug("id: %d, rx_fd: %d, tx_fd: %d, has_rx: %d, has_tx: %d\n",
               this->id, this->rx_fd, this->tx_fd, this->has_rx, this->has_tx);
}

xdma_channel::~xdma_channel() {}

/**
 * @brief 开启一个xdma接收进程
 *
 * @param order 用户板卡编号
 * @return int
 */
int xdma_channel::start(int order, int upload_period) {
    this->user++;
    logf_debug("this user count: %d of order: %d\n", this->user.load(), this->order);
    if (this->is_running) {
        return 0;
    }
    try {
        this->order = order;
        sprintf(this->shm_file, xdma_shm, order);
        void *pointer;
        create_shm_only(this->shm_file, intp::read_write, sizeof(xdma_share_param), pointer);
        xdma_share_param *param = reinterpret_cast<xdma_share_param *>(pointer);
        param->running = true;
        param->upload_period = upload_period;
        param->xdma_ready = false;
    } catch (const std::exception &e) {
        logf_err("create controller.shm failed\n");
        return -1;
    }

    pid_t pid = fork();
    if (pid > 0) {
        this->child_pid = pid;
        this->is_running = true;

        void *pointer;
        open_shm_only(shm_file, intp::read_write, pointer);
        xdma_share_param *param = reinterpret_cast<xdma_share_param *>(pointer);

        if (param != nullptr) {
            int try_num = 100;
            while (try_num-- > 0) {
                if (param->xdma_ready)
                    break;
                usleep(100000);
            }
            if (param->xdma_ready) {
                for (auto &i : this->start_post_callbacks)
                    i.second();
                return 0;
            } else {
                logf_err("panic: xdma child process not ready\n");
                return -1;
            }
        } else {
            return -1;
        }

    } else if (pid < 0) {
        logf_err("create pid failed\n");
        return -1;
    } else {
        try {
            if (execl("/usr/bin/demod_controller_xdma", "/usr/bin/demod_controller_xdma", "-c", std::to_string(order).c_str(), "-s", this->shm_file, "-f", std::to_string(this->rx_fd).c_str(), nullptr) == 1)
                logf_err("xdma engine of channel %d run child process error: %s\n", order, strerror(errno));

        } catch (const std::exception &e) {
            logf_err("%s\n", e.what());
        }
        exit(1);
    }
}

/**
 * @brief 停止接收任务，并返回接收数据的统计值
 *
 * @return unsigned long long
 */
int xdma_channel::stop() {
    if (!this->is_running) {
        logf_debug("not running\n");
        return 0;
    }
    this->user--;
    logf_debug("this user count: %d of order: %d\n", this->user.load(), this->order);
    if (this->user < 0) {
        logf_err("xdma user record less than 0.\n");
        this->user = 0;
        return -1;
    }
    if (this->user > 0)
        return 0;
    try {
        void *pointer;
        open_shm_only(this->shm_file, intp::read_write, pointer);
        xdma_share_param *param = reinterpret_cast<xdma_share_param *>(pointer);
        param->running = false;
        waitpid(this->child_pid, nullptr, 0);
        logf_info("xdma channel %d transfer %lld bytes\n", this->order, param->trans_len);
        intp::shared_memory_object::remove(this->shm_file);
        this->is_running = false;

        // post process
        for (auto &i : this->stop_post_callbacks)
            i.second();
        return 0;
    } catch (const exception &e) {
        logf_err("stop failed %s\n", e.what());
        return -1;
    }
}

void xdma_channel::register_start_callback(std::string key, xdma_channel_post_callback callback) {
    this->start_post_callbacks.insert(std::make_pair(key, callback));
}

void xdma_channel::unregister_start_callback(std::string key) {
    this->start_post_callbacks.erase(key);
}

void xdma_channel::register_stop_callback(std::string key, xdma_channel_post_callback callback) {
    this->stop_post_callbacks.insert(std::make_pair(key, callback));
}

void xdma_channel::unregister_stop_callback(std::string key) {
    this->stop_post_callbacks.erase(key);
}

xdma_engine::xdma_engine() {
}

xdma_engine::~xdma_engine() {
}

/**
 * @brief 初始话对应板卡的xdma设备，并创建对应通道
 *
 * @param id
 * @return int
 */
int xdma_engine::init(int id, string bus) {
    char devname[256] = {0};

    sprintf(devname, "/dev/xdma%d_bypass_bus0%s", id, bus.c_str());
    int mmapfdTmp;
    mmapfdTmp = open(devname, O_RDWR);
    if (mmapfdTmp < 0) {
        logf_err("xdma_engine[%d]bypass open failed! %s\n", id, strerror(errno));
        return mmapfdTmp;
    }
    this->mmapfd.insert(make_pair(id, mmapfdTmp));

    for (int i = 0; i < 4; i++) {
        bool has_tx = false;
        bool has_rx = false;
        int rx_fd = 0, tx_fd = 0;

        struct stat s;
        sprintf(devname, "/dev/xdma%d_h2c_%d_bus%s", id, i, bus.c_str());
        if (stat(devname, &s) >= 0) {
            tx_fd = open(devname, O_RDWR | O_NONBLOCK);
            if (tx_fd < 0) {
                logf_err("tx channel %d open failed: %s\n", i, strerror(errno));
            } else {
                has_tx = true;
            }
        }
        sprintf(devname, "/dev/xdma%d_c2h_%d_bus%s", id, i, bus.c_str());
        if (stat(devname, &s) >= 0) {
            rx_fd = open(devname, O_RDWR | O_NONBLOCK);
            if (tx_fd < 0) {
                logf_err("rx channel %d open failed: %s\n", i, strerror(errno));
            } else {
                has_rx = true;
            }
        }

        if (has_tx || has_rx) {
            shared_ptr<xdma_channel> ptr = make_shared<xdma_channel>(id, rx_fd, tx_fd, has_rx, has_tx);
            this->channels.insert(make_pair(id, ptr));
            map<unsigned long long, segment> segItem;
            this->segments.insert(make_pair(id, segItem));
            logf_debug("xdma[%d] has_rx = %d, has_tx = %d\n", id, has_rx, has_tx);
        }
    }

    logf_info("xdma channel %d init finish\n", id);
    return 0;
}

/**
 * @brief 分配映射到寄存器的内存
 *
 * @param offset 偏移量
 * @param len 长度
 * @param channel 设备通道id
 * @return void*
 */
void *xdma_engine::register_map(unsigned long long offset, unsigned int len, int channel) {
    for (auto p : this->segments[channel]) {
        if ((offset >= p.first && offset < (p.second.offset + p.second.len)) || ((offset + len) > p.first && (offset + len) <= (p.second.offset + p.second.len))) {
            logf_err("cannot map segments: offset: %lld, len: len: %d, channel: %d\n", offset, len, channel);
            return nullptr;
        }
    }

    void *ret = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, this->mmapfd[channel], offset);

    if (ret == nullptr) {
        logf_err("map failed %s\n", strerror(errno));
        return ret;
    }

    segment seg = {offset, len, ret};
    this->segments[channel].insert(make_pair(offset, seg));

    return ret;
}

/**
 * @brief 取消寄存器映射
 *
 * @param addr
 * @param channel
 * @return int
 */
int xdma_engine::register_unmap(void *addr, int channel) {
    for (auto p : this->segments[channel]) {
        if (p.second.address == addr) {
            munmap(addr, p.second.len);
            return 0;
        }
    }
    logf_warn("segments not found!\n");
    return -1;
}

/**
 * @brief 启动指定通道
 *
 * @param chid 通道id
 * @param order 用户序号
 * @return int
 */
int xdma_engine::start_channel(int chid, int order, int upload_period) {
    auto ch = this->channels.find(chid);
    if (ch == this->channels.end()) {
        logf_warn("channel %d not found\n", chid);
        errno = ENOKEY;
        return -1;
    }

    return ch->second->start(order, upload_period);
}

/**
 * @brief 停止指定通道
 *
 * @param chid
 * @return int
 */
int xdma_engine::stop_channel(int chid) {
    auto ch = this->channels.find(chid);
    if (ch == this->channels.end()) {
        logf_warn("channel %d not found\n", chid);
        errno = ENOKEY;
        return -1;
    }

    return ch->second->stop();
}

/**
 * @brief 释放资源
 *
 */
void xdma_engine::deinit() {
    for (auto ch : this->channels) {
        ch.second->stop();
        close(ch.second->get_rx_fd());
        close(ch.second->get_tx_fd());
    }
    this->channels.clear();

    for (auto p : this->segments) {
        for (auto pp : p.second)
            munmap(pp.second.address, pp.second.len);
    }
}

/**
 * @brief 获取tx_fd
 * 
 * @param chid 
 * @return int 
 */
int xdma_engine::get_tx_fd(int chid) {
    auto ch = this->channels.find(chid);
    if (ch == this->channels.end()) {
        logf_warn("channel %d not found\n", chid);
        errno = ENOKEY;
        return -1;
    }
    return ch->second->get_tx_fd();
}

/**
 * @brief 获取rx_fd
 * 
 * @param chid 
 * @return int 
 */
int xdma_engine::get_rx_fd(int chid) {
    auto ch = this->channels.find(chid);
    if (ch == this->channels.end()) {
        logf_warn("channel %d not found\n", chid);
        errno = ENOKEY;
        return -1;
    }
    return ch->second->get_rx_fd();
}

/**
 * @brief 获取运行状态
 * 
 * @param chid 
 * @return true 
 * @return false 
 */
bool xdma_engine::is_running(int chid) {
    auto ch = this->channels.find(chid);
    if (ch == this->channels.end()) {
        logf_warn("channel %d not found\n", chid);
        errno = ENOKEY;
        return false;
    }
    return ch->second->running_stat();
}

bool xdma_engine::register_start_callback(int chid, std::string key, xdma_channel_post_callback callback) {
    try {
        this->channels[chid]->register_start_callback(key, callback);
        return true;
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        return false;
    }
}

bool xdma_engine::unregister_start_callback(int chid, std::string key) {
    try {
        this->channels[chid]->unregister_start_callback(key);
        return true;
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        return false;
    }
}

bool xdma_engine::register_stop_callback(int chid, std::string key, xdma_channel_post_callback callback) {
    try {
        this->channels[chid]->register_stop_callback(key, callback);
        return true;
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        return false;
    }
}

bool xdma_engine::unregister_stop_callback(int chid, std::string key) {
    try {
        this->channels[chid]->unregister_stop_callback(key);
        return true;
    } catch (const std::exception &e) {
        logf_err("%s\n", e.what());
        return false;
    }
}
