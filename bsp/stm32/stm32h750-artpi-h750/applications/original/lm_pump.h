#ifndef __LM_PUMP_H__
#define __LM_PUMP_H__
#include "lower_moudle.h"

#define LM_CMD_PUMP_INDEX_split_mt 0x01
#define LM_CMD_PUMP_INDEX_lni_rinse 0x02
#define LM_CMD_PUMP_INDEX_rni_rinse 0x03
#define LM_CMD_PUMP_INDEX_lno_rinse 0x04
#define LM_CMD_PUMP_INDEX_rno_rinse 0x05
#define LM_CMD_PUMP_INDEX_rinse_letout 0x06
#define LM_CMD_PUMP_INDEX_rinse_feed_liquor 0x07
#define LM_CMD_PUMP_INDEX_valve_lni_rinse 0x08 //左针内壁阀
#define LM_CMD_PUMP_INDEX_valve_rni_rinse 0x09
#define LM_CMD_PUMP_INDEX_valve_lno_rinse 0x0a
#define LM_CMD_PUMP_INDEX_valve_rno_rinse 0x0b

#define LM_CMD_PUMP_pour ((uint32_t)0x0301)
#define LM_CMD_PUMP_action ((uint32_t)0x0401)
#define LM_CMD_PUMP_pour_full ((uint32_t)0x0501)
#define LM_CMD_PUMP_all_rest ((uint32_t)0x601)
#define LM_CMD_PUMP_four_inject_channel1 ((uint8_t)0x01)
#define LM_CMD_PUMP_four_inject_channel2 ((uint8_t)0x02)
#define LM_CMD_PUMP_four_inject_channel3 ((uint8_t)0x04)
#define LM_CMD_PUMP_four_inject_channel4 ((uint8_t)0x08)
#define LM_CMD_PUMP_four_inject_channel_action ((uint32_t)0x701)

#define LM_CMD_PUMP_set_ignore_switch ((uint32_t)0x0201)
#define LM_CMD_PUMP_get_ignore_switch ((uint32_t)0x0403)
#define LM_CMD_PUMP_set_io ((uint32_t)0x0102)

#define LM_CMD_PUMP_set_hold_time ((uint32_t)0x0103)
#define LM_CMD_PUMP_get_hold_time ((uint32_t)0x0104)

#define LM_CMD_PUMP_get_optocoupler_status ((uint32_t)0x0204)

#pragma pack(1)
typedef struct
{
    Lower_moudle_t pub;
    uint16_t hold_time[21];        //持续时间，单位（ms)
    uint8_t optocoupler_status[7]; //光耦状态
    uint8_t ignore_switch[6];      //报警或是否忽略开关。
} LM_pump_t;
#pragma pack()

extern LM_pump_t pump_info;
#define PUMP_ADDR pump_info.pub.pro_addr

#endif
