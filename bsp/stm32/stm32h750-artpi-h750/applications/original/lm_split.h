#ifndef __LM_SPLIT_H__
#define __LM_SPLIT_H__
#include "lower_moudle.h"

#define LM_SPLIT_dish_reset_index 1 //复位后的盘位号

#define LM_SPLIT_MOTOR_NUM_MAX 10

//电机号
#define SPLIT_MT_index_rotate 1
#define SPLIT_MT_index_updown 2
#define SPLIT_MT_index_pump_wriggle 3
#define SPLIT_MT_index_pump_a 4
#define SPLIT_MT_index_mix1 5
#define SPLIT_MT_index_mix2 6
#define SPLIT_MT_index_fourinject 7
#define SPLIT_MT_index_mix3 8
#define SPLIT_MT_index_mix4 9
#define SPLIT_MT_index_mix5 10

#define LM_CMD_SPLIT_mix_ctr ((uint32_t)0x0601)
#define LM_CMD_SPLIT_dish_rotate_index ((uint32_t)0x0701)
#define LM_CMD_SPLIT_liquid_z_index ((uint32_t)0x0801)
#define LM_CMD_SPLIT_dish_rotate_1 ((uint32_t)0x0901) //盘转1格
#define LM_CMD_SPLIT_set_valve ((uint32_t)0x0b01)
#define LM_CMD_SPLIT_fourinject_liquid_action ((uint32_t)0x0c01) // 4联注射器吸吐液
#define LM_CMD_SPLIT_suck_waste ((uint32_t)0x0d01)               //吸废液(负值方向才对)
#define LM_CMD_SPLIT_mt_rotate ((uint32_t)0x0602)

#define LM_CMD_SPLIT_get_switch_ctr ((uint32_t)0x2804)
#define LM_CMD_SPLIT_get_opto_status ((uint32_t)0x2904)
#define LM_CMD_SPLIT_get_rotate_setp ((uint32_t)0x2a04)
#define LM_CMD_SPLIT_get_lift_setp ((uint32_t)0x2b04)
#define LM_CMD_SPLIT_get_volume_factor ((uint32_t)0x2c04)
#define LM_CMD_SPLIT_get_volume2step_factor ((uint32_t)0x2d04)

#define LM_CMD_SPLIT_set_switch_ctr ((uint32_t)0x2803)
#define LM_CMD_SPLIT_set_rotate_setp ((uint32_t)0x2a03)
#define LM_CMD_SPLIT_set_lift_setp ((uint32_t)0x2b03)
#define LM_CMD_SPLIT_set_volume_factor ((uint32_t)0x2c03)
#define LM_CMD_SPLIT_set_volume2step_factor ((uint32_t)0x2d03)

#pragma pack(1)
typedef struct
{
    Lower_moudle_t pub;
    // TODO:其它参数
    uint8_t motor_num;
    Motor_para_t motor_para[LM_SPLIT_MOTOR_NUM_MAX];
    uint8_t switch_ctr[6];
    int32_t position_para[7];
    uint8_t opto_status[16];
} LM_split_t;
#pragma pack()

extern LM_split_t split_info;

#endif
