#ifndef __CAN_PROTOCOL_H__
#define __CAN_PROTOCOL_H__

#include "rtthread.h"
#include "connect.h"
#include "net_protocol.h"

//配置
#define CAN_SEND_TIMES_MAX 3
#define CAN_RESEND_TIME Msec(8) //重发间隔时间（节拍数）

//模块ID定义
#define LMOD_ID_temp 0X02
#define LMOD_ID_motor1 0X05
#define LMOD_ID_motor2 0X06
#define LMOD_ID_motor3 0X07

//指令类型
#define CAN_CMD_TYPE_work 0x01
#define CAN_CMD_TYPE_step 0x02
#define CAN_CMD_TYPE_set 0x03
#define CAN_CMD_TYPE_get 0x04
#define CAN_CMD_TYPE_test 0x05

// can命令错误码
#define CAN_CMD_ERR_succeed 0 //成功

// can命令的状态
#define CAN_STATE_init 0
#define CAN_STATE_delete 1
#define CAN_STATE_done 2
#define CAN_STATE_can_start 3
#define CAN_STATE_start 4
#define CAN_STATE_wait 5
#define CAN_STATE_timeout 6

// can帧类型
#define CAN_PACK_TYPE_cmd 1
#define CAN_PACK_TYPE_ack 2
#define CAN_PACK_TYPE_result 3
#define CAN_PACK_TYPE_data 4
#define CAN_PACK_TYPE_report 5
#define CAN_PACK_TYPE_heartbeat 6
#define CAN_PACK_TYPE_upgrate 7

//操作id的宏
#define GET_CAN_PACK_TYPE(x) (x & 0x07)
#define SET_CAN_PACK_TYPE(x, y)            \
    do                                     \
    {                                      \
        x = (x & 0x1ffffff8) | (y & 0x07); \
    } while (0)

#define GET_CAN_PACK_serial(x) ((x>>3)&0xff)
#define SET_CAN_PACK_serial(x, y)                 \
    do                                            \
    {                                             \
        x = (x & 0x1FFFF807) | ((y & 0xff) << 3); \
    } while (0)

#define GET_CAN_PACK_mserial(x) ((x>>11)&0xff)
#define SET_CAN_PACK_mserial(x, y)                 \
    do                                             \
    {                                              \
        x = (x & 0x1ff807ff) | ((y & 0xff) << 11); \
    } while (0)

#define GET_CAN_PACK_daddr(x) ((x>>19)&0x1f)
#define SET_CAN_PACK_daddr(x, y)                   \
    do                                             \
    {                                              \
        x = (x & 0x1f07ffff) | ((y & 0x1f) << 19); \
    } while (0)

#define GET_CAN_PACK_saddr(x) ((x>>24)&0x1f) //获取源目的地址
#define SET_CAN_PACK_saddr(x, y)                 \
    do                                           \
    {                                            \
        x = (x & 0xffffff) | ((y & 0x1f) << 24); \
    } while (0)

#pragma pack(1)
// can总线的一帧数据
typedef struct Can_cmd
{
    uint32_t id;
    uint8_t len; // buf有效数据长度
    uint8_t cmd1;
    uint8_t cmd2;
    uint8_t buf[6];
} Can_cmd_t;

//命令集定义
typedef struct Can_set
{
    uint8_t step;     //命令自身的步数（上层步骤控制时使用）,必须存在0步骤.
    uint8_t pro_addr; //下位机板子的can协议地址
    uint8_t len;      // can帧数据域长度
    uint16_t cmd;     //下位机命令
    uint8_t buf[6];   //下位机命令参数
    uint8_t flag;     //用于表示此命令与其它同级命令的处理关系。
} Can_set_t;

struct Can_pack_deal;
typedef struct Can_pack_deal
{
    Connect_t connect;
    uint8_t state;
    uint8_t last_state;
    uint32_t start_tick;
    uint32_t timeout_tick; //等待结果或数据的最长时间
    uint8_t flag:3;     //用于表示此命令与其它同级命令的处理关系。
    uint8_t can_port:3; //用于标识使用哪个can总线进行通信
    uint8_t is_take_res_lock : 1; //占有资源
    uint8_t is_unrelease_lock : 1; //出错时不释放资源
    uint32_t res_lock;            //所需占有的资源

    uint32_t err;
    uint8_t step;
    uint8_t cnt;           //重发计数
    uint8_t pack_len;
    Can_cmd_t *pack;
    void (*recv_state_deal)(struct Can_pack_deal *deal, struct Can_pack_deal *rec); //接收处理函数
    void (*state_deal)(struct Can_pack_deal *deal, void *data);
    void (*state_change_cb)(struct Can_pack_deal *deal, void *data);
    //收到数据应答(cmd)时的回调函数
    uint32_t (*recv_data_cb)(struct Can_pack_deal *deal,  struct Can_pack_deal *rec);
    //收到结果应答(cmd)时的回调函数
    uint32_t (*recv_result_cb)(struct Can_pack_deal *deal, struct Can_pack_deal *rec);
    uint32_t (*timeout_cb)(struct Can_pack_deal *deal, void *data);
} Can_pack_deal_t;

#pragma pack()

void can_pro_change_state(Can_pack_deal_t *deal, uint8_t new_state);
Can_pack_deal_t *can_pro_create(const Can_pack_deal_t *d);
uint8_t can_pro_add2line(Can_pack_deal_t *deal);
void can_pro_deal_thread(void *p);
void Can_recv_thread(void *p);
uint8_t can_pro_send(Can_pack_deal_t *cp);
uint32_t can_pro_pub_recv_result_cb(Can_pack_deal_t *deal, Can_pack_deal_t *rec);
void can_pro_pub_recv_deal(Can_pack_deal_t *deal, Can_pack_deal_t *rec);
void can_pro_pub_state_deal(Can_pack_deal_t *deal, void *data);
void can_pro_pub_state_change_cb(Can_pack_deal_t *deal, void *data);
Can_pack_deal_t *can_pro_create_by_cmd(Can_cmd_t *cmd);
Can_pack_deal_t *can_pro_create_by_set(Can_set_t *set);
Can_pack_deal_t *can_pro_create(const Can_pack_deal_t *d);
void can_pro_delete(Can_pack_deal_t *cp);

#endif
