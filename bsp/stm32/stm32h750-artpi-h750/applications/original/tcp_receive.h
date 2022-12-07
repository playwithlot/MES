
#ifndef __TCP_RECEIVE_H__
#define __TCP_RECEIVE_H__

#include "rtthread.h"
#include "net_protocol.h"

#define TCP_PACK_MAX_LEN   1500


void tcp_deal_init(void);

void tcpserv_receive_thread(void *p);

int tcp_send_by_connect(const void *data, rt_uint32_t size, int flags);
int net_tcp_send(Net_pack_deal_t *deal);

extern int now_tcp_connected;
#endif
