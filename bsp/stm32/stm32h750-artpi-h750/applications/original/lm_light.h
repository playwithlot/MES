#ifndef __LM_LIGHT_H__
#define __LM_LIGHT_H__
#include "lower_moudle.h"

#define LM_LIGHT_MOTOR_NUM_MAX 9

#define t1_rotate_index_inside_33 1
#define t1_rotate_index_middle_36 2
#define t1_rotate_index_split 3
#define t1_rotate_index_split2 4
#define t1_rotate_index_drop_1 5
#define t1_rotate_index_drop_2 6

#define t2_rotate_index_inside_26 1
#define t2_rotate_index_inside_36 2
#define t2_rotate_index_middle_36 3
#define t2_rotate_index_pmt 4
#define t2_rotate_index_split_drop 5
#define t2_rotate_index_split_pmtinout 6
#define t2_rotate_index_outside_36 7

//电机标号
#define LIGHT_MT_index_t1_z 1
#define LIGHT_MT_index_t1_clamp 2
#define LIGHT_MT_index_t2_z 3
#define LIGHT_MT_index_t2_clamp 4
#define LIGHT_MT_index_t1_rotate 8
#define LIGHT_MT_index_t2_rotate 9

#define LM_CMD_LIGHT_t1_move_cup ((uint32_t)0x0801)
#define LM_CMD_LIGHT_t2_move_cup ((uint32_t)0x0901)
#define LM_CMD_LIGHT_t1_action ((uint32_t)0x0a01)
#define LM_CMD_LIGHT_t2_action ((uint32_t)0x0b01)
#define LM_CMD_LIGHT_t1_rotate_index ((uint32_t)0x0c01)
#define LM_CMD_LIGHT_t2_rotate_index ((uint32_t)0x0f01)
#define LM_CMD_LIGHT_pmt_door_ctr ((uint32_t)0x1201)
#define LM_CMD_LIGHT_pmt_shutter_ctr ((uint32_t)0x1301)

#define LM_CMD_LIGHT_mt_reset ((uint32_t)0x0102)

#define LM_CMD_LIGHT_get_switch_ctr ((uint32_t)0x2804)
#define LM_CMD_LIGHT_get_opto_status ((uint32_t)0x2904)
#define LM_CMD_LIGHT_get_t1_rotate_step ((uint32_t)0x2a04)
#define LM_CMD_LIGHT_get_t1_lift_step ((uint32_t)0x2b04)
#define LM_CMD_LIGHT_get_t1_claw_step ((uint32_t)0x2c04)
#define LM_CMD_LIGHT_get_t2_rotate_step ((uint32_t)0x2d04)
#define LM_CMD_LIGHT_get_t2_lift_step ((uint32_t)0x2e04)
#define LM_CMD_LIGHT_get_t2_claw_step ((uint32_t)0x2f04)
#define LM_CMD_LIGHT_get_add_alkali_step ((uint32_t)0x3004)
#define LM_CMD_LIGHT_get_window_step ((uint32_t)0x3104)
#define LM_CMD_LIGHT_get_volume_factor ((uint32_t)0x3204)
#define LM_CMD_LIGHT_get_volume2step_factor ((uint32_t)0x3304)

#define LM_CMD_LIGHT_set_switch_ctr ((uint32_t)0x2803)
#define LM_CMD_LIGHT_set_t1_rotate_step ((uint32_t)0x2a03)
#define LM_CMD_LIGHT_set_t1_lift_step ((uint32_t)0x2b03)
#define LM_CMD_LIGHT_set_t1_claw_step ((uint32_t)0x2c03)
#define LM_CMD_LIGHT_set_t2_rotate_step ((uint32_t)0x2d03)
#define LM_CMD_LIGHT_set_t2_lift_step ((uint32_t)0x2e03)
#define LM_CMD_LIGHT_set_t2_claw_step ((uint32_t)0x2f03)
#define LM_CMD_LIGHT_set_add_alkali_step ((uint32_t)0x3003)
#define LM_CMD_LIGHT_set_window_step ((uint32_t)0x3103)
#define LM_CMD_LIGHT_set_volume_factor ((uint32_t)0x3203)
#define LM_CMD_LIGHT_set_volume2step_factor ((uint32_t)0x3303)

#pragma pack(1)
typedef struct
{
    Lower_moudle_t pub;
    // TODO:其它参数
    uint8_t motor_num;
    Motor_para_t motor_para[LM_LIGHT_MOTOR_NUM_MAX];
    uint8_t switch_ctr[20];
    int32_t position_para[30];
    uint8_t opto_status[29];
} LM_light_t;
#pragma pack()

extern LM_light_t light_info;

#endif
