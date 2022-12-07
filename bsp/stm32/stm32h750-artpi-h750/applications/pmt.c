/**
 * @file pmt.c
 * @author King (lzj..@com)
 * @brief  pmt处理文件。
 * @version 0.1
 * @date 2021-10-25
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "rtthread.h"
#include "rtdevice.h"
#include "drv_gpio.h"
#include "pmt.h"

#include "dev_flash.h"
#include "CRC16.h"
#include "stdio.h"
#define LOG_TAG "pmt"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

#define PMT_DATA_FIFO_ITEM_NUM 1000

#define RX_BUF_SIZE 500 //处理时使用的内存，并非驱动接收缓存
#define PMT_DEV_NAME "uart8"
#define PMT_DEV_BAUD BAUD_RATE_19200


static rt_device_t pmt_dev = RT_NULL; //串口驱动
static struct rt_semaphore rx_sem;    /* 用于接收消息的信号量 */
static rt_mailbox_t pmt_ctr_mb;
static struct rt_timer pmt_timer;

static Pmt_t pmt_info = {0};
static rt_mutex_t data_fifo_mutex;
//static uint8_t data_buf[PMT_DATA_FIFO_ITEM_NUM * sizeof(Pmt_data_t)] = {0}; //用来存放pmt数据
static uint8_t rx_buf[RX_BUF_SIZE + 4] = {0};


//在等待时间内接收应答, 使用统一的缓存
static uint16_t pmt_wait_ack(uint32_t timeout)
{
    uint8_t len;
    uint8_t flag = 0;
    uint8_t buf[10];
    uint16_t rlen = 0;

    while (1)
    {
        len = 0;
        if (rt_sem_take(&rx_sem, timeout) == RT_EOK)
        {
            len = rt_device_read(pmt_dev, -1, buf, 10);
        }
    
    }
    return rlen;
}



//将串口中的内容全部读出来丢弃
static void clear_rx(void)
{
    uint8_t buf[10];
    uint8_t len;

    do
    {
        len = rt_device_read(pmt_dev, -1, &buf, 10);
    } while (len);
    rt_sem_control(&rx_sem, RT_IPC_CMD_RESET, (void *)0);
    //fifo_clear(&rx_fifo);
}

static uint8_t pmt_send_cmd(uint8_t *buf, uint8_t buf_len)
{
    uint8_t send_len;

    LOG_I("pmt send, cmd:%c", buf[0]);
    LOG_HEX("pmt send", 20, (rt_uint8_t *)buf, buf_len);
    send_len = rt_device_write(pmt_dev, -1, buf, buf_len);
    if (send_len != buf_len)
    {
        LOG_E("send cmd err");
    }

    return send_len;
}


//将Pmt字符型的版本号转换为4字节数值。
void pmt_version_format(uint32_t *v)
{
    uint8_t *res = (uint8_t *)v;
    char buf[10];
    uint8_t i;
    uint8_t len = 0, offset = 0;

    *v = 0;
    for (i = 0; i < 4; i++)
    {
        rt_memset(buf, 0, sizeof(buf));
        sscanf(&pmt_info.version[1 + offset], "%[^.]", buf);
        len = rt_strlen(buf);
        if (len > 0)
        {
            offset += len + 1;
            res[i] = atoi(buf);
        }
        else
        {
            break;
        }
    }
}


/**
 * @brief 发送命令到PMT获取版本.
 *
 * @return RT_TRUE:执行成功
 */
static uint8_t __pmt_get_version(void *cmd)
{
    uint16_t len = 3;
    uint8_t buf[3];
    uint8_t ret = RT_FALSE;
    uint8_t i;

    buf[0] = '?'; //查询版本
    buf[1] = '\r';
    buf[2] = '\n';

    clear_rx();
    rt_memset(rx_buf, 0, sizeof(pmt_info.version) + 2);
    if (pmt_send_cmd(buf, len) == len)
    {
        len = pmt_wait_ack(Msec(300)); //形似"V6.1.20"
        LOG_I("recv len:%d", len);
        LOG_HEX("data", 10, rx_buf, len);
        if ((len > 6) && (len <= sizeof(pmt_info.version)))
        {
            for (i = 0; i < len; i++)
            {
                if (rx_buf[i] == 'V' && (len - i) >= 6)
                {
                    rt_memset(&pmt_info.version, 0, sizeof(pmt_info.version));
                    rt_memcpy(&pmt_info.version, &rx_buf[i], sizeof(pmt_info.version) - 1);
                    ret = RT_TRUE;
                    break;
                }
            }
        }
    }
    return ret;
}



static void pmt_param_reset_def(void)
{
    rt_memset(&pmt_info.version, 0, sizeof(pmt_info.version));
    pmt_info.resolution = 17;
    pmt_info.gating_time = 1; // 100;
}

static void pmt_param_save(void)
{
    uint16_t crc;
    uint16_t len = ((uint32_t)&pmt_info.crc - (uint32_t)&pmt_info);

    crc = CRC16_modbus((uint8_t *)&pmt_info, len);
    pmt_info.crc = crc;
    len += sizeof(pmt_info.crc);
    dev_flash_write(SPI_FLASH_PMT_PARA_ADDR, (uint8_t *)&pmt_info, len);
}

static void pmt_param_read(void)
{
    uint16_t crc;
    uint16_t len = ((uint32_t)&pmt_info.crc - (uint32_t)&pmt_info);

    dev_flash_read(SPI_FLASH_PMT_PARA_ADDR, (uint8_t *)&pmt_info, len + sizeof(pmt_info.crc));
    crc = CRC16_modbus((uint8_t *)&pmt_info, len);
    if (crc != pmt_info.crc)
    {
        LOG_E("pmt reset default");
        pmt_param_reset_def();
        pmt_param_save();
    }
}




//设置脉冲分辨时间
//resolution: 脉冲分辨率
static uint8_t __pmt_set_resolution(void *ctr, uint16_t resolution)
{
    uint8_t value;
    uint16_t len = 3;
    uint8_t buf[20] = {0};
    uint8_t i;
    uint8_t ret = RT_FALSE;

    buf[0] = 'C';
    value = resolution; //value = ctr->para;
    rt_sprintf((char *)&buf[1], "%d", value);
    for (i = 0; i < 20; i++)
    {
        if (buf[i] == '\0')
        {
            buf[i] = '\r';
            buf[i + 1] = '\n';
            len = i + 2;
            break;
        }
    }
    clear_rx();
    if (pmt_send_cmd(buf, len) == len)
    {
        len = pmt_wait_ack(Msec(300));
        LOG_I("recv len:%d", len);
        LOG_HEX("data", 10, rx_buf, len);
        if (len == 2 && rx_buf[0] == 0x43)
        {
            if (rx_buf[1] == value)
            {
                if (pmt_info.resolution != value)
                {
                    pmt_info.resolution = value;
                    pmt_param_save();
                }
                ret = RT_TRUE;
            }
            else
            {
                LOG_E("set resolution err, real:%d, purpose:%d", rx_buf[1], value);
            }
        }
    }
    return ret;
}


//设置门控时间
static uint8_t __pmt_set_gating_time(void *ctr, uint32_t gating_time)
{
    uint16_t value;
    uint16_t len = 3;
    uint8_t buf[20] = {0};
    uint8_t i;
    uint8_t ret = RT_FALSE;
    uint16_t temp;

    value = gating_time; //value = ctr->para;
    buf[0] = 'T';
    rt_sprintf((char *)&buf[1], "%d", value);
    for (i = 0; i < 20; i++)
    {
        if (buf[i] == '\0')
        {
            buf[i] = '\r'; //增加尾部
            buf[i + 1] = '\n';
            len = i + 2;
            break;
        }
    }
    clear_rx();
    if (pmt_send_cmd(buf, len) == len)
    {
        len = pmt_wait_ack(Msec(300));
        LOG_I("recv len:%d", len);
        LOG_HEX("data", 10, rx_buf, len);
        if (len == 3 && rx_buf[0] == 0x54)
        {
            temp = (rx_buf[1] << 8) | rx_buf[2];
            if (temp == value)
            {
                if (pmt_info.gating_time != value)
                {
                    pmt_info.gating_time = value;
                    pmt_param_save();
                }
                ret = RT_TRUE;
            }
            else
            {
                LOG_E("set gating time err, real:%d, purpose:%d", temp, value);
            }
        }
    }
    return ret;
}


//测试通信
static uint8_t __pmt_test_comm(void *ctr)
{
    uint8_t len = 3;
    uint8_t buf[4];
    uint8_t ret = RT_FALSE;

    buf[0] = 'Q';
    buf[1] = '\r';
    buf[2] = '\n';

    clear_rx();
    rt_memset(rx_buf, 0, 2);
    if (pmt_send_cmd(&buf[0], len) == len)
    {
        len = pmt_wait_ack(Msec(100));
        if (len == 1 && rx_buf[0] == '!')
        {
            ret = RT_TRUE;
        }
    }
    return ret;
}


//启动计数，连续计数模式
static uint8_t __pmt_start_continue_mode(void *ctr, uint16_t time_out)
{
    uint16_t len = 3;
    uint8_t buf[10];
    //Pmt_data_t data;
    uint8_t ret = RT_FALSE;
    uint8_t cnt = 0;
    uint8_t vef = 0;
    uint16_t num = 0;
    uint8_t step = 0;
    uint8_t i;
    uint32_t start_tick;
    uint32_t timeout;
    uint32_t timeout_tick;
    uint32_t tick = 0;

    buf[0] = 'S';
    buf[1] = '\r';
    buf[2] = '\n';

    if (pmt_info.gating_time <= 1)
    {
        timeout = Msec(20);
    }
    else
    {
        timeout = Msec(pmt_info.gating_time * 10) + Msec(2);
    }
		
		
    if (time_out == 0 || time_out > PMT_DATA_FIFO_ITEM_NUM) //次数
    {
        time_out = 1;
    }
		
		
    timeout_tick = time_out * Msec(pmt_info.gating_time * 10) + Msec(10);
		
    rt_mutex_take(data_fifo_mutex, RT_WAITING_FOREVER);
    rt_mutex_release(data_fifo_mutex);
    clear_rx();
    start_tick = rt_tick_get();
    if (pmt_send_cmd(buf, len) == len)
    {
        while (1)
        {
            if (timeout_tick < rt_tick_get() - start_tick)
            {
                ret = RT_TRUE;
                break;
            }
            len = 0;
            if (rt_sem_take(&rx_sem, timeout) == RT_EOK)
            {
                len = rt_device_read(pmt_dev, -1, buf, 10);
            }
 
        }

    }
    return ret;
}

//停止计数并清零
static uint8_t __pmt_stop_and_clean(void *ctr)
{
    uint16_t len = 3;
    uint8_t buf[3];
    uint8_t ret = RT_FALSE;

    buf[0] = 'E'; //停止命令
    buf[1] = '\r';
    buf[2] = '\n';

    clear_rx();
    if (pmt_send_cmd(buf, len) == len)
    {
        len = pmt_wait_ack(Msec(210));
        LOG_I("recv len:%d", len);
        LOG_HEX("data", 10, rx_buf, len);
        if (len >= 1 && rx_buf[0] == 'E')
        {
            ret = RT_TRUE;
        }
    }
    return ret;
}


//启动一次计数
static uint8_t __pmt_start_once(void *ctr)
{
    uint16_t len = 3;
    uint8_t buf[3];
    uint8_t ret = RT_FALSE;

    buf[0] = 'J';
    buf[1] = '\r';
    buf[2] = '\n';

    clear_rx();
    if (pmt_send_cmd(buf, len) == len)
    {
        ret = RT_TRUE;
        len = pmt_wait_ack(Msec(100));
        LOG_I("recv len:%d", len);
        LOG_HEX("data", 10, rx_buf, len);
        uint32_t data;
        if (len == 6)
        {
            data = (rx_buf[1] << 24) + (rx_buf[2] << 16) + (rx_buf[3] << 8) + rx_buf[4];
            LOG_I("pmt data:%d", data);
            ret = RT_TRUE;
        }
    }
    return ret;
}


//启动一次计数
static uint8_t __pmt_return_once(void *ctr)
{
    uint16_t len = 3;
    uint8_t buf[3];
    uint8_t ret = RT_FALSE;

    buf[0] = 'R';
    buf[1] = '\r';
    buf[2] = '\n';

    clear_rx();
    if (pmt_send_cmd(buf, len) == len)
    {
        len = pmt_wait_ack(Msec(5));
        LOG_I("recv len:%d", len);
        LOG_HEX("data", 10, rx_buf, len);
        if (len == 6)
        {
            ret = RT_TRUE;
        }
        else
        {
            LOG_E("recv len err");
        }
    }
    return ret;
}


