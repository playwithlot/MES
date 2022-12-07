#ifndef __TCP_LOG_H__
#define __TCP_LOG_H__

#include "rtthread.h"

void tcp_log_thread(void *p);
void tcp_log_add(const char *buf, uint32_t len);

#endif
