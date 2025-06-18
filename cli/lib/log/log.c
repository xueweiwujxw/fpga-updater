/**
 * @file log.c
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2023-02-27
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "log.h"

static const char format[] = "%Y-%m-%d %H:%M:%S";
static char result[4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1];

char *get_local_time() {
    time_t tt_;
    struct tm *tm_;
    time(&tt_);
    tm_ = localtime(&tt_);
    strftime(result, sizeof(result), format, tm_);
    return result;
}
