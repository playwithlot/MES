/**
 * @file app_init.c
 * @author lzj (lzj..@com)
 * @brief  线程建立相关处理文件。
 * @version 0.1
 * @date 2021-07-29
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "rtthread.h"
#include "monitor.h"
#include "tcp_receive.h"
#include "can_protocol.h"
#include "app_init.h"
#include "tcp_log.h"
#include "Udp_receive.h"
#include "scanner.h"
#include "net_protocol.h"
#include "pmt.h"
#include "exe_deal.h"
#include "rfid_reader.h"
#include "report.h"

extern void tdma_tcp_deal_thread(void *p);

static struct rt_thread monitor_hand;
static struct rt_thread tcp_recv_hand;
static struct rt_thread can_deal_hand;
struct rt_thread tdma_tcp_deal_hand;
static struct rt_thread can_recv_deal_hand;
static struct rt_thread tcp_log_deal_hand;
static struct rt_thread udp_receive_hand;
static struct rt_thread scanner_deal_hand;
static struct rt_thread net_pack_deal_hand;
static struct rt_thread pmt_deal_hand;
static struct rt_thread exe_deal_hand;
static struct rt_thread rfid_deal_hand;
static struct rt_thread rfid_recv_hand;
static struct rt_thread report_deal_hand;

struct rt_thread telnet_thread_hand;

/**
 * @brief 用户应用线程初始化。
 *
 */
void app_init(void)
{
    rt_err_t err;

//    // 监视及LED显示线程
//    err = rt_thread_init(&monitor_hand, "monitor", monitor_thread, RT_NULL,
//                         (void *)MONITOR_THREAD_STACK_ADDR,
//                         MONITOR_THREAD_STACK_SIZE,
//                         MONITOR_THREAD_PRO, 20);
//    if (err == RT_EOK)
//    {
//        rt_thread_startup(&monitor_hand);
//    }

//    // pmt
//    err = rt_thread_init(&pmt_deal_hand, "pmt", pmt_deal_thread, RT_NULL,
//                         (void *)PMT_DEAL_THREAD_STACK_ADDR,
//                         PMT_DEAL_THREAD_STACK_SIZE,
//                         PMT_DEAL_THREAD_PRO, 20);
//    if (err == RT_EOK)
//    {
//        rt_thread_startup(&pmt_deal_hand);
//    }
//		
//    // 命令对接处理线程
//    err = rt_thread_init(&exe_deal_hand, "exe", exe_deal_thread, RT_NULL,
//                         (void *)EXE_DEAL_THREAD_STACK_ADDR,
//                         EXE_DEAL_THREAD_STACK_SIZE,
//                         EXE_DEAL_THREAD_pro, 20);
//    if (err == RT_EOK)
//    {
//        rt_thread_startup(&exe_deal_hand);
//    }

//    // tcp上传日志线程
//    err = rt_thread_init(&tcp_log_deal_hand, "tcp_log", tcp_log_thread, RT_NULL,
//                         (void *)TCP_LOG_THREAD_STACK_ADDR,
//                         TCP_LOG_THREAD_STACK_SIZE,
//                         TCP_LOG_THREAD_PRO, 20);
//    if (err == RT_EOK)
//    {
//        rt_thread_startup(&tcp_log_deal_hand);
//    }

//    // tcp接收线程（接收命令）
//    err = rt_thread_init(&tcp_recv_hand, "tcp_rec", tcpserv_receive_thread, RT_NULL,
//                         (void *)TCP_RECV_THREAD_STACK_ADDR,
//                         TCP_RECV_THREAD_STACK_SIZE,
//                         TCP_RECV_THREAD_PRO, 20);
//    if (err == RT_EOK)
//    {
//        rt_thread_startup(&tcp_recv_hand);
//    }
//		
//		
//    // report 
//    err = rt_thread_init(&report_deal_hand, "report", report_deal_thread, RT_NULL,
//                         (void *)REPORT_DEAL_THREAD_STACK_ADDR,
//                         REPORT_DEAL_THREAD_STACK_SIZE,
//                         REPORT_THREAD_PRO, 20);
//    if (err == RT_EOK)
//    {
//        rt_thread_startup(&report_deal_hand);
//    }

//    // udp接收线程（接收命令等）
//    err = rt_thread_init(&udp_receive_hand, "udp_rec", udp_recv_thread, RT_NULL,
//                         (void *)UDP_RECV_THREAD_STACK_ADDR,
//                         UDP_RECV_THREAD_STACK_SIZE,
//                         UDP_RECV_THREAD_PRO, 20);
//    if (err == RT_EOK)
//    {
//        rt_thread_startup(&udp_receive_hand);
//    }
//    //网络包处理线程
//    err = rt_thread_init(&net_pack_deal_hand, "net_pro", net_pro_deal_thread, RT_NULL,
//                         (void *)NET_PACK_DEAL_THREAD_STACK_ADDR,
//                         NET_PACK_DEAL_THREAD_STACK_SIZE,
//                         NET_PACK_DEAL_THREAD_PRO, 20);
//    if (err == RT_EOK)
//    {
//        rt_thread_startup(&net_pack_deal_hand);
//    }

//    //扫描枪处理线程
//    err = rt_thread_init(&scanner_deal_hand, "scanner", scanner_deal_thread, RT_NULL,
//                         (void *)SCANNER_DEAL_THREAD_STACK_ADDR,
//                         SCANNER_DEAL_THREAD_STACK_SIZE,
//                         SCANNER_DEAL_THREAD_PRO, 20);
//    if (err == RT_EOK)
//    {
//        rt_thread_startup(&scanner_deal_hand);
//    }

//    //主要的命令处理线程（网络命令和CAN命令的对接处理）
//    err = rt_thread_init(&can_deal_hand, "can_pro", can_pro_deal_thread, RT_NULL,
//                         (void *)CAN_DEAL_THREAD_STACK_ADDR,
//                         CAN_DEAL_THREAD_STACK_SIZE,
//                         CAN_DEAL_THREAD_pro, 20);
//    if (err == RT_EOK)
//    {
//        rt_thread_startup(&can_deal_hand);
//    }

//    // CAN接收线程
//    err = rt_thread_init(&can_recv_deal_hand, "can_recv", Can_recv_thread, RT_NULL,
//                         (void *)CAN_RECV_DEAL_THREAD_STACK_ADDR,
//                         CAN_RECV_DEAL_THREAD_STACK_SIZE,
//                         CAN_RECV_DEAL_THREAD_PRO, 20);
//    if (err == RT_EOK)
//    {
//        rt_thread_startup(&can_recv_deal_hand);
//    }

//    err = rt_thread_init(&rfid_recv_hand, "rfid_recv", rfid_recv_thread, RT_NULL,
//                         (void *)RFID_RECV_THREAD_STACK_ADDR,
//                         RFID_RECV_THREAD_STACK_SIZE,
//                         RFID_RECV_THREAD_PRO, 20);
//    if (err == RT_EOK)
//    {
//        rt_thread_startup(&rfid_recv_hand);
//    }

//    err = rt_thread_init(&rfid_deal_hand, "rfid_deal", rfid_deal_thread, RT_NULL,
//                         (void *)RFID_DEAL_THREAD_STACK_ADDR,
//                         RFID_DEAL_THREAD_STACK_SIZE,
//                         RFID_DEAL_THREAD_PRO, 20);
//    if (err == RT_EOK)
//    {
//        rt_thread_startup(&rfid_deal_hand);
//    }
}
