#ifndef __NET_TEMP_H__
#define __NET_TEMP_H__

#include "net_protocol.h"

uint16_t net_pro_temp_recv_deal(Net_pack_deal_t *deal, uint32_t *err);
uint16_t net_pro_temp_get_temp(Net_pack_deal_t *deal, uint32_t *err);

#endif

