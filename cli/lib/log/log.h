/**
 * @file log.h
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-09
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <string.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char *get_local_time();

#ifndef APPNAME
#define LOGPREIX ""
#else
#define LOGPREIX "[" APPNAME "]"
#endif

#define __LOCAL_TIME__ \
    get_local_time()

#define __FILENAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define logf_info(fmt, ...) \
    fprintf(stderr, LOGPREIX "[%s] [info ] %s: %s: " fmt, __LOCAL_TIME__, __FILENAME__, __func__, ##__VA_ARGS__)

#define logf_err(fmt, ...) \
    fprintf(stderr, LOGPREIX "[%s] [error] %s:%d: %s: " fmt, __LOCAL_TIME__, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#define logf_warn(fmt, ...) \
    fprintf(stderr, LOGPREIX "[%s] [warn ] %s:%d: %s: " fmt, __LOCAL_TIME__, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#ifdef DEBUG
#define logf_debug(fmt, ...) \
    fprintf(stderr, LOGPREIX "[%s] [debug] %s:%d: %s: " fmt, __LOCAL_TIME__, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define logf_debug(fmt, ...)
#endif

#ifdef __cplusplus
}
#endif
