#ifndef __NET_PROTOCOL_H__
#define __NET_PROTOCOL_H__

#include "rtthread.h"
#include "connect.h"

//传输时常规类型转换
#define FLOAT_TO_INT_ZOOM(x) ((int32_t)(x*100000))
#define INT_TO_FLOAT_ZOOM(x) ((float)(x/100000.0))
#define FLOAT_TO_UINT_ZOOM(x) ((uint32_t)(x*100000))
#define UINT_TO_FLOAT_ZOOM(x) ((float)(x/100000.0))

#define NET_SEND_TIMES_MAX 3

//帧类型
#define NET_PACK_TYPE_cmd 0x01
#define NET_PACK_TYPE_ack 0x02
#define NET_PACK_TYPE_result 0x03
#define NET_PACK_TYPE_data 0x04
#define NET_PACK_TYPE_report 0x05
#define NET_PACK_TYPE_heartbeat 0x06

//错误码(第1字节)
#define NET_CMD_ERR_succeed 0x00
#define NET_CMD_ERR_crc_err 0x01
#define NET_CMD_ERR_unknow_cmd 0x02
#define NET_CMD_ERR_unknow_type 0x02
#define NET_CMD_ERR_para 0x03
#define NET_CMD_ERR_no_memery 0x04
#define NET_CMD_ERR_exe_faile 0x05
#define NET_CMD_ERR_timeout 0x06
#define NET_CMD_ERR_force_stop 0x07
#define NET_CMD_ERR_head_err 0x01 // 0x06
#define NET_CMD_ERR_len_err 0x01  // 0x07

//错误码（第2字节）
#define NET_CMD_ERR_clamp_pickup 0x00000100  //没夹起
#define NET_CMD_ERR_clamp_drop 0x00000200    //没放下
#define NET_CMD_ERR_clamp_had_cup 0x00000300 //夹抓上有杯
#define NET_CMD_ERR_no_suck 0x00000400       //单针未吸液
#define NET_CMD_ERR_suck 0x00000500       //单针已吸液
#define NET_CMD_ERR_ls_rdone 0x00000600        //双针动作时，左吸液，右成功
#define NET_CMD_ERR_lns_rdone 0x00000700        //双针动作时，左未吸液，右成功
#define NET_CMD_ERR_ldone_rs 0x00000800        //双针动作时，左成功，右吸液
#define NET_CMD_ERR_ldone_rns 0x00000900        //双针动作时，左成功，右未吸液
#define NET_CMD_ERR_lns_rns 0x00000a00        //双针动作时，均失败，左未吸，右未吸
#define NET_CMD_ERR_ls_rs 0x00000b00        //双针动作时，均失败，左吸，右吸
#define NET_CMD_ERR_lns_rs 0x00000c00        //双针动作时，均失败，左未吸，右吸
#define NET_CMD_ERR_ls_rns 0x00000d00        //双针动作时，均失败，左吸，右未吸

//错误码（第3字节，中位机使用）
#define EXE_ERR_err_deal 0x010000
#define EXE_ERR_force_stop 0x020000
//错误码（第4字节，下位机使用）


//传输类型,主要用于标识收到的命令来自哪个接收口,回应时使用对应的发送口进行回应
#define TRANSFER_TYPE_tcp 0               //使用tcp传输
#define TRANSFER_TYPE_udp 1               //使用udp 单播socket传输
#define TRANSFER_TYPE_udp_broadcast 2     //使用udp 广播socket传输
#define TRANSFER_TYPE_udp_mutibroadcast 3 //使用udp组播socket传输
#define TRANSFER_TYPE_can 4               //使用CAN总线传输

//网络命令可能存在的状态, 控制相关。
#define NET_STATE_init 0
#define NET_STATE_delet 1
#define NET_STATE_done 2
#define NET_STATE_exe 3
#define NET_STATE_exe_data 4
#define NET_STATE_data 5
#define NET_STATE_result 6
#define NET_STATE_report 7

//帧头帧尾
#define NET_PACK_HEAD 0X5a1aa1a5
#define NET_PACK_END 0Xa5a11a5a

#pragma pack(1)
#define NET_DEFAULT_HEAD_LEN 15 //网络包数据域之前的所有字节数
typedef struct Net_pack_t
{
    uint32_t head;
    uint16_t len;
    uint16_t serial;
    uint8_t type; //
    uint8_t src_addr;
    uint8_t dest_addr;
    uint32_t cmd;
    uint8_t buf[];
} Net_pack_t;

typedef struct Net_pack_ack_t
{
    Net_pack_t pub;
    uint32_t err;
    uint8_t buf[];
} Net_pack_ack_t;

struct Net_pack_deal;
typedef struct Net_pack_deal
{
    Connect_t connect;
    uint8_t state;
    uint8_t last_state;
    uint32_t start_tick;
    uint32_t timeout_tick;
    //如果需要支持多连接等的时候，需要传递端口等额外的信息。
    uint8_t transfer_type; //传输类型, 如TCP\UDP等
    int sock_fd;           // tcp或UDP时使用的套接字
    uint32_t dest_ip;      // udp时ip,port才有用。
    uint16_t dest_port;

    //网络包处理相关参数
    uint8_t cnt;       //重发计数
    uint32_t err;      //错误码
    uint16_t data_len; //有效数据长度
    uint16_t pack_len; // pack指向的内存长度
    Net_pack_t *pack;
    uint32_t para_tem; //用于携带少量额外的数据, 各命令可以根据需要使用。
    void (*recv_state_deal)(struct Net_pack_deal *deal, struct Net_pack_deal *rec);
    void (*state_deal)(struct Net_pack_deal *deal, void *data);
    void (*state_change_cb)(struct Net_pack_deal *deal, void *data);
    //只有一份数据时使用此回调(无数据返回的命令此回调函数为空即可),
    // 对于上报帧，共用此处理函数,用于处理收到上报帧应答时的处理。
    uint32_t (*data_pack_cb)(struct Net_pack_deal *deal, void *data);
    //有多份数据时使用此回调(无数据返回的命令此回调函数为空即可)
    uint32_t (*data_more_cb)(struct Net_pack_deal *deal, void *data);
} Net_pack_deal_t;

typedef struct Net_pro_cmd_map
{
    uint32_t cmd;
    uint16_t (*net_recv_cmd_deal)(Net_pack_deal_t *deal, uint32_t *err);
    uint8_t can_addr;
} Net_pro_cmd_map_t;
#pragma pack()

extern struct rt_semaphore net_pack_change_sem;

void net_pro_get_frame_reset(void);
uint8_t net_pro_get_frame(uint8_t *buf, uint16_t *buflen, uint8_t *fram_buf, uint16_t *framlen);
uint16_t net_pro_recv(Net_pack_deal_t *deal, uint32_t *err);
void net_pro_deal_thread(void *p);

Net_pack_deal_t *net_pro_create(Net_pack_deal_t *deal);
void net_pro_delete(Net_pack_deal_t *deal);
uint8_t net_pro_add2line(Net_pack_deal_t *deal);
void net_pro_change_state(Net_pack_deal_t *deal, uint8_t new_state);
void net_pro_pub_err_para(Net_pack_deal_t *newdeal, uint32_t err);
uint8_t net_pro_creat_report(uint32_t cmd, void *buf, uint16_t buf_len, uint32_t data,  uint32_t (*data_pack_cb)(struct Net_pack_deal *deal, void *data));

#endif
