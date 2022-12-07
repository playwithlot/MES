#ifndef __LOWER_MOUDLE_H__
#define __LOWER_MOUDLE_H__

#include "rtthread.h"
#include "line.h"
#include "can_protocol.h"
#include "iot_parameter.h"
#include "exe_deal.h"

#define LM_ERR_succeed ((uint8_t)0)
#define LM_ERR_para_err ((uint8_t)1)
#define LM_ERR_time_out ((uint8_t)2)

//与下位机通信时的can协议中的地址
#define CAN_MASTER_ADDR 23 // iot_para.can_addr //中位机can地址可配置
//#define CAN_MASTER_ADDR 23 // iot_para.can_addr //中位机can地址可配置, 振金板地址
//#define CAN_MASTER_ADDR 18 // iot_para.can_addr //中位机can地址可配置,临时测试用
#define CAN_TEMP_ADDR 1
#define CAN_LIQUID_ADDR 2
#define CAN_REACTION_ADDR 3
#define CAN_REAGENT_ADDR 4
#define CAN_SPLIT_2_ADDR 5
#define CAN_SPLIT_ADDR 6
#define CAN_LIGHT_ADDR 7
#define CAN_DOSING_ADDR 8

#define LM_CMD_set ((uint32_t)0x03)
#define LM_CMD_get ((uint32_t)0x04)

//下位机的命令，2字节，其它认为是参数
#define LM_CMD_handshake ((uint32_t)0x0101)
#define LM_CMD_reset_para ((uint32_t)0x0201)
#define LM_CMD_allreset ((uint32_t)0x0301)
#define LM_CMD_get_version ((uint32_t)0x0401)
#define LM_CMD_reboot ((uint32_t)0x0501)

//电机设置与查询通用命令
#define LM_CMD_set_mt_r_vstart ((uint32_t)0x000103)
#define LM_CMD_set_mt_r_a1 ((uint32_t)0x000203)
#define LM_CMD_set_mt_r_v1 ((uint32_t)0x000303)
#define LM_CMD_set_mt_r_amax ((uint32_t)0x000403)
#define LM_CMD_set_mt_r_vmax ((uint32_t)0x000503)
#define LM_CMD_set_mt_r_dmax ((uint32_t)0x000603)
#define LM_CMD_set_mt_r_d1 ((uint32_t)0x000703)
#define LM_CMD_set_mt_r_vstop ((uint32_t)0x000803)

#define LM_CMD_set_mt_run1_vstart ((uint32_t)0x000903)
#define LM_CMD_set_mt_run1_a1 ((uint32_t)0x000a03)
#define LM_CMD_set_mt_run1_v1 ((uint32_t)0x000b03)
#define LM_CMD_set_mt_run1_amax ((uint32_t)0x000c03)
#define LM_CMD_set_mt_run1_vmax ((uint32_t)0x000d03)
#define LM_CMD_set_mt_run1_dmax ((uint32_t)0x000e03)
#define LM_CMD_set_mt_run1_d1 ((uint32_t)0x000f03)
#define LM_CMD_set_mt_run1_vstop ((uint32_t)0x001003)

#define LM_CMD_set_mt_run2_vstart ((uint32_t)0x001103)
#define LM_CMD_set_mt_run2_a1 ((uint32_t)0x001203)
#define LM_CMD_set_mt_run2_v1 ((uint32_t)0x001303)
#define LM_CMD_set_mt_run2_amax ((uint32_t)0x001403)
#define LM_CMD_set_mt_run2_vmax ((uint32_t)0x001503)
#define LM_CMD_set_mt_run2_dmax ((uint32_t)0x001603)
#define LM_CMD_set_mt_run2_d1 ((uint32_t)0x001703)
#define LM_CMD_set_mt_run2_vstop ((uint32_t)0x001803)

#define LM_CMD_set_mt_run3_vstart ((uint32_t)0x001903)
#define LM_CMD_set_mt_run3_a1 ((uint32_t)0x001a03)
#define LM_CMD_set_mt_run3_v1 ((uint32_t)0x001b03)
#define LM_CMD_set_mt_run3_amax ((uint32_t)0x001c03)
#define LM_CMD_set_mt_run3_vmax ((uint32_t)0x001d03)
#define LM_CMD_set_mt_run3_dmax ((uint32_t)0x001e03)
#define LM_CMD_set_mt_run3_d1 ((uint32_t)0x0001f03)
#define LM_CMD_set_mt_run3_vstop ((uint32_t)0x002003)

#define LM_CMD_set_mt_zero_compensation ((uint32_t)0x002103)
#define LM_CMD_set_mt_code_compensation ((uint32_t)0x002203)
#define LM_CMD_set_mt_reset_step ((uint32_t)0x002303)
#define LM_CMD_set_mt_max_step ((uint32_t)0x002403)
#define LM_CMD_set_mt_max_time ((uint32_t)0x002503)
#define LM_CMD_set_mt_coder_rpm ((uint32_t)0x002603)
#define LM_CMD_set_mt_coder_num ((uint32_t)0x002703)

#define LM_CMD_get_mt_r_vstart ((uint32_t)0x000104)
#define LM_CMD_get_mt_r_a1 ((uint32_t)0x000204)
#define LM_CMD_get_mt_r_v1 ((uint32_t)0x000304)
#define LM_CMD_get_mt_r_amax ((uint32_t)0x000404)
#define LM_CMD_get_mt_r_vmax ((uint32_t)0x000504)
#define LM_CMD_get_mt_r_dmax ((uint32_t)0x000604)
#define LM_CMD_get_mt_r_d1 ((uint32_t)0x000704)
#define LM_CMD_get_mt_r_vstop ((uint32_t)0x000804)
#define LM_CMD_get_mt_coder_num ((uint32_t)0x002704)

//电机调试通用命令
#define LM_CMD_motor_test_enable ((uint32_t)0x000202)
#define LM_CMD_motor_test_break ((uint32_t)0x000302)
#define LM_CMD_motor_test_reset ((uint32_t)0x000102)
#define LM_CMD_motor_test_run_abs ((uint32_t)0x000402)
#define LM_CMD_motor_test_run_rel ((uint32_t)0x000502)
#define LM_CMD_motor_test_rotate ((uint32_t)0x000602)

//错误码
#define LM_CMD_ERR_succeed 0 //
#define LM_CMD_ERR_faile 1   //

//盘标识
#define DIS_reaction 1        //反应盘
#define DIS_split 2           // 1步法盘
#define DIS_split2 3          // 2步法盘
#define DIS_attenuation_out 4 //稀释盘外圈

#pragma pack(1)

//命令映射表
typedef struct lower_moudle_cmd_map
{
    uint32_t cmd;          //命令
    uint32_t timeout_tick; //命令执行超时时间（从发出命令到收到结果的时间）
    //接收处理函数
    void (*recv_state_deal)(struct Can_pack_deal *deal, struct Can_pack_deal *rec);
    //主状态循环处理
    void (*state_deal)(struct Can_pack_deal *deal, void *data);
    //状态变更回调函数
    void (*state_change_cb)(struct Can_pack_deal *deal, void *data);
    //收到数据应答时的回调函数
    uint32_t (*recv_data_cb)(struct Can_pack_deal *deal, struct Can_pack_deal *rec);
    //收到结果应答时的回调函数
    uint32_t (*recv_result_cb)(struct Can_pack_deal *deal, struct Can_pack_deal *rec);
    uint32_t (*timeout_cb)(struct Can_pack_deal *deal, void *data);
} lower_moudle_cmd_map_t;

//针对硬件板子所含功能定义的类结构
typedef struct
{
    uint8_t pro_addr; //协议地址
    uint8_t serial;   //帧序号
    uint8_t soft_version[4];
    uint8_t hard_version[4];
    uint8_t id_code[4];
    uint8_t faile_comm_cnt;
    uint8_t state;
    uint16_t map_size;
    uint8_t log_level;
#define LM_RES_LOCK_null ((uint32_t)0x00)
#define LM_RES_LOCK_all ((uint32_t)0xffffffff)
    volatile uint32_t res_lock;               //每位表示一个资源锁
    lower_moudle_cmd_map_t *map;              //命令映射表
    void (*get_res_lock)(Can_pack_deal_t *d); //根据命令获取对应的资源锁(每个下位机模块相互独立)
} Lower_moudle_t;                             //模块公共参数

#pragma pack(1)
//电机基本参数结构体(需要注意元素的位置)
typedef struct
{
    uint32_t reset_vstart;           //起始速度
    uint32_t reset_a1;               //第一段加速度
    uint32_t reset_v1;               //节点速度
    uint32_t reset_amax;             //第二段加速度
    uint32_t reset_vmax;             //运行速度
    uint32_t reset_dmax;             //第一段减速度
    uint32_t reset_d1;               //第二段减速度
    uint32_t reset_vstop;            //停止速度
    uint32_t area_1_vstart;          //起始速度
    uint32_t area_1_a1;              //第一段加速度
    uint32_t area_1_v1;              //节点速度
    uint32_t area_1_amax;            //第二段加速度
    uint32_t area_1_vmax;            //运行速度
    uint32_t area_1_dmax;            //第一段减速度
    uint32_t area_1_d1;              //第二段减速度
    uint32_t area_1_vstop;           //停止速度
    uint32_t area_2_vstart;          //起始速度
    uint32_t area_2_a1;              //第一段加速度
    uint32_t area_2_v1;              //节点速度
    uint32_t area_2_amax;            //第二段加速度
    uint32_t area_2_vmax;            //运行速度
    uint32_t area_2_dmax;            //第一段减速度
    uint32_t area_2_d1;              //第二段减速度
    uint32_t area_2_vstop;           //停止速度
    uint32_t area_3_vstart;          //起始速度
    uint32_t area_3_a1;              //第一段加速度
    uint32_t area_3_v1;              //节点速度
    uint32_t area_3_amax;            //第二段加速度
    uint32_t area_3_vmax;            //运行速度
    uint32_t area_3_dmax;            //第一段减速度
    uint32_t area_3_d1;              //第二段减速度
    uint32_t area_3_vstop;           //停止速度
    int32_t zero_compensation;       //零点光耦补偿
    int32_t code_wheel_compensation; //码盘光耦补偿
    int32_t reset_compensation;      //复位光耦补偿
    int32_t max_route;               //最大行程
    uint32_t timeout;                //最大运行时间
    uint32_t coder_cnt;              //编码器每转计数
    uint32_t coder_num;              //当前编码器数
} Motor_para_t;
#pragma pack()
typedef struct Lm_config
{
    // TODO:配置参数,需要掉电保存
    uint32_t reaction_mix_speed; //反应盘位混匀速度系数
    uint32_t split_mix_speed;    //一步法盘位混匀速度系数
    uint32_t split2_mix_speed;   // 2步法盘位混匀速度系数
    uint32_t magic;
} Lm_config_t;

typedef struct Lm_cup_status
{
    uint8_t is_not_empty;      //是否为空
#define REACTION_cup_pos_72 72 //反应盘落杯位
    uint16_t position;         //当前所在空间物理位置(以复位后的状态为绝对位置)
    uint16_t volume;           //杯子中已经存在的液体量
} Lm_cup_status_t;

//所有下位机的公共资源状态
typedef struct Lm_status
{
    Lm_config_t config;
    Lower_moudle_t *md[8]; //下位机个数
#define CUP_reaction_num ((uint8_t)81 * 3)
#define CUP_split_num ((uint8_t)40)
#define CUP_split2_num ((uint8_t)24)
#define CUP_attenuation_out_num ((uint8_t)21)
    Lm_cup_status_t cup_reaction[CUP_reaction_num];                         //反应盘杯子状态
    Lm_cup_status_t cup_split[CUP_split_num];                                //一步法磁分离盘杯子状态
    Lm_cup_status_t cup_split2[CUP_split2_num];                               // 2步法磁分离盘杯子状态
    Lm_cup_status_t cup_attenuation_out[CUP_attenuation_out_num];                      //稀释液外盘杯子状态
    Lm_cup_status_t cup_reagent[34][4];                           //试剂盘杯子状态
    Lm_cup_status_t cup_attenuation[15];                          //稀释液杯子状态
#define LM_RES_LOCK_FLAG_null 0x00                                //空资源
#define LM_RES_LOCK_FLAG_s1 0x01                                  // s1针
#define LM_RES_LOCK_FLAG_t3 0x02                                  // t3抓手
#define LM_RES_LOCK_FLAG_t1_t2 0x04                               // t1 t2抓手
#define LM_RES_LOCK_FLAG_r1 0x08                                  // r1 针
#define LM_RES_LOCK_FLAG_r2 0x10                                  // r2针
#define LM_RES_LOCK_FLAG_r1_r2 (0x08 | 0x10)                      // r1 r2针
#define LM_RES_LOCK_FLAG_reagent_dish 0x20                        //试剂盘
#define LM_RES_LOCK_FLAG_reaction_dish 0x40                       //反应盘
#define LM_RES_LOCK_FLAG_split_dish 0x80                          //一步法磁分离盘
#define LM_RES_LOCK_FLAG_split2_dish 0x0100                       // 2步法磁分离盘
#define LM_RES_LOCK_FLAG_attenuation_out_dish 0x0200              //稀释外盘
#define LM_RES_LOCK_FLAG_dish_run (0x40 | 0x80 | 0x0100 | 0x0200) //周期盘转命令
#define LM_RES_LOCK_FLAG_pmt 0x0400                               // pmt测光
#define LM_RES_LOCK_FLAG_all 0xffffffff                           //全部资源
    volatile uint32_t res_lock;                                   //每位表示一个资源锁
    volatile uint8_t is_all_reset;
} Lm_status_t;

#pragma pack()

extern Lm_status_t lm_info;

int lmod_init(void);
void lmod_clear_all_line(void);
void lmod_set_ops_and_serial(Can_pack_deal_t *d);
uint32_t lm_pub_recv_result_cb(Can_pack_deal_t *deal, Can_pack_deal_t *rec);
Exe_cls_t *lmode_create_exe_by_set(Can_set_t *set, uint8_t set_size);
uint8_t lmod_netaddr2canaddr(uint8_t net_addr);
Motor_para_t *lmod_get_mt_para(uint8_t addr, uint8_t mt_index);
Lower_moudle_t *lmod_get_pub_by_addr(uint8_t addr);
uint32_t lm_pub_get_mt_para_recv_data_cb(struct Can_pack_deal *deal, struct Can_pack_deal *rec);
Lm_cup_status_t *lm_get_cup_status(uint8_t which_dis, uint16_t position, uint8_t *index);
uint8_t lm_is_cup_next_status_right(uint8_t which_dis, uint16_t next_position, uint8_t rotate_num);

#endif
