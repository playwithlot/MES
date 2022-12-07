/**
 * @file tcp_log.c
 * @author King (lzj..@com)
 * @brief 从rt_kprintf和LOG_X中截取所有的字符流，并通过tcp传到上位机。让上位机保存到文件中，方便查找问题用。
 * @version 0.1
 * @date 2021-09-08
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "tcp_log.h"
#include "ipc/ringbuffer.h"
#include "iot_parameter.h"
#include <sys/socket.h> /* 使用BSD socket，需要包含socket.h头文件 */
#include "app_init.h"

#define LOG_TAG "tcp.log"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

#define LOG_BUF_MAX 1200

//static uint8_t log_ringbuf[2048];
static struct rt_ringbuffer rb = {0};
static rt_sem_t log_sem = RT_NULL;
static int log_sock;
static struct rt_thread tcp_log_deal_hand;

int tcp_log_pre_init(void)
{
    static rt_bool_t is_init = RT_FALSE;
    if (is_init == RT_FALSE)
    {
        //rt_ringbuffer_init(&rb, &log_ringbuf[0], sizeof(log_ringbuf));
        rt_ringbuffer_init(&rb, (uint8_t *)LOG_BUF_ADDR, LOG_BUF_SIZE);
        is_init = RT_TRUE;
    }
    return 0;
}
#ifdef RT_USING_COMPONENTS_INIT
INIT_BOARD_EXPORT(tcp_log_pre_init);
#endif

//创建一个环形日志缓冲区、互斥量、信号量。此函数应该尽可能得先调用。
int tcp_log_init(void)
{
    tcp_log_pre_init();
    log_sem = rt_sem_create("log", 0, RT_IPC_FLAG_PRIO);
    if (log_sem == RT_NULL)
    {
        LOG_E("create tcp log sem failed");
    }
    return 0;
}
#ifdef RT_USING_COMPONENTS_INIT
INIT_APP_EXPORT(tcp_log_init);
#endif

//用于截获日志字符流的接口.
void tcp_log_print(const char *buf, uint32_t len)
{
    rt_base_t level;
    if (rb.buffer_ptr == RT_NULL)
    {
        return;
    }
    //考虑到本函数可能在中断函数中调用，所以不使用互斥量进行保护。
    level = rt_hw_interrupt_disable();
    rt_ringbuffer_put(&rb, (const rt_uint8_t *)buf, len);
    rt_hw_interrupt_enable(level);

    if (log_sem)
    {
        rt_sem_release(log_sem);
    }
}

//日志上传处理线程
void tcp_log_thread(void *p)
{
    int ret, reuse;
    uint16_t len = 0;
    struct sockaddr_in server_addr;
    //因为打算将此线程的栈放到CCM内存中，所以将栈开大些，此buf将从栈分配
    uint8_t log_buf[LOG_BUF_MAX];

    while (1)
    {
        while (1)
        {
            iot_para.status.is_tcp_log_connect = RT_FALSE;
            if ((log_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            {
                /* 创建socket失败 */
                LOG_E("create tcp log socket failed");
                rt_thread_delay(Msec(20));
                continue;
            }
            /* 初始化预连接的服务端地址 */
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(iot_para.tcp_log_port);
            server_addr.sin_addr = *((struct in_addr *)iot_para.tcp_log_server_addr);
            rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
            setsockopt(log_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
            /* 连接到服务端 */
            if (connect(log_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
            {
                closesocket(log_sock);
                rt_thread_delay(Msec(500));
                continue;
            }
            else
            {
                LOG_I("connet tcp log server succeed(%d.%d.%d.%d:%d)",
                      iot_para.tcp_log_server_addr[0],
                      iot_para.tcp_log_server_addr[1],
                      iot_para.tcp_log_server_addr[2],
                      iot_para.tcp_log_server_addr[3],
                      iot_para.tcp_log_port);
                iot_para.status.is_tcp_log_connect = RT_TRUE;
                break;
            }
        }
        rt_sem_release(log_sem);
        while (1)
        {
            //当TCP继开时，这里无法及时感知，且无法及时连接上。
            rt_sem_take(log_sem, RT_WAITING_FOREVER);
            if (len > 0) //之前从ringbuffer中读出的内容还没上传
            {
                ret = send(log_sock, log_buf, len, 0);
                if (ret <= 0)
                {
                    closesocket(log_sock);
                    rt_thread_delay(Msec(20));
                    LOG_E("close");
                    break;
                }
                else
                {
                    rt_thread_delay(Msec(2));
                    len = 0;
                }
            }
            len = rt_ringbuffer_get(&rb, log_buf, LOG_BUF_MAX);
            if (len > 0)
            {
                ret = send(log_sock, log_buf, len, 0);
                if (ret <= 0)
                {
                    closesocket(log_sock);
                    rt_thread_delay(Msec(20));
                    LOG_E("close");
                    break;
                }
                else
                {
                    len = 0;
                }
            }
            rt_thread_delay(Msec(3));
        }
    }
}



void create_tcp_log_thread(void)
{
	rt_err_t err;
	
	err = rt_thread_init(&tcp_log_deal_hand, "tcp_log", tcp_log_thread, RT_NULL,
											 (void *)TCP_LOG_THREAD_STACK_ADDR,
											 TCP_LOG_THREAD_STACK_SIZE,
											 TCP_LOG_THREAD_PRO, 20);
											
	if (err == RT_EOK)
	{
			rt_thread_startup(&tcp_log_deal_hand);
	}

}



