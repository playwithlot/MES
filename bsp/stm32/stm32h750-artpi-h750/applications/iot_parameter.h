#ifndef __IOT_PARAMETER_H__
#define __IOT_PARAMETER_H__

#include "rtthread.h"

#define SPI_FLASH_NAME "w25q64"

#pragma pack(1)
typedef struct iot_parameter
{
    //基本参数
    rt_uint8_t net_addr; //网络协议中用到的地址
    rt_uint8_t uid[12];
    char name[20];
    rt_uint8_t hardware_version[4];
    rt_uint8_t software_version[4];
    //本机的网络参数
    rt_uint8_t ip_addr[4];
    rt_uint8_t gw_addr[4];
    rt_uint8_t netmask[4];
    rt_uint8_t mac[6];
    //TCP 相关
    rt_uint16_t local_tcp_server_port; //本机的服务器端口
    rt_uint16_t tcp_log_port;//远程log服务器端口
    rt_uint8_t tcp_log_server_addr[4];//远程log服务器地址
    //Udp
    rt_uint16_t udp_recv_port;//单播端口
    rt_uint16_t udp_recv_bc_port;//广播端口
    rt_uint16_t udp_recv_mc_port;//组播端口

    //其它参数
    rt_uint8_t can_addr; //在can总线通信中使用的地址
    uint16_t crc; //此字段前面的内容均会掉电保存

    //基本状态
    struct Iot_parameter_status
    {
        rt_bool_t is_need_reboot; //TRUE的时候，系统应该进行重启。
        rt_bool_t is_tcp_connect;
        rt_bool_t is_tcp_log_connect;
        uint8_t can_bus_send_failed_cnt;
        uint8_t remote_net_addr;//网络协议中的上位机地址
    } status;

} Iot_parameter_t;
#pragma pack()

void iot_para_init(void);
void iot_para_set(Iot_parameter_t *para);
void iot_para_save(Iot_parameter_t *para);
void iot_para_read(Iot_parameter_t *para);
uint8_t iot_para_is_legal(Iot_parameter_t *para);

rt_int32_t str2int(char *c);

extern Iot_parameter_t iot_para;

#endif
