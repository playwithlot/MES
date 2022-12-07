#ifndef __TCP_LOG_H__
#define __TCP_LOG_H__

#include "rtthread.h"
#include "common.h"

void create_tcp_log_thread(void);
void tcp_log_print(const char *buf, uint32_t len);

#endif
