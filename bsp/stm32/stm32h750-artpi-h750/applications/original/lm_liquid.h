#ifndef __LM_LIQUID_H__
#define __LM_LIQUID_H__
#include "lower_moudle.h"

#define LM_LIQUID_MOTOR_NUM_MAX 1

// TODO:数量待统一
#define LM_LIQUID_PUMP_VOLUME_MAX 38

//泵阀位号
#define LIQUID_ctr_index_p9 ((uint8_t)1)
#define LIQUID_ctr_index_p12 ((uint8_t)2)
#define LIQUID_ctr_index_p14 ((uint8_t)3)
#define LIQUID_ctr_index_p13 ((uint8_t)4)
#define LIQUID_ctr_index_sv2 ((uint8_t)9)
#define LIQUID_ctr_index_sv3 ((uint8_t)10)
#define LIQUID_ctr_index_sv4 ((uint8_t)11)
#define LIQUID_ctr_index_sv6 ((uint8_t)13)
#define LIQUID_ctr_index_sv7 ((uint8_t)14)
#define LIQUID_ctr_index_sv14 ((uint8_t)27)

#define LM_CMD_LIQUID_ctr_valve ((uint32_t)0x0B01)
#define LM_CMD_LIQUID_set_valve ((uint32_t)0x0c01)

#define LM_CMD_LIQUID_get_switch_ctr ((uint32_t)0x2804)
#define LM_CMD_LIQUID_get_opto_status ((uint32_t)0x2904)
#define LM_CMD_LIQUID_get_volume_factor ((uint32_t)0x2a04)
#define LM_CMD_LIQUID_get_volume2step_factor ((uint32_t)0x2b04)
#define LM_CMD_LIQUID_get_pressure_detection_para ((uint32_t)0x2c04)
#define LM_CMD_LIQUID_get_current_pressure ((uint32_t)0x2d04)

#define LM_CMD_LIQUID_set_switch_ctr ((uint32_t)0x2803)
#define LM_CMD_LIQUID_set_volume_factor ((uint32_t)0x2a03)
#define LM_CMD_LIQUID_set_volume2step_factor ((uint32_t)0x2b03)
#define LM_CMD_LIQUID_set_pressure_detection_para ((uint32_t)0x2c03)

#define LM_CMD_LIQUID_set_pump_volume_ctr ((uint32_t)0x0b01)

#pragma pack(1)
typedef struct
{
    Lower_moudle_t pub;
    // TODO:其它参数
    uint8_t motor_num;
    Motor_para_t motor_para[LM_LIQUID_MOTOR_NUM_MAX];
    int32_t pos_para_32[6]; //体积补偿系数+体积转步数系数
    int16_t pos_para_16[3]; //气压检测参数
    uint8_t switch_ctr[17];
    uint8_t opto_status[17];
    uint16_t pressure_val; //真空罐气压
} LM_liquid_t;
#pragma pack()

extern LM_liquid_t liquid_info;

#endif
