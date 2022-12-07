/**
 * @file scanner.c
 * @author King (lzj..@com)
 * @brief  扫码枪处理线程。
 * @version 0.1
 * @date 2021-09-27
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "scanner.h"
#include "rtthread.h"
#include "rtdevice.h"
#include "drv_gpio.h"


#define LOG_TAG "scanner"
#define LOG_LVL LOG_LVL_DBG // LOG_LVL_INFO
#include <ulog.h>

//扫码枪类型
#define SCANNER_TYPE_cognex 0     //康耐视
#define SCANNER_TYPE_keyence 1    //基恩士
#define SCANNER_TYPE_leuze 2      //德国劳易测
#define SCANNER_TYPE_leuze_CR55 3 //德国劳易测

//扫码枪工作模式
#define SCANNER_MODE_leuze_once 0     //单次模式(连续扫描，扫到后停止)
#define SCANNER_MODE_leuze_continue 1 //连续模式（连续扫描，扫到后返回码，然后继续扫）

//劳易测的相关操作命令
#define LEUZE_CMD_reset "CFR"
#define LEUZE_CMD_set_raw_mode "++++FWCMSOR1"
#define LEUZE_CMD_once_start "RDCMXEV1,P11,P20"
#define LEUZE_CMD_once_stop "RDCMXEV1,P10"
#define LEUZE_CMD_continue_start "CDOPSMD2"
#define LEUZE_CMD_continue_stop "CDOPSMD0"

#define RX_BUF_SIZE 256
#define SCANNER_DEV_NAME "uart6"
#define SCANNER_DEV_BAUD BAUD_RATE_115200

#define SCANNER_TRIG_PIN GET_PIN(B, 5)
#define SCANNER_PWR_PIN GET_PIN(B, 6)

rt_mq_t scanner_handle_queue = RT_NULL;
static struct rt_thread scanner_send_handle;
static struct rt_thread scanner_recv_handle;

static rt_device_t scanner_dev = RT_NULL; //串口驱动
static struct rt_semaphore scanner_rx_sem;        /* 用于接收消息的信号量 */
static uint8_t recv_buf[RX_BUF_SIZE];




//将串口中的内容全部读出来丢弃
static void clear_rx(void)
{
    uint8_t buf[10];
    uint8_t len;

    do
    {
        len = rt_device_read(scanner_dev, 0, &buf, 10);
    } while (len);
    rt_sem_control(&scanner_rx_sem, RT_IPC_CMD_RESET, (void *)0);
    rt_memset(recv_buf, 0, sizeof(recv_buf));

}


//启动时尽早把触发引脚设置为合适的模式
static int scanner_pre_inint(void)
{
    rt_pin_mode(SCANNER_TRIG_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(SCANNER_TRIG_PIN, PIN_LOW);
    return 0;
}
INIT_BOARD_EXPORT(scanner_pre_inint);


static int scanner_init(void)
{
    return 0;
}
INIT_ENV_EXPORT(scanner_init);



uint8_t __scanner_pwr_on(void *cmd)
{
    LOG_I("scanner pwr on");
    rt_pin_write(SCANNER_PWR_PIN, PIN_HIGH);
    return TRUE;
}
MSH_CMD_EXPORT_ALIAS(__scanner_pwr_on, scanpwron, "scanner power on");


uint8_t __scanner_pwr_off(void *cmd)
{
    LOG_I("scanner pwr off");
    rt_pin_write(SCANNER_PWR_PIN, PIN_LOW);
    return TRUE;
}
MSH_CMD_EXPORT_ALIAS(__scanner_pwr_off, scanpwroff, "scanner power off");


//set baud rate for scanner
void scanner_dev_init(struct serial_configure *config, uint8_t type)
{
	switch (type)
	{
		case SCANNER_TYPE_keyence:
			config->baud_rate = BAUD_RATE_9600;
			config->data_bits = DATA_BITS_7;
			config->stop_bits = STOP_BITS_1;
			config->parity = PARITY_EVEN;
		break;
		
		case SCANNER_TYPE_leuze_CR55:
			config->baud_rate = BAUD_RATE_9600;
		break;
		case SCANNER_TYPE_leuze:
		case SCANNER_TYPE_cognex:
			
		default:
			config->baud_rate = BAUD_RATE_115200;
		break;
	}
	
	config->bufsz = 1024;
}


//set scanner work mode
static uint8_t scanner_set_default(void *cmd, uint8_t type)
{
	LOG_I("scanner set defult");
	switch (type)
	{
		case SCANNER_TYPE_leuze_CR55:
		{
				{
						//开启
						uint8_t len = 1;
						uint8_t buf[1] = {0x0e};
						LOG_I("scanner send");
						LOG_HEX("send", 20, buf, len);
						if (rt_device_write(scanner_dev, -1, &buf, len) != len)
						{
								LOG_E("write err");
								return FALSE;
						}
						rt_thread_delay(Sec(1));
				}
				{
						//恢复出厂
						uint8_t len = 8;
						uint8_t buf[8] = {0XBA, 0X06, 0X25, 0X2B, 0X2B, 0X25, 0XFE, 0XA0};
						LOG_I("scanner send");
						LOG_HEX("send", 20, buf, len);
						if (rt_device_write(scanner_dev, -1, &buf, len) != len)
						{
								LOG_E("write err");
								return FALSE;
						}
						rt_thread_delay(Sec(1));
				}
				{
						//停止
						uint8_t len = 4;
						uint8_t buf[4] = {0x1b, 0x41, 0x31, 0x0d};
						LOG_I("scanner send");
						LOG_HEX("send", 20, buf, len);
						if (rt_device_write(scanner_dev, -1, &buf, len) != len)
						{
								LOG_E("write err");
								return FALSE;
						}
						rt_thread_delay(Sec(1));
				}
		}
		break;
		
		case SCANNER_TYPE_leuze:
		{
				uint8_t buf[20];
				uint8_t len;

				len = sizeof(LEUZE_CMD_reset);
				rt_memcpy(buf, LEUZE_CMD_reset, len);
				buf[len - 1] = '\r'; //结束符
				LOG_I("scanner send");
				LOG_HEX("send", 20, buf, len);
				if (rt_device_write(scanner_dev, -1, &buf, len) != len)
				{
						LOG_E("write err");
						return FALSE;
				}
				rt_thread_delay(Sec(1));
				len = sizeof(LEUZE_CMD_set_raw_mode);
				rt_memcpy(buf, LEUZE_CMD_set_raw_mode, len);
				buf[len - 1] = '\r'; //结束符
				LOG_I("scanner send");
				LOG_HEX("send", 20, buf, len);
				if (rt_device_write(scanner_dev, -1, &buf, len) != len)
				{
						LOG_E("write err");
						return FALSE;
				}
				rt_thread_delay(Sec(1));
				len = sizeof(LEUZE_CMD_continue_start);
				rt_memcpy(buf, LEUZE_CMD_continue_start, len);
				buf[len - 1] = '\r'; //结束符
				LOG_I("scanner send");
				LOG_HEX("send", 20, buf, len);
				if (rt_device_write(scanner_dev, -1, &buf, len) != len)
				{
						LOG_E("write err");
						return FALSE;
				}
				rt_thread_delay(Msec(500));
				len = sizeof(LEUZE_CMD_continue_stop);
				rt_memcpy(buf, LEUZE_CMD_continue_stop, len);
				buf[len - 1] = '\r'; //结束符
				LOG_I("scanner send");
				LOG_HEX("send", 20, buf, len);
				if (rt_device_write(scanner_dev, -1, &buf, len) != len)
				{
						LOG_E("write err");
						return FALSE;
				}
		}
		break;
		
		default:
				LOG_E("unknwo scanner type!!!");
		return FALSE;
	}
	
	return TRUE;
}


void scantrig(void)
{
    rt_pin_write(SCANNER_TRIG_PIN, PIN_HIGH);
    rt_thread_delay(Msec(5));
    rt_pin_write(SCANNER_TRIG_PIN, PIN_LOW);
}
MSH_CMD_EXPORT(scantrig, "scanner trigeer");




//恢复默认值
uint8_t scanner_reset(void *cmd, uint8_t type)
{
    switch(type)
    {
    case SCANNER_TYPE_leuze:
    {
        uint8_t buf[] = "CFR";
        uint8_t len;
        len = sizeof(buf);
        buf[len - 1] = '\r'; //结束符
        LOG_HEX("send", 20, buf, len);
        if (rt_device_write(scanner_dev, -1, &buf, len) != len)
        {
            LOG_E("write err");
        }
        rt_thread_delay(Sec(3));
    }
    break;

    default:
        LOG_E("unknwo scanner type!!!");
        return FALSE;
    }
    return scanner_set_default(RT_NULL, type);
}

uint8_t __scanner_stop(void *cmd, uint8_t type)
{
    switch(type)
    {
    case SCANNER_TYPE_leuze_CR55:
    {
        uint8_t len = 4;
        uint8_t buf[4] = {0x1b, 0x41, 0x31, 0x0d};
        LOG_I("scanner send");
        LOG_HEX("send", 20, buf, len);
        if (rt_device_write(scanner_dev, -1, &buf, len) != len)
        {
            LOG_E("write err");
        }
        rt_thread_delay(Msec(200));
    }
    break;
    case SCANNER_TYPE_leuze:
    {
        uint8_t len = 0;
        uint8_t buf[20];

        len = sizeof(LEUZE_CMD_once_stop);
        rt_memcpy(buf, LEUZE_CMD_once_stop, len);
        buf[len - 1] = '\r'; //结束符
        LOG_I("scanner send");
        LOG_HEX("send", 20, buf, len);
        if (rt_device_write(scanner_dev, -1, &buf, len) != len)
        {
            LOG_E("write err");
        }
        rt_thread_delay(Msec(10));
        len = sizeof(LEUZE_CMD_continue_stop);
        rt_memcpy(buf, LEUZE_CMD_continue_stop, len);
        buf[len - 1] = '\r'; //结束符
        LOG_I("scanner send");
        LOG_HEX("send", 20, buf, len);
        if (rt_device_write(scanner_dev, -1, &buf, len) != len)
        {
            LOG_E("write err");
        }
    }
    break;
    case SCANNER_TYPE_cognex:
    {
        uint8_t buf[2];
        buf[0] = '-';
        if (rt_device_write(scanner_dev, -1, &buf, 1) != 1)
        {
            LOG_E("write err");
        }
    }
    break;
    case SCANNER_TYPE_keyence:
    {
    }
    break;

    default:
        LOG_E("unknwo scanner type!!!");
        return FALSE;
    }
    clear_rx();
    return TRUE;
}


uint8_t __scanner_start(void *cmd, uint8_t type, uint8_t mode)
{
    uint8_t res = TRUE;

    clear_rx();
    //fifo_clear(&code_data_fifo);
    switch (type)
    {
			case SCANNER_TYPE_leuze_CR55:
			{
					if (mode == SCANNER_MODE_leuze_once)
					{
							uint8_t len = 4;
							uint8_t buf[4] = {0x1b, 0x41, 0x32, 0x0d};
							LOG_I("scanner send");
							LOG_HEX("send", 20, buf, len);
							if (rt_device_write(scanner_dev, -1, &buf, len) != len)
							{
									LOG_E("write err");
							}
					}
					else if (mode == SCANNER_MODE_leuze_continue)
					{
							uint8_t len = 4;
							uint8_t buf[4] = {0x1b, 0x41, 0x30, 0x0d};
							LOG_I("scanner send");
							LOG_HEX("send", 20, buf, len);
							if (rt_device_write(scanner_dev, -1, &buf, len) != len)
							{
									LOG_E("write err");
							}
					}
			}
			break;
			
			case SCANNER_TYPE_leuze:
			{
					// uint8_t buf[] = "CDOPSMD2";
					uint8_t len = 0;
					uint8_t buf[20];
					if (mode == SCANNER_MODE_leuze_once)
					{
							len = sizeof(LEUZE_CMD_once_start);
							rt_memcpy(buf, LEUZE_CMD_once_start, len);
					}
					else if (mode == SCANNER_MODE_leuze_continue)
					{
							len = sizeof(LEUZE_CMD_continue_start);
							rt_memcpy(buf, LEUZE_CMD_continue_start, len);
					}
					if (len > 0)
					{
							buf[len - 1] = '\r'; //结束符
							LOG_I("scanner send");
							LOG_HEX("send", 20, buf, len);
							if (rt_device_write(scanner_dev, -1, &buf, len) != len)
							{
									LOG_E("write err");
							}
					}
			}
			break;
			
			case SCANNER_TYPE_cognex:
			{
					uint8_t buf[2];
					buf[0] = '+';
					if (rt_device_write(scanner_dev, -1, &buf, 1) != 1)
					{
							LOG_E("write err");
					}
			}
			break;
			case SCANNER_TYPE_keyence:
			{
					scantrig();
			}
			break;
			
			
			default:
					LOG_E("unknwo scanner type!!!");
			return FALSE;
    }
		
    //len = scanner_rx_data(cmd->timeout_tick);
    //__scanner_stop(cmd, type);
  
    return res;
}

uint8_t __scanner_start_10(void *cmd, uint8_t type, uint8_t mode)
{
    uint8_t res = TRUE;

    clear_rx();

		
    switch(type)
    {
			case SCANNER_TYPE_leuze_CR55:
			{
					if(mode == SCANNER_MODE_leuze_once)
					{
							uint8_t len = 4;
							uint8_t buf[4] = {0x1b, 0x41, 0x32, 0x0d};
							LOG_I("scanner send");
							LOG_HEX("send", 20, buf, len);
							if (rt_device_write(scanner_dev, -1, &buf, len) != len)
							{
									LOG_E("write err");
							}
					}
					else if(mode == SCANNER_MODE_leuze_continue)
					{
							uint8_t len = 4;
							uint8_t buf[4] = {0x1b, 0x41, 0x30, 0x0d};
							LOG_I("scanner send");
							LOG_HEX("send", 20, buf, len);
							if (rt_device_write(scanner_dev, -1, &buf, len) != len)
							{
									LOG_E("write err");
							}
					}
			}
			break;
			
			case SCANNER_TYPE_leuze:
			{
					// uint8_t buf[] = "CDOPSMD2";
					uint8_t len = 0;
					uint8_t buf[20];
					if(mode == SCANNER_MODE_leuze_once)
					{
							len = sizeof(LEUZE_CMD_once_start);
							rt_memcpy(buf, LEUZE_CMD_once_start, len);
					}
					else if(mode == SCANNER_MODE_leuze_continue)
					{
							len = sizeof(LEUZE_CMD_continue_start);
							rt_memcpy(buf, LEUZE_CMD_continue_start, len);
					}
					if (len > 0)
					{
							buf[len - 1] = '\r'; //结束符
							LOG_I("scanner send");
							LOG_HEX("send", 20, buf, len);
							if (rt_device_write(scanner_dev, -1, &buf, len) != len)
							{
									LOG_E("write err");
							}
					}
			}
			break;
			case SCANNER_TYPE_cognex:
			{
					uint8_t buf[2];
					buf[0] = '+';
					if (rt_device_write(scanner_dev, -1, &buf, 1) != 1)
					{
							LOG_E("write err");
					}
			}
			break;
			case SCANNER_TYPE_keyence:
			{
					scantrig();
			}
			break;
			default:
					LOG_E("unknwo scanner type!!!");
					return FALSE;
    }
		
    //len = scanner_rx_data(cmd->timeout_tick);
    //rt_thread_delay(Msec(10));
    //__scanner_stop(cmd, type);

    return res;
}







//uart recv callback
static rt_err_t uart_recivece(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&scanner_rx_sem);

    return RT_EOK;
}



static void scanner_recv_cmd_handle(uint8_t cmd, uint8_t type, uint8_t mode)
{
	switch(cmd)
	{
		case SCANNER_CMD_pwr_on:
		{
			__scanner_pwr_on(RT_NULL);
			break;
		}
		
		case SCANNER_CMD_pwr_off:
		{
			__scanner_pwr_off(RT_NULL);
			break;
		}
		
		case SCANNER_CMD_start:
		{
			__scanner_start(RT_NULL, type, mode);
			break;
		}
		
		case SCANNER_CMD_stop:
		{
			__scanner_stop(RT_NULL, type);
			break;
		}
		
		case SCANNER_CMD_start_10:
		{
			__scanner_start_10(RT_NULL, type, mode);
			break;
		}
	
	
	}



}


void create_scanner_frame(uint8_t cmd, uint8_t type, uint8_t mode)
{
	uint32_t dp = 0;
	scanner_frame_t *frame = RT_NULL;

	//frame = rt_malloc(sizeof(can_frame_t));
	
	if(frame)
	{
		frame->cmd = cmd;
		frame->type = type;
		frame->mode = mode;
		
		dp = (uint32_t)frame;
		rt_mq_send(scanner_handle_queue, &dp, sizeof(void*));  

	}
	
}

void scanner_send_thread(void *args)
{

	uint32_t dp = 0;
	scanner_frame_t *scaner_frame = RT_NULL;
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

	//scanner initailze
	rt_pin_mode(SCANNER_PWR_PIN, PIN_MODE_OUTPUT);
	__scanner_pwr_on(RT_NULL);
	scanner_dev_init(&config, SCANNER_TYPE_leuze_CR55);
	
	//init / create semaphore
	rt_sem_init(&scanner_rx_sem, "rx_sem", 0, RT_IPC_FLAG_PRIO);
	
	//fine and opend uart dev
	while (1) 
	{
		scanner_dev = rt_device_find(SCANNER_DEV_NAME);
		if (scanner_dev == RT_NULL)
		{
				LOG_E("find %s failed", SCANNER_DEV_NAME);
				rt_thread_delay(Msec(100));
		}
		else
		{
				rt_device_control(scanner_dev, RT_DEVICE_CTRL_CONFIG, &config);
				if(rt_device_open(scanner_dev, RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX) != RT_EOK)
				{
						rt_thread_delay(Msec(100));
						continue;
				}
				//set uart recv callback
				rt_device_set_rx_indicate(scanner_dev, uart_recivece);
				break;
		}
	}
	
	//set scanner 
	rt_thread_delay(Sec(3));
	scanner_set_default(RT_NULL, SCANNER_TYPE_leuze_CR55);
	//__scanner_pwr_off(); //power off
	clear_rx();
	LOG_I("scanner ready...");
	
	//recv and handle cmd
	while(1)
	{
		rt_mq_recv(scanner_handle_queue, &dp, sizeof(scanner_frame_t), RT_WAITING_FOREVER);
		scaner_frame = (scanner_frame_t*)(dp);
		scanner_recv_cmd_handle(scaner_frame->cmd, scaner_frame->type, scaner_frame->mode);
		
		if(scaner_frame)
		{
			rt_free(scaner_frame);
		}
		
	
	}
	
}


void scanner_recv_thread(void *args)
{
    uint16_t len = 0;

    while (1)
    {
        rt_sem_take(&scanner_rx_sem, RT_WAITING_FOREVER);
        len = rt_device_read(scanner_dev, 0, &recv_buf[0], RX_BUF_SIZE);
				
				for(uint8_t i = 0; i < len; i++)
				{
					rt_kprintf("%02x ", recv_buf[i]);
				}
				rt_kprintf("\n");
				

    }

}


void create_scanner_send_thread(void)
{
		rt_err_t err;
	

    err = rt_thread_init(&scanner_send_handle, "scanner send thread", scanner_send_thread, RT_NULL,
                         (void *)SCANNER_SEND_THREAD_STACK_ADDR,
                         SCANNER_SEND_THREAD_STACK_SIZE,
                         SCANNER_SEND_THREAD_PRIORITY, 20);
    if (err == RT_EOK)
    {
        rt_thread_startup(&scanner_send_handle);
    }
		
}


void create_scanner_recv_thread(void)
{
		rt_err_t err;
	

    err = rt_thread_init(&scanner_recv_handle, "scanner recv thread", 
						 scanner_recv_thread, RT_NULL,
                         (void *)SCANNER_RECV_THREAD_STACK_ADDR,
                         SCANNER_RECV_THREAD_STACK_SIZE,
                         SCANNER_RECV_THREAD_PRIORITY, 20);
    if (err == RT_EOK)
    {
        rt_thread_startup(&scanner_recv_handle);
    }
		
}
