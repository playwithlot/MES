#ifndef __LM_TEMP_H__
#define __LM_TEMP_H__
#include "lower_moudle.h"

#define LM_CMD_TEMP_get_temp_max ((uint32_t)0x0104)
#define LM_CMD_TEMP_get_temp_target ((uint32_t)0x0204)
#define LM_CMD_TEMP_get_temp_min ((uint32_t)0x0304)
#define LM_CMD_TEMP_get_temp_compensation ((uint32_t)0x0404)
#define LM_CMD_TEMP_get_temp_kp ((uint32_t)0x0504)
#define LM_CMD_TEMP_get_temp_ki ((uint32_t)0x0604)
#define LM_CMD_TEMP_get_temp_kd ((uint32_t)0x0704)
#define LM_CMD_TEMP_get_temp_kit ((uint32_t)0x0804)

#define LM_CMD_TEMP_get_switch_ctr ((uint32_t)0x0904)
#define LM_CMD_TEMP_get_opto_status    ((uint32_t)0x0a04)
#define LM_CMD_TEMP_get_tmp_val    ((uint32_t)0x0b04)

#define LM_CMD_TEMP_set_temp_max ((uint32_t)0x0103)
#define LM_CMD_TEMP_set_temp_target ((uint32_t)0x0203)
#define LM_CMD_TEMP_set_temp_min ((uint32_t)0x0303)
#define LM_CMD_TEMP_set_temp_compensation ((uint32_t)0x0403)
#define LM_CMD_TEMP_set_temp_kp ((uint32_t)0x0503)
#define LM_CMD_TEMP_set_temp_ki ((uint32_t)0x0603)
#define LM_CMD_TEMP_set_temp_kd ((uint32_t)0x0703)
#define LM_CMD_TEMP_set_temp_kit ((uint32_t)0x0803)
#define LM_CMD_TEMP_set_switch_ctr ((uint32_t)0x0903)


#pragma pack(1)
//温控参数
typedef struct TemperatureCtrPara 
{
    int16_t max;
    int16_t target;
    int16_t min;
    int16_t compensation;
    uint16_t kp;
    uint16_t ki;
    uint16_t kd;
    uint16_t kit;
}TemperatureCtrPara_t;
//温控
typedef struct
{
    Lower_moudle_t pub;
    //TODO:增加其它参数
    TemperatureCtrPara_t temp_ctr[9];
    uint8_t switch_ctr[21];//功能开关
    int16_t temp_val[11];
    uint8_t opto_status[1];
} LM_temp_t;
#pragma pack()

extern LM_temp_t temp_info;

uint32_t lm_temp_get_temp_ctr_para_recv_data_cb(struct Can_pack_deal *deal, struct Can_pack_deal *rec);

#endif
