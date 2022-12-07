/**
 * @file iot_parameter.c
 * @author lzj (lzj..@com)
 * @brief  此文件用来存放板子常规参数配置与处理相关函数.
 * @version 0.1
 * @date 2021-07-28
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "iot_parameter.h"
#include "stm32h750xx.h"
#include "iot_version.h"
#include "string.h"
#include "dev_flash.h"
#include "CRC16.h"

#define LOG_TAG "iot"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

void reset_mac(void);

// TCP相关端口
#define TCP_LOCAL_SERVER_PORT 8500      //本地tcp服务器端口
#define TCP_REMOTE_LOG_SERVER_PORT 8600 //远程TCP 日志服务器端口

// UDP接收端口不应该重复
#define UDP_RECV_PORT 8700   //单播端口
#define UDPBC_RECV_PORT 8800 //广播端口
#define UDPMC_RECV_PORT 8900 //组播端口

#define NET_DEFAULT_ADDR 254//网络协议中用到的数据

Iot_parameter_t iot_para = {0};

static const char iot_default_name[30] = PRJ_NMAE;
static uint32_t save_len = sizeof(Iot_parameter_t) - sizeof(struct Iot_parameter_status);

//基本参数恢复默认值
static void resetDef(Iot_parameter_t *para)
{
    uint16_t crc;
    LOG_I("iot para reset defult");
    rt_memcpy(&para->name, &iot_default_name, sizeof(para->name));
    para->hardware_version[0] = HARD_VERSION_A;
    para->hardware_version[1] = HARD_VERSION_B;
    para->hardware_version[2] = HARD_VERSION_C;
    para->hardware_version[3] = HARD_VERSION_D;

    para->software_version[0] = SOFT_VERSION_A;
    para->software_version[1] = SOFT_VERSION_B;
    para->software_version[2] = SOFT_VERSION_C;
    para->software_version[3] = SOFT_VERSION_D;

    para->gw_addr[0] = 192;
    para->gw_addr[1] = 168;
    para->gw_addr[2] = 0;
    para->gw_addr[3] = 1;

    para->netmask[0] = 255;
    para->netmask[1] = 255;
    para->netmask[2] = 255;
    para->netmask[3] = 0;

    para->ip_addr[0] = 192;
    para->ip_addr[1] = 168;
    para->ip_addr[2] = 0;
    para->ip_addr[3] = 100;

    reset_mac();

    para->tcp_log_server_addr[0] = 192;
    para->tcp_log_server_addr[1] = 168;
    para->tcp_log_server_addr[2] = 0;
    para->tcp_log_server_addr[3] = 103;

    para->tcp_log_port = TCP_REMOTE_LOG_SERVER_PORT;
    para->local_tcp_server_port = TCP_LOCAL_SERVER_PORT;

    para->udp_recv_port = UDP_RECV_PORT;
    para->udp_recv_bc_port = UDPBC_RECV_PORT;
    para->udp_recv_mc_port = UDPMC_RECV_PORT;

    para->net_addr = NET_DEFAULT_ADDR;
    para->can_addr = 9;

    crc = CRC16_modbus((uint8_t *)para, save_len - 2);
    para->crc = crc;
    iot_para_save(para);
}
MSH_CMD_EXPORT(resetDef, "reset iot para to default");

// iot基本参数合法性检测, 返回1则合法
uint8_t iot_para_is_legal(Iot_parameter_t *para)
{
    uint8_t ret = RT_TRUE;

    //端口检测, 强制本机相关的端口互相不能相同.
    if (para->udp_recv_port == para->udp_recv_bc_port ||
        para->udp_recv_port == para->udp_recv_mc_port ||
        para->udp_recv_port == para->local_tcp_server_port ||
        para->udp_recv_bc_port == para->udp_recv_mc_port ||
        para->udp_recv_bc_port == para->local_tcp_server_port ||
        para->udp_recv_mc_port == para->local_tcp_server_port)
    {
        return RT_FALSE;
    }
    //本机端口不能使用系统保留的
    if(para->local_tcp_server_port < 1024)
    {
        return RT_FALSE;
    }
    //不能使用回环地址
    if (para->ip_addr[0] == 127 &&
        para->ip_addr[1] == 0 &&
        para->ip_addr[2] == 0 &&
        para->ip_addr[3] == 1)
    {
        return RT_FALSE;
    }
    if (para->tcp_log_server_addr[0] == 127 &&
        para->tcp_log_server_addr[1] == 0 &&
        para->tcp_log_server_addr[2] == 0 &&
        para->tcp_log_server_addr[3] == 1)
    {
        return RT_FALSE;
    }
    if (para->gw_addr[0] == 127 &&
        para->gw_addr[1] == 0 &&
        para->gw_addr[2] == 0 &&
        para->gw_addr[3] == 1)
    {
        return RT_FALSE;
    }
    //不能使用广播地址
    if (para->ip_addr[0] == 255 &&
        para->ip_addr[1] == 255 &&
        para->ip_addr[2] == 255 &&
        para->ip_addr[3] == 255)
    {
        return RT_FALSE;
    }
    if (para->tcp_log_server_addr[0] == 255 &&
        para->tcp_log_server_addr[1] == 255 &&
        para->tcp_log_server_addr[2] == 255 &&
        para->tcp_log_server_addr[3] == 255)
    {
        return RT_FALSE;
    }
    if (para->gw_addr[0] == 255 &&
        para->gw_addr[1] == 255 &&
        para->gw_addr[2] == 255 &&
        para->gw_addr[3] == 255)
    {
        return RT_FALSE;
    }
    if (para->ip_addr[3] == 255 || para->tcp_log_server_addr[3] == 255)
    {
        return RT_FALSE;
    }

    return ret;
}

/**
 * @brief 基本参数初始化
 *
 */
void iot_para_init(void)
{
    uint16_t crc;
    uint8_t flag;

    save_len = (uint32_t)&iot_para.crc - (uint32_t)&iot_para + sizeof(iot_para.crc);
    iot_para_read(&iot_para);
    flag = iot_para_is_legal(&iot_para);
    crc = CRC16_modbus((uint8_t *)&iot_para, save_len - 2);
    if (iot_para.crc != crc || flag == RT_FALSE)
    {
        LOG_E("iot para err");
        resetDef(&iot_para);
    }
    iot_para.status.remote_net_addr = 0;
}

void iot_para_set(Iot_parameter_t *para)
{
    iot_para = *para;
}

/**
 * @brief 保存参数到flash(等ROM介质)
 *
 * @param para
 */
void iot_para_save(Iot_parameter_t *para)
{
    uint16_t crc;
    //版本信息
    para->hardware_version[0] = HARD_VERSION_A;
    para->hardware_version[1] = HARD_VERSION_B;
    para->hardware_version[2] = HARD_VERSION_C;
    para->hardware_version[3] = HARD_VERSION_D;

    para->software_version[0] = SOFT_VERSION_A;
    para->software_version[1] = SOFT_VERSION_B;
    para->software_version[2] = SOFT_VERSION_C;
    para->software_version[3] = SOFT_VERSION_D;
    rt_memcpy(&para->uid[0], (uint8_t *)UID_BASE, 12);
    crc = CRC16_modbus((uint8_t *)&iot_para, save_len - 2);
    para->crc = crc;
    //保存基本参数
    rt_memcpy(&para->uid[0], (uint8_t *)UID_BASE, 12);
    dev_flash_write(SPI_FLASH_IOT_PARA_ADDR, para, save_len);
}

/**
 * @brief  从flash中读取
 *
 * @param para
 */
void iot_para_read(Iot_parameter_t *para)
{
    //读取基本参数
    dev_flash_read(SPI_FLASH_IOT_PARA_ADDR, para, save_len);
    rt_memcpy(&para->uid[0], (uint8_t *)UID_BASE, 12);
    //版本信息
    para->hardware_version[0] = HARD_VERSION_A;
    para->hardware_version[1] = HARD_VERSION_B;
    para->hardware_version[2] = HARD_VERSION_C;
    para->hardware_version[3] = HARD_VERSION_D;

    para->software_version[0] = SOFT_VERSION_A;
    para->software_version[1] = SOFT_VERSION_B;
    para->software_version[2] = SOFT_VERSION_C;
    para->software_version[3] = SOFT_VERSION_D;
}

//字符转数字
rt_int32_t str2int(char *c)
{
    uint8_t i = 0;
    rt_int32_t ret = 0;
    uint8_t flag = 0;

    if (c[0] != '\0' && c[1] != '\0')
    {
        if (c[0] == '0' && (c[1] == 'x' || c[1] == 'X'))
        {
            flag = 1;
        }
    }
    if (flag == 1)
    {
        i = 2;
    }
    for (;; i++)
    {
        if (c[i] == '\0')
        {
            break;
        }
        if (flag == 1)
        {
            ret = ret * 16 + c[i] - '0';
        }
        else
        {
            ret = ret * 10 + c[i] - '0';
        }
    }

    return ret;
}

void reset_mac(void)
{
    uint8_t iLoop;
    uint8_t *pid8;
    uint32_t val = 0;
    //清mac地址
    rt_memset(iot_para.mac, 0, sizeof(iot_para.mac));
    //赋值
    iot_para.mac[0] = 0;
    iot_para.mac[1] = 91;

    //使用BKD_Hash可以减少冲突率
    //其中，mac地址只使用1次芯片UID数据，长度为12字节
    pid8 = (uint8_t *)UID_BASE;
    val = 0;
    for (iLoop = 0; iLoop < 12; iLoop++)
    {
        val = val * 131 + *pid8; // 也可以乘以31、131、1313、13131、131313
        pid8++;
    }

    rt_memcpy(&iot_para.mac[2], &val, sizeof(val));
}

#if 1 //设置ip等。
void iot(int argc, char **argv)
{

    if (argc < 3)
    {
        goto __p_err;
    }
    if (!rt_strcmp(argv[1], "ip") && argc >= 5)
    {
        iot_para.ip_addr[0] = str2int(argv[2]);
        iot_para.ip_addr[1] = str2int(argv[3]);
        iot_para.ip_addr[2] = str2int(argv[4]);
        iot_para.ip_addr[3] = str2int(argv[5]);
        iot_para_save(&iot_para);
        LOG_D("set ip:%d.%d.%d.%d", iot_para.ip_addr[0], iot_para.ip_addr[1], iot_para.ip_addr[2], iot_para.ip_addr[3]);
    }
    else if (!rt_strcmp(argv[1], "gw") && argc >= 5)
    {
        iot_para.gw_addr[0] = str2int(argv[2]);
        iot_para.gw_addr[1] = str2int(argv[3]);
        iot_para.gw_addr[2] = str2int(argv[4]);
        iot_para.gw_addr[3] = str2int(argv[5]);
        iot_para_save(&iot_para);
        LOG_D("set gw:%d.%d.%d.%d", iot_para.gw_addr[0], iot_para.gw_addr[1], iot_para.gw_addr[2], iot_para.gw_addr[3]);
    }
    else if (!rt_strcmp(argv[1], "mac") && argc >= 7)
    {
        iot_para.mac[0] = str2int(argv[2]);
        iot_para.mac[1] = str2int(argv[3]);
        iot_para.mac[2] = str2int(argv[4]);
        iot_para.mac[3] = str2int(argv[5]);
        iot_para.mac[4] = str2int(argv[6]);
        iot_para.mac[5] = str2int(argv[7]);
        iot_para_save(&iot_para);
        LOG_D("set mac:%d %d %d %d %d %d",
              iot_para.mac[0],
              iot_para.mac[1],
              iot_para.mac[2],
              iot_para.mac[3],
              iot_para.mac[4],
              iot_para.mac[5]);
    }
    else if (!rt_strcmp(argv[1], "mask") && argc >= 5)
    {
        iot_para.netmask[0] = str2int(argv[2]);
        iot_para.netmask[1] = str2int(argv[3]);
        iot_para.netmask[2] = str2int(argv[4]);
        iot_para.netmask[3] = str2int(argv[5]);
        iot_para_save(&iot_para);
        LOG_D("set mask:%d.%d.%d.%d", iot_para.netmask[0], iot_para.netmask[1], iot_para.netmask[2], iot_para.netmask[3]);
    }
    else if (!rt_strcmp(argv[1], "sport") && argc >= 3)
    {
        iot_para.local_tcp_server_port = str2int(argv[2]);
        LOG_D("set local server port: %d", iot_para.local_tcp_server_port);
    }
    else if (!rt_strcmp(argv[1], "lsport") && argc >= 3)
    {
        iot_para.tcp_log_port = str2int(argv[2]);
        LOG_D("set remote log server port: %d", iot_para.tcp_log_port);
    }
    else if (!rt_strcmp(argv[1], "lsip") && argc >= 5)
    {
        iot_para.tcp_log_server_addr[0] = str2int(argv[2]);
        iot_para.tcp_log_server_addr[1] = str2int(argv[3]);
        iot_para.tcp_log_server_addr[2] = str2int(argv[4]);
        iot_para.tcp_log_server_addr[3] = str2int(argv[5]);
        iot_para_save(&iot_para);
        LOG_D("set remote log server ip:%d.%d.%d.%d",
              iot_para.tcp_log_server_addr[0],
              iot_para.tcp_log_server_addr[1],
              iot_para.tcp_log_server_addr[2],
              iot_para.tcp_log_server_addr[3]);
    }
    else
    {
    __p_err:
        rt_kprintf("use for example...........\r\n");
        rt_kprintf("iot ip 192 168 1 2\r\n");
        rt_kprintf("iot gw 192 168 1 1\r\n");
        rt_kprintf("iot mac 0xAA 0xBB 0xCC 0xDD\r\n");
        rt_kprintf("iot mask 255 255 255 0\r\n");
        rt_kprintf("iot sport 8500   (set local tcp server port)\r\n");
        rt_kprintf("iot lsport 8600   (set remote log tcp server port)\r\n");
        rt_kprintf("iot lsip 192 168 1 123    (set remot log tcp server ip)\r\n");
    }
}
MSH_CMD_EXPORT(iot, "set iot ip. you should reboot after set");
#endif

#if 0 // 测试用
#include "finsh.h"
//打印板子的基本信息
void show(void)
{
    show_compile_time();
    LOG_I("name:%s", iot_para.name);
    LOG_I("local ip: %d.%d.%d.%d",
          iot_para.ip_addr[0],
          iot_para.ip_addr[1],
          iot_para.ip_addr[2],
          iot_para.ip_addr[3]);
    LOG_I("local tcp port(server, telnet): %d, %d", iot_para.local_tcp_server_port, 23);
    LOG_I("remote tcp log server: %d.%d.%d.%d:%d",
          iot_para.tcp_log_server_addr[0],
          iot_para.tcp_log_server_addr[1],
          iot_para.tcp_log_server_addr[2],
          iot_para.tcp_log_server_addr[3],
          iot_para.tcp_log_port);
}
MSH_CMD_EXPORT(show, "print iot base message");

#endif

#if 0 // 调试用
const static char map_table[] = "0123456789";
void data2Ascii(rt_uint32_t h, rt_int8_t b, rt_uint8_t *buf)
{
    rt_int8_t i;

    if(b<=0)
    {
        return;
    }
    i = b -1;
    for (; i >=0 ; i--)
    {
        buf[i] = map_table[h%10]; 
        h = h/10;
    }
}

#include "finsh.h"

extern time_t get_rtc_timestamp(void);
extern rt_err_t set_rtc_time_stamp(time_t time_stamp);
void tt(void)
{
    rt_uint8_t buf[15]={0};

    struct tm *p_tm;
    time_t timestmap = get_rtc_timestamp();

    p_tm = localtime(&timestmap);
    /*
    RTC_TimeStruct.Seconds = p_tm->tm_sec ;
    RTC_TimeStruct.Minutes = p_tm->tm_min ;
    RTC_TimeStruct.Hours   = p_tm->tm_hour;
    RTC_DateStruct.Date    = p_tm->tm_mday;
    RTC_DateStruct.Month   = p_tm->tm_mon + 1 ;
    RTC_DateStruct.Year    = p_tm->tm_year - 100;
    RTC_DateStruct.WeekDay = p_tm->tm_wday + 1;
    */
    p_tm->tm_year += 1900;
    rt_kprintf("year %d\r\n", p_tm->tm_year);
    data2Ascii(p_tm->tm_year, 4, buf);
    data2Ascii(p_tm->tm_year, 4, buf);
    buf[4] = '\n';
    //data2Ascii(p_tm->tm_mon, 2, buf+4);
    //data2Ascii(p_tm->tm_mday, 2, buf+2);
    //data2Ascii(p_tm->tm_hour, 2, buf+2);
    //data2Ascii(p_tm->tm_min, 2, buf+2);
    //data2Ascii(p_tm->tm_sec, 2, buf+2);

    //buf[14] = '\n';
    rt_kprintf("%s\r\n",buf);
}
MSH_CMD_EXPORT(tt, "test");

#endif
