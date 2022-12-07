/**
 * @file rfid_reader.c
 * @author King (lzj..@com)
 * @brief rfid读卡器处理。
 * @version 0.1
 * @date 2022-01-13
 *
 * @copyright Copyright (c) 2022
 *
 * @par 修改日志:
 * 修改内容                       修改人       修改日期
 * 初版                           lzj         2022-01-13
 */
#include "rtthread.h"
#include "rtdevice.h"
#include "rfid_reader.h"


#define LOG_TAG "rfid"
#define LOG_LVL LOG_LVL_INFO
#include <ulog.h>

static rfid_config_t rfid_cfg = 
{ 
	.channel = RFID_CH_1, .baud = BAUD_RATE_115200, .dev_name = "uart2"
};

static rfid_info_t rfid_info;
static struct rt_semaphore refid_rx_sem;   
//static uart_dev = ;
static rt_device_t uart_dev;


//uart recv callback
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&refid_rx_sem);
	
    return RT_EOK;
}

static void rfid_dev_init(rfid_config_t *rfid_dev)
{
    struct serial_configure serial_cfg = RT_SERIAL_CONFIG_DEFAULT;
    static uint8_t is_first_try = TRUE;
	
    serial_cfg.baud_rate = rfid_dev->baud;
    serial_cfg.bufsz = 256;
		rt_sem_init(&refid_rx_sem, "refid_rx_sem", 0, RT_IPC_FLAG_FIFO);
	
    while (1) //打开串口设备
    {
        rfid_dev->dev_handle = rt_device_find(&rfid_dev->dev_name[0]);
        if (rfid_dev->dev_handle == RT_NULL)
        {
            if (is_first_try)
            {
                is_first_try = FALSE;
                LOG_E("find %s failed", rfid_dev->dev_name);
            }
            rt_thread_delay(Msec(500));
        }
        else
        {
            rt_device_control(rfid_dev->dev_handle, RT_DEVICE_CTRL_CONFIG, &serial_cfg);
            if (rt_device_open(rfid_dev->dev_handle, RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX) != RT_EOK)
            {
                rt_thread_delay(Msec(100));
                continue;
            }
            /* 设置接收回调函数 */
            rt_device_set_rx_indicate(rfid_dev->dev_handle, uart_input);
            break;
        }
    }
}



void rfid_init(void)
{
	uint8_t i;
	rt_err_t result;
	
	rt_kprintf("uart dev initail...\n");
	rfid_dev_init(&rfid_cfg);
	rt_kprintf("uart dev is done!\n");
}

static uint8_t _cal_vef(uint8_t *buf, uint8_t buf_len)
{
    uint8_t vef = 0;
    uint8_t i;

    for (i = 0; i < buf_len; i++)
    {
        vef += buf[i];
    }
    return vef;
}


void send_cmd(rfid_info_t *info, uint8_t *cmd_buf, uint8_t cmd_buf_len)
{
    uint8_t i;
    uint8_t vef = 0;
    if ((cmd_buf_len * 2) > sizeof(info->cmd_buf))
    {
        return;
    }
    info->is_exe = TRUE;
    info->cmd_buf[0] = YZ_FRAM_head;
    vef = _cal_vef(cmd_buf, cmd_buf_len);
    info->cmd_len = 1;
    for (i = 0; i < cmd_buf_len; i++)
    {
        if (cmd_buf[i] == YZ_FRAM_head || cmd_buf[i] == YZ_FRAM_esc || cmd_buf[i] == YZ_FRAM_end)
        {
            info->cmd_buf[info->cmd_len] = YZ_FRAM_esc;
            info->cmd_len++;
        }
        info->cmd_buf[info->cmd_len] = cmd_buf[i];
        info->cmd_len++;
    }
    info->cmd_buf[info->cmd_len] = vef;
    info->cmd_len++;
    info->cmd_buf[info->cmd_len] = YZ_FRAM_end;
    info->cmd_len++;
    info->rec_state = REC_STATE_wait_head;
    rt_device_write(info->cfg->dev_handle, -1, info->cmd_buf, info->cmd_len);
    info->send_tick = rt_tick_get();
    info->cmd_state = CMD_STATE_wait_ack;
    LOG_D("ch:%d send cmd:%x(h)", info->con->channel, info->curent_cmd);
    LOG_HEX("send", 10, info->cmd_buf, info->cmd_len);
}


void get_version(rfid_info_t *info)
{
    uint8_t buf[] = {0, 0, 0x03, YZ_CMD_get_hardversion};
    info->curent_cmd = YZ_CMD_get_hardversion;
    send_cmd(info, buf, sizeof(buf));
}


static uint8_t _get_version_recv_ack_deal(rfid_info_t *info)
{
    uint8_t ret = FALSE;
    Yz_frame_t *frame;

    frame = (Yz_frame_t *)&info->buf[0];
    if (info->curent_cmd != YZ_CMD_get_hardversion ||
        frame->cmd != YZ_CMD_get_hardversion ||
        frame->len < 5 ||
        frame->buf[0] != 0)
    {
        info->err = NET_CMD_ERR_exe_faile;
        return ret;
    }
    ret = TRUE;
    info->hard_vesion[3] = frame->buf[1];
    info->hard_vesion[2] = frame->buf[2];
		
    return ret;
}

static uint8_t _def_recv_ack_deal(rfid_info_t *info)
{
    uint8_t ret = FALSE;
    Yz_frame_t *f;

    f = (Yz_frame_t *)&info->buf[0];
    if (f->cmd != info->curent_cmd ||
        f->len < 4 ||
        f->buf[0] != 0)
    {
        info->err = NET_CMD_ERR_exe_faile;
        return ret;
    }
    ret = TRUE;
    return ret;
}


static uint8_t _vef_keya_recv_ack_deal(rfid_info_t *info)
{
    uint8_t ret = FALSE;
    Yz_frame_t *f;

    f = (Yz_frame_t *)&info->buf[0];
    if (f->cmd != YZ_CMD_vef_keyA ||
        f->len < 3 ||
        f->buf[0] != 0)
    {
        info->err = NET_CMD_ERR_exe_faile;
        return ret;
    }
    ret = TRUE;
    return ret;
}



static uint8_t _read_3_block_recv_ack_deal(rfid_info_t *info)
{
    uint8_t ret = FALSE;
    Yz_frame_t *f;

    f = (Yz_frame_t *)&info->buf[0];
    if (info->curent_cmd != YZ_CMD_read_3_block ||
        f->cmd != YZ_CMD_read_3_block ||
        f->len < 48 ||
        f->buf[0] != 0)
    {
        info->err = NET_CMD_ERR_exe_faile;
        return ret;
    }
    ret = TRUE;
    rt_memcpy(&info->data[16 * info->para], &f->buf[1], 48);
    return ret;
}


static uint8_t _read_1_block_recv_ack_deal(rfid_info_t *info)
{
    uint8_t ret = FALSE;
    Yz_frame_t *f;

    f = (Yz_frame_t *)&info->buf[0];
    if (info->curent_cmd != YZ_CMD_read_1_block ||
        f->cmd != YZ_CMD_read_1_block ||
        f->len < 16 ||
        f->buf[0] != 0)
    {
        info->err = NET_CMD_ERR_exe_faile;
        return ret;
    }
    ret = TRUE;
    rt_memcpy(&info->data[16 * info->para], &f->buf[1], 16);
    return ret;
}


void rfid_send_thread(void *args)
{
	uint8_t i = 0;
	rt_err_t err = 0; 

	
	rt_thread_delay(100);
	//get_version(&rfid_info);
	
	while(1)
	{
	
		rt_thread_delay(Msec(1));
	}
	
	
}





#if 1
typedef struct
{
	uint8_t size;
	uint8_t buf[100];

} uart_rx_msg_t;

uart_rx_msg_t recv_msg;
static rt_mq_t rfid_recv_mq = RT_NULL;



static uint8_t uart2_recv_buf[100];
static uint8_t pos = 0;
static uint32_t timeout_tick = 0;
static uint32_t recv_len = 0;
uint8_t rfid_get_frame(rt_device_t dev, uint8_t *buf, uint32_t timeout)
{ 
	recv_len = 0;
	
	if(pos == 0)
	{
		timeout_tick = rt_tick_get();
	}
	
	if(rt_device_read(dev, 0, &uart2_recv_buf[pos], 1))
	{
		pos++;
	}
	
	if(rt_tick_get() - timeout_tick > timeout)
	{	
		recv_len = pos;
		rt_memcpy(buf, &uart2_recv_buf[0], recv_len);
		pos = 0;

	}
	
	return recv_len;
	
}


//uart recv callback
static rt_err_t uart_recv_dma(rt_device_t dev, rt_size_t size)
{
		uint32_t dp = 0;
    rt_err_t result = 0;
		
//		if(rfid_recv_mq)
//		{
//			recv_msg.size = size;
//			rt_device_read(dev, 0, recv_msg.buf, size);
//			dp = (uint32_t)&recv_msg;
//			result = rt_mq_send(rfid_recv_mq, &dp, sizeof(void*));
//		}
		recv_msg.size = rfid_get_frame(dev, recv_msg.buf, 60);
		if(recv_msg.size && rfid_recv_mq)
		{
			dp = (uint32_t)&recv_msg;
			result = rt_mq_send(rfid_recv_mq, &dp, sizeof(void*));
		}
	
    if ( result == -RT_EFULL)
    {
        rt_kprintf("message queue full!\n");
    }
		
    return result;
	
}

static void rfid_dev_init_dma(rfid_config_t *rfid_dev)
{
    struct serial_configure serial_cfg = RT_SERIAL_CONFIG_DEFAULT;

    //serial_cfg.baud_rate = rfid_dev->baud;
    //serial_cfg.bufsz = 256;
	
		rfid_recv_mq = rt_mq_create("rfid_recv_mq", sizeof(void*), 10, RT_IPC_FLAG_FIFO);
		if(local_handle_queue != RT_NULL)
		{
			rt_kprintf("create rfid_recv_mq sucessful!\n");
		
		}

	
    while (1) //打开串口设备
    {
        uart_dev = rt_device_find("uart6");
				//uart_dev = rfid_dev->dev_handle;
			
        if (uart_dev == RT_NULL)
        {
						rt_kprintf("find %s failed", rfid_dev->dev_name);
            rt_thread_delay(Msec(500));
        }
        else
        {
            //rt_device_control(uart_dev, RT_DEVICE_CTRL_CONFIG, &serial_cfg);
            if (rt_device_open(uart_dev, RT_DEVICE_FLAG_INT_RX|RT_DEVICE_FLAG_INT_TX) != RT_EOK)
            {
                rt_thread_delay(Msec(100));
								rt_kprintf("open %s failed", rfid_dev->dev_name);
                continue;
            }
            /* 设置接收回调函数 */
            rt_device_set_rx_indicate(uart_dev, uart_recv_dma);
						
						
            break;
        }
    }
}
#endif

static struct rt_thread rfid_send_handle;
static struct rt_thread rfid_recv_handle;

void create_rfid_send_thread(void)
{
		rt_err_t err;
	
    err = rt_thread_init(&rfid_send_handle, "rfid_send_thread", 
                         rfid_send_thread, 
                         RT_NULL,
                         (void *)RFID_SEND_THREAD_STACK_BASE,
                         RFID_SEND_THREAD_STACK_SIZE,
                         RFID_SEND_THREAD_PRIORITY, 
												 RFID_SEND_THREAD_TIME_SLINCE);
    if (err == RT_EOK)
    {
			rt_kprintf("rfid_send_thread done!\n");
			rt_thread_startup(&rfid_send_handle);
    }
		
}


static void rfid_recv_thread(void *args)
{
    rt_err_t res = 0;
    rt_uint32_t dp = 0;
		uart_rx_msg_t *msg = NULL;

    while (1)
    {
        res = rt_mq_recv(rfid_recv_mq, &dp, sizeof(void*), RT_WAITING_FOREVER);
				msg = (uart_rx_msg_t*)dp;
        
				if (res == RT_EOK)
        {
						rt_kprintf("uart2 recv %d bytes: %s\n", msg->size, msg->buf);
						


				
        }
    }
}



void rfid_init_dma(void)
{
	uint8_t i;
	rt_err_t result;
	
	rt_kprintf("uart dev initail...\n");
	rfid_dev_init_dma(&rfid_cfg);
	rt_kprintf("uart dev is done!\n");
}


void create_rfid_recv_thread(void)
{
		rt_err_t err;
			 
		//rfid_init();
		rfid_init_dma();
	
    err = rt_thread_init(&rfid_recv_handle, "rfid_recv_handle", 
                         rfid_recv_thread, 
                         RT_NULL,
                         (void *)RFID_RECV_THREAD_STACK_BASE,
                         RFID_RECV_THREAD_STACK_SIZE,
                         RFID_RECV_THREAD_PRIORITY, 
												 RFID_RECV_THREAD_TIME_SLINCE);
    if (err == RT_EOK)
    {
				rt_kprintf("rfid_recv_thread done!\n");
        rt_thread_startup(&rfid_recv_handle);
    }
		
}


void rfid_send_test(void)
{
	//const uint8_t buf[] = {"hello rt_device write test!\n"};
	rt_device_write(uart_dev, 0, "hello rt_device write test!\n", 20);

}




MSH_CMD_EXPORT(rfid_send_test, "rfid_send test");
