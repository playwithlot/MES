#ifndef __LM_REAGENT_H__
#define __LM_REAGENT_H__
#include "lower_moudle.h"

#define LM_REAGENT_dish_reset_index 1 //复位后的盘位号

#define LM_REAGENT_MOTOR_NUM_MAX 10

// r1针旋转位号
#define r1_rotate_index_reagent_1 1
#define r1_rotate_index_reagent_2 2
#define r1_rotate_index_reagent_3 3
#define r1_rotate_index_reagent_4 4
#define r1_rotate_index_clean 5
#define r1_rotate_index_out_51 6

//升降
#define r1_z_index_reagent_suck 1
#define r1_z_index_clean 2
#define r1_z_index_reaction_spit 3

#define r2_rotate_index_reagent_5 1
#define r2_rotate_index_reagent_6 2
#define r2_rotate_index_reagent_7 3
#define r2_rotate_index_reagent_8 4
#define r2_rotate_index_clean 5
#define r2_rotate_index_out_51 6
#define r2_rotate_index_inside_43 7

//升降
#define r2_z_index_reagent_suck 1
#define r2_z_index_clean 2
#define r2_z_index_reaction_spit 3

//盘转位号
#define reagent_dis_rotate_index_1 1
#define reagent_dis_rotate_index_2 2
#define reagent_dis_rotate_index_3 3
#define reagent_dis_rotate_index_4 4
#define reagent_dis_rotate_index_5 5
#define reagent_dis_rotate_index_6 6
#define reagent_dis_rotate_index_7 7
#define reagent_dis_rotate_index_8 8

//电机位号
#define REAGENT_mt_index_r1_z 1
#define REAGENT_mt_index_r1_p2 2
#define REAGENT_mt_index_r2_z 3
#define REAGENT_mt_index_r2_p3 4
#define REAGENT_mt_index_r1_rotate 8
#define REAGENT_mt_index_r2_rotate 9

#define LM_CMD_REAGENT_r1_move_liquid ((uint32_t)0x0a01)
#define LM_CMD_REAGENT_r2_move_liquid ((uint32_t)0x0b01)
#define LM_CMD_REAGENT_r1_liquic_action ((uint32_t)0x0d01)
#define LM_CMD_REAGENT_r2_liquic_action ((uint32_t)0x0e01)
#define LM_CMD_REAGENT_r1_rotate_index ((uint32_t)0x1301)
#define LM_CMD_REAGENT_r2_rotate_index ((uint32_t)0x1401)

#define LM_CMD_REAGENT_r1_z_index ((uint32_t)0x1601)
#define LM_CMD_REAGENT_r2_z_index ((uint32_t)0x1701)

#define LM_CMD_REAGENT_dish_rotate_index ((uint32_t)0x1901) //反应盘转到指定的位置
#define LM_CMD_REAGENT_dish_rotate_5 ((uint32_t)0x1a01)     //反应盘转5格

#define LM_CMD_REAGENT_mt_reset ((uint32_t)0x0102)

#define LM_CMD_REAGENT_get_switch_ctr ((uint32_t)0x2804)
#define LM_CMD_REAGENT_get_opto_status ((uint32_t)0x2904)
#define LM_CMD_REAGENT_get_r1_rotate_step ((uint32_t)0x2a04)
#define LM_CMD_REAGENT_get_r2_rotate_step ((uint32_t)0x2b04)
#define LM_CMD_REAGENT_get_r3_rotate_step ((uint32_t)0x2c04)
#define LM_CMD_REAGENT_get_lift_step ((uint32_t)0x2d04)
#define LM_CMD_REAGENT_get_disc_rotate_step ((uint32_t)0x2e04)
#define LM_CMD_REAGENT_get_volume_factor ((uint32_t)0x2f04)
#define LM_CMD_REAGENT_get_volume2step_factor ((uint32_t)0x3004)
#define LM_CMD_REAGENT_get_reagent_needle_para ((uint32_t)0x3104)
#define LM_CMD_REAGENT_get_lv_detect_max_position ((uint32_t)0x3204)
#define LM_CMD_REAGENT_get_lv_detect_para ((uint32_t)0x3304)
#define LM_CMD_REAGENT_get_current_AD_val ((uint32_t)0x3404)

#define LM_CMD_REAGENT_set_switch_ctr ((uint32_t)0x2803)
#define LM_CMD_REAGENT_set_r1_rotate_step ((uint32_t)0x2a03)
#define LM_CMD_REAGENT_set_r2_rotate_step ((uint32_t)0x2b03)
#define LM_CMD_REAGENT_set_r3_rotate_step ((uint32_t)0x2c03)
#define LM_CMD_REAGENT_set_lift_step ((uint32_t)0x2d03)
#define LM_CMD_REAGENT_set_disc_rotate_step ((uint32_t)0x2e03)
#define LM_CMD_REAGENT_set_volume_factor ((uint32_t)0x2f03)
#define LM_CMD_REAGENT_set_volume2step_factor ((uint32_t)0x3003)
#define LM_CMD_REAGENT_set_reagent_needle_para ((uint32_t)0x3103)
#define LM_CMD_REAGENT_set_lv_detect_max_position ((uint32_t)0x3203)
#define LM_CMD_REAGENT_set_lv_detect_para ((uint32_t)0x3303)

#pragma pack(1)
typedef struct
{
    Lower_moudle_t pub;
    // TODO:其它参数
    uint8_t motor_num;
    Motor_para_t motor_para[LM_REAGENT_MOTOR_NUM_MAX];
    uint8_t switch_ctr[14];
    int32_t position_para_32[50];
    int16_t position_para_16[12];
    int16_t current_AD[3];
    uint8_t opto_status[24];
} LM_reagent_t;
#pragma pack()

extern LM_reagent_t reagent_info;

#endif
