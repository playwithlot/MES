#ifndef __UDP_RECEIVE_H__
#define __UDP_RECEIVE_H__

#include "net_protocol.h"
void udp_recv_thread(void *paramemter);
int net_udp_send(Net_pack_deal_t *deal);

#endif





