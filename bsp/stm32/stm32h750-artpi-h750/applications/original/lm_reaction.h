#ifndef __LM_REACTION_H__
#define __LM_REACTION_H__
#include "lower_moudle.h"

#define LM_REACTION_dish_reset_index 1 //复位后的盘位号

#define LM_REACTION_MOTOR_NUM_MAX 8 //电机个数

#define LM_CMD_REACTION_mix ((uint32_t)0x0601)            //混匀
#define LM_CMD_REACTION_add_cup ((uint32_t)0x0901)        //落杯到反应盘
#define LM_CMD_REACTION_dish_rotate_10 ((uint32_t)0x0a01) //反应盘转10格
#define LM_CMD_REACTION_get_switch_ctr ((uint32_t)0x2804)
#define LM_CMD_REACTION_get_opto_status ((uint32_t)0x2904)
#define LM_CMD_REACTION_get_position_para ((uint32_t)0x2a04) //位号

#define LM_CMD_REACTION_set_switch_ctr ((uint32_t)0x2803)
#define LM_CMD_REACTION_set_position_para ((uint32_t)0x2a03) //位号

#pragma pack(1)
//反应部
typedef struct
{
    Lower_moudle_t pub;
    uint8_t motor_num;
    Motor_para_t motor_para[LM_REACTION_MOTOR_NUM_MAX];
    uint8_t switch_ctr[6];
    int32_t rotate_steps[2]; //盘旋转位号步数
    uint8_t opto_status[12];
} LM_reaction_t;
#pragma pack()

extern LM_reaction_t reaction_info;

#endif
