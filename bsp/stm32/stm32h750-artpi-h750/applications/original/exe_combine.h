#ifndef __EXE_COMBINE_H__
#define __EXE_COMBINE_H__

#include "exe_deal.h"
#include "can_protocol.h"
#include "lower_moudle.h"

//组合动作命令标识
#define EXE_CMD_NULL 0
#define EXE_CMD_all_reset 1
#define EXE_CMD_s1_clean 2
#define EXE_CMD_r1_clean 3
#define EXE_CMD_r2_clean 4

#pragma pack(1)
typedef struct EcS1LiquidAction
{
    uint8_t simple_type;
    uint8_t simple_cup_type;
    uint8_t simple_cup_index;
    uint8_t simple_volume;
    uint8_t reaction_cup_index;
    uint8_t attenuation_cup_index; //稀释位
    uint8_t attenuation_volume;    //稀释量
    uint8_t reaction_cup_index2;
} EcS1LiquidAction_t;

typedef struct EcT3Action
{
    uint8_t reaction_index;
    uint8_t attenuation_index;  //稀释位
    uint8_t attenuation_index2; //稀释位
    uint8_t reaction_index2;
    uint8_t reaction_index3;
    uint8_t reaction_index4;
} EcT3Action_t;

typedef struct EcT1T2Action
{
    //抓手1
    uint8_t t1_dest_drop_cup; // 0不动作
    uint8_t t1_src_1_reaction;
    uint8_t t1_dest_1_split;
    uint8_t t1_src_2_split2;
    uint8_t t1_dest_2_reaction;
    uint8_t t1_src_3_reaction;
    uint8_t t1_dest_3_split2;
    //抓手2
    uint8_t t2_dest_pmt;
    uint8_t t2_src_1_reaction;
    uint8_t t2_dest_1_split;
    uint8_t t2_src_2_reaction;
    uint8_t t2_dest_2_reaction;
    uint8_t t2_dest_pmt_to_split;
} EcT1T2Action_t;

typedef struct EcR1R2Action
{
    uint8_t r1_dest_1_reaction;
    uint8_t r1_src_1_reagent_shelve;
    uint8_t r1_src_1_reagent_offset;
    uint8_t r1_volume1;
    uint8_t r1_dest_2_reaction;
    uint8_t r1_src_2_reagent_shelve;
    uint8_t r1_src_2_reagent_offset;
    uint8_t r1_volume2;
    uint8_t r2_dest_1_reaction;
    uint8_t r2_src_1_reagent_shelve;
    uint8_t r2_src_1_reagent_offset;
    uint8_t r2_volume1;
    uint8_t r2_dest_2_reaction;
    uint8_t r2_src_2_reagent_shelve;
    uint8_t r2_src_2_reagent_offset;
    uint8_t r2_volume2;
    uint8_t r3_dest_1_reaction;
    uint8_t r3_src_1_reagent_shelve;
    uint8_t r3_src_1_reagent_offset;
    uint8_t r3_volume1;
    uint8_t r3_dest_2_reaction;
    uint8_t r3_src_2_reagent_shelve;
    uint8_t r3_src_2_reagent_offset;
    uint8_t r3_volume2;
} EcR1R2Action_t;

typedef struct EcSplitAction
{
    uint8_t spilt_next_index;
    //四阶清洗通道选择
    uint8_t fourinject_ch1;
    uint8_t fourinject_ch2;
    uint8_t fourinject_ch3;
    uint8_t fourinject_ch4;
    uint8_t preinject_volume;
    //混匀力度等级
    uint8_t mix_level_1;
    uint8_t mix_level_2;
    uint8_t mix_level_3;
    uint8_t mix_level_4;
    uint8_t preinject_mix_level;
} EcSplitAction_t;

typedef struct EcSplit2Action
{
    uint8_t spilt2_next_index;
    //注液通道选择
    uint8_t fouinject_ch1;
    uint8_t fouinject_ch2;
    //混匀力度等级
    uint8_t mix_level_1;
    uint8_t mix_level_2;
} EcSplit2Action_t;
#pragma pack()

char *exe_get_cmd_name(Exe_cls_t *e);
Exe_cls_t *ec_delay(uint32_t delay_tick);
void exe_some_stop(uint8_t some);
Exe_cls_t *ec_allreset(void);
Exe_cls_t *ec_dish_run(void);
Exe_cls_t *ec_s1_liquid_action(EcS1LiquidAction_t *p);
Exe_cls_t *ec_t3_action(EcT3Action_t *p);
Exe_cls_t *ec_t1_t2_action(EcT1T2Action_t *p);
Exe_cls_t *ec_r1_r2_action(EcR1R2Action_t *p);
Exe_cls_t *ec_split_action(EcSplitAction_t *p);
Exe_cls_t *ec_split2_action(EcSplit2Action_t *p);
Exe_cls_t *ec_inspire_action_and_pmt(uint32_t volume, uint32_t pmt_times);

#endif
