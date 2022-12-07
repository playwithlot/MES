#ifndef __LM_SPLIT_2_H__
#define __LM_SPLIT_2_H__
#include "lower_moudle.h"

#define LM_SPLIT_2_dish_reset_index 1 //复位后的盘位号

#define LM_SPLIT_2_MOTOR_NUM_MAX 6

#define LM_CMD_SPLIT_2_mix_ctr ((uint32_t)0x0601)

#define LM_CMD_SPLIT_2_dish_rotate_1 ((uint32_t)0x0901) //盘转1格
#define LM_CMD_SPLIT_2_suck_waste ((uint32_t)0x0d01) //吸废液

#define LM_CMD_SPLIT_2_get_switch_ctr ((uint32_t)0x2804)
#define LM_CMD_SPLIT_2_get_opto_status ((uint32_t)0x2904)
#define LM_CMD_SPLIT_2_get_disc_rotate_pos ((uint32_t)0x2a04)
#define LM_CMD_SPLIT_2_get_lift_pos ((uint32_t)0x2b04)
#define LM_CMD_SPLIT_2_get_volume_factor ((uint32_t)0x2c04)
#define LM_CMD_SPLIT_2_get_volume2step_factor ((uint32_t)0x2d04)

#define LM_CMD_SPLIT_2_set_switch_ctr ((uint32_t)0x2803)
#define LM_CMD_SPLIT_2_set_disc_rotate_pos ((uint32_t)0x2a03)
#define LM_CMD_SPLIT_2_set_lift_pos ((uint32_t)0x2b03)
#define LM_CMD_SPLIT_2_set_volume_factor ((uint32_t)0x2c03)
#define LM_CMD_SPLIT_2_set_volume2step_factor ((uint32_t)0x2d03)

#pragma pack(1)
typedef struct
{
    Lower_moudle_t pub;
    // TODO:其它参数
    uint8_t motor_num;
    Motor_para_t motor_para[LM_SPLIT_2_MOTOR_NUM_MAX];
    uint8_t switch_ctr[2];
    int32_t position_para[5];
    uint8_t opto_status[8];
} LM_split2_t;
#pragma pack()

extern LM_split2_t split2_info;
#endif
