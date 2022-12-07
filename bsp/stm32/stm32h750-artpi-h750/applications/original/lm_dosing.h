#ifndef __LM_DOSING_H__
#define __LM_DOSING_H__
#include "lower_moudle.h"

#define LM_DOSING_dish_reset_index 1 //复位后的盘位号

#define LM_DOSING_MOTOR_NUM_MAX 6 //电机个数

//参照位号表
#define S1_rotate_index_outside_1 1
#define S1_rotate_index_outside_81 2
#define S1_rotate_index_attenuation_outside_dish_9 3 //稀释外盘吸液位
#define S1_rotate_index_attenuation_dish_1 4         //稀释盘吸液位
#define S1_rotate_index_clean 5                      //清洗位
#define S1_rotate_index_normal_simple 6
#define S1_rotate_index_emergency_simple 7

//升降位号表
#define S1_z_index_clean 4

// t3抓位号
#define T3_rotate_index_out_11 1
#define T3_rotate_index_out_22 2
#define T3_rotate_index_middle_22 3
#define T3_rotate_index_attenuation_outside_1 4

//电机标号
#define DOSING_MT_index_s1_z 1
#define DOSING_MT_index_pump_p1 2
#define DOSING_MT_index_t3_z 3

#define LM_CMD_DOSING_select_simple_cup_type ((uint32_t)0x0801)
#define LM_CMD_DOSING_s1_move_liquid ((uint32_t)0x0a01)
#define LM_CMD_DOSING_s1_adjust ((uint32_t)0x0d01)
#define LM_CMD_DOSING_s1_rotate_index ((uint32_t)0x1001)
#define LM_CMD_DOSING_s1_z_index ((uint32_t)0x1101)
#define LM_CMD_DOSING_t3_move_cup ((uint32_t)0x1201)
#define LM_CMD_DOSING_inside_dish_rotate_index ((uint32_t)0x1701) //内圈转格
#define LM_CMD_DOSING_outside_dish_rotate_8 ((uint32_t)0x1901)    //外圈转8格

#define LM_CMD_DOSING_mt_reset ((uint32_t)0x0102)

#define LM_CMD_DOSING_get_switch_ctr ((uint32_t)0x2804)
#define LM_CMD_DOSING_get_opto_status ((uint32_t)0x2904)
#define LM_CMD_DOSING_get_s1_rotate_step ((uint32_t)0x2a04)
#define LM_CMD_DOSING_get_s1_lift_step ((uint32_t)0x2b04)
#define LM_CMD_DOSING_get_t3_rotate_step ((uint32_t)0x2c04)
#define LM_CMD_DOSING_get_t3_lift_step ((uint32_t)0x2d04)
#define LM_CMD_DOSING_get_t3_claw_step ((uint32_t)0x2e04)
#define LM_CMD_DOSING_get_dilute_rotate_step ((uint32_t)0x2f04)
#define LM_CMD_DOSING_get_reaction_rotate_step ((uint32_t)0x3004)
#define LM_CMD_DOSING_get_volume_factor ((uint32_t)0x3104)
#define LM_CMD_DOSING_get_volume2step_factor ((uint32_t)0x3204)
#define LM_CMD_DOSING_get_sample_needle_para ((uint32_t)0x3304)
#define LM_CMD_DOSING_get_lv_detect_max_step ((uint32_t)0x3404)
#define LM_CMD_DOSING_get_lv_detect_para ((uint32_t)0x3504)
#define LM_CMD_DOSING_get_plug_needle_para ((uint32_t)0x3604)
#define LM_CMD_DOSING_get_LV_detect_val ((uint32_t)0x3704)
#define LM_CMD_DOSING_get_current_pressure ((uint32_t)0x3804)

#define LM_CMD_DOSING_set_switch_ctr ((uint32_t)0x2803)
#define LM_CMD_DOSING_set_s1_rotate_step ((uint32_t)0x2a03)
#define LM_CMD_DOSING_set_s1_lift_step ((uint32_t)0x2b03)
#define LM_CMD_DOSING_set_t3_rotate_step ((uint32_t)0x2c03)
#define LM_CMD_DOSING_set_t3_lift_step ((uint32_t)0x2d03)
#define LM_CMD_DOSING_set_t3_claw_step ((uint32_t)0x2e03)
#define LM_CMD_DOSING_set_dilute_rotate_step ((uint32_t)0x2f03)
#define LM_CMD_DOSING_set_reaction_rotate_step ((uint32_t)0x3003)
#define LM_CMD_DOSING_set_volume_factor ((uint32_t)0x3103)
#define LM_CMD_DOSING_set_volume2step_factor ((uint32_t)0x3203)
#define LM_CMD_DOSING_set_sample_needle_para ((uint32_t)0x3303)
#define LM_CMD_DOSING_set_lv_detect_max_step ((uint32_t)0x3403)
#define LM_CMD_DOSING_set_lv_detect_para ((uint32_t)0x3503)
#define LM_CMD_DOSING_set_plug_needle_para ((uint32_t)0x3603)

#pragma pack(1)
//加样部, 具体含义需要查看相关协议文档
typedef struct
{
    Lower_moudle_t pub;
    uint8_t motor_num;
    Motor_para_t motor_para[LM_DOSING_MOTOR_NUM_MAX];
    uint8_t switch_ctr[12];
    int32_t position_para_32[45];
    int16_t position_para_16[10];
    uint8_t opto_status[20];
    uint16_t s1_pressure;
    int16_t LV_AD_val;
} LM_dosing_t;
#pragma pack()

extern LM_dosing_t dosing_info;

#endif // !__LM_DOSING_H__
