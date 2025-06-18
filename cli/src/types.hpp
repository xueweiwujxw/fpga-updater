/**
 * @file types.hpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief 
 * @date 2024-09-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

/**
 * @brief 仅创建共享内存
 * @param file_name char[] 共享内存文件名
 * @param mode boost::interprocess::mode_t 共享内存读写模式
 * @param size size_t 共享内存使用大小
 * @param pointer void* 内存映射的指针
 *
 */
#define create_shm_only(file_name, mode, size, pointer)                 \
    intp::shared_memory_object::remove(file_name);                      \
    intp::shared_memory_object shm(intp::create_only, file_name, mode); \
    shm.truncate(size);                                                 \
    intp::mapped_region region(shm, mode);                              \
    pointer = region.get_address();

/**
 * @brief 创建或打开共享内存，存在则打开，否则创建
 * @param file_name char[] 共享内存文件名
 * @param mode boost::interprocess::mode_t 共享内存读写模式
 * @param size size_t 共享内存使用大小
 * @param pointer void* 内存映射的指针
 *
 */
#define open_or_create_shm(file_name, mode, size, pointer)              \
    intp::shared_memory_object shm(intp::create_only, file_name, mode); \
    shm.truncate(size);                                                 \
    intp::mapped_region region(shm, mode);                              \
    pointer = region.get_address();

/**
 * @brief 仅打开共享内存
 * @param file_name char[] 共享内存文件名
 * @param mode boost::interprocess::mode_t 共享内存读写模式
 * @param pointer void* 内存映射的指针
 *
 */
#define open_shm_only(file_name, mode, pointer)                       \
    intp::shared_memory_object shm(intp::open_only, file_name, mode); \
    intp::mapped_region region(shm, mode);                            \
    pointer = region.get_address();

namespace intp = boost::interprocess;

constexpr size_t KiB = 1024;
constexpr size_t MiB = 1024 * KiB;
constexpr size_t GiB = 1024 * MiB;
