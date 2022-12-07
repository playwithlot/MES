#include "dev_scanner.h"
#include "rtthread.h"
#include "rtdevice.h"
#include "drv_gpio.h"


#define LOG_TAG "scanner"
#define LOG_LVL LOG_LVL_DBG // LOG_LVL_INFO
#include <ulog.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//劳易测的相关操作命令
#define LEUZE_CMD_reset "CFR"
#define LEUZE_CMD_set_raw_mode "++++FWCMSOR1"
#define LEUZE_CMD_once_start "RDCMXEV1,P11,P20"
#define LEUZE_CMD_once_stop "RDCMXEV1,P10"
#define LEUZE_CMD_continue_start "CDOPSMD2"
#define LEUZE_CMD_continue_stop "CDOPSMD0"


static rt_device_t scanner_dev = RT_NULL; 		
static struct rt_semaphore scanner_rx_sem;  

//init gpio for scanner
static int scanner_gpio_inint(void)
{
    rt_pin_mode(SCANNER_PWR_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(SCANNER_PWR_PIN, PIN_LOW);
	
	rt_pin_mode(SCANNER_TRIG_PIN, PIN_MODE_INPUT_PULLUP);
   
    return 0;
}
INIT_BOARD_EXPORT(scanner_gpio_inint);


uint8_t __scanner_pwr_on(void *cmd)
{
    rt_pin_write(SCANNER_PWR_PIN, PIN_HIGH);
    return TRUE;
}
MSH_CMD_EXPORT_ALIAS(__scanner_pwr_on, scanpwron, "scanner power on");


uint8_t __scanner_pwr_off(void *cmd)
{
    rt_pin_write(SCANNER_PWR_PIN, PIN_LOW);
    return TRUE;
}
MSH_CMD_EXPORT_ALIAS(__scanner_pwr_off, scanpwroff, "scanner power off");

void scaner_trigger(void)
{
//    rt_pin_write(SCANNER_TRIG_PIN, PIN_HIGH);
//    rt_thread_delay(Msec(5));
//    rt_pin_write(SCANNER_TRIG_PIN, PIN_LOW);
}
MSH_CMD_EXPORT(scaner_trigger, "scanner trigger");

uint8_t __scanner_start(void *cmd, uint8_t type, uint8_t mode)
{
    uint8_t res = TRUE;

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
			
			case SCANNER_TYPE_leuze_BCL95:
			{
				if (mode == SCANNER_MODE_leuze_once)
				{
						uint8_t len = 4;
						const uint8_t buf[4] = {0x02, 0x2b, 0x0d, 0x0a};
						if (rt_device_write(scanner_dev, 0, &buf[0], len) != len)
						{
								LOG_D("write err\n");
						}
				}
				else if (mode == SCANNER_MODE_leuze_continue)
				{
						uint8_t len = 13;
						uint8_t buf[13] =  { 0x02, 0x50, 0x54 ,0x30, 0x30, 0x32, 0x30, 0x38, 0x36 , 0x41, 0x30, 0x0D ,0x0A };
						uint8_t buf2[13] = { 0x02, 0x50, 0x54 ,0x30, 0x33, 0x32, 0x30, 0x33, 0x34 , 0x30, 0x31, 0x0D ,0x0A };
						
						//LOG_D("send", 20, buf, len);
						//rt_device_write(scanner_dev, 0, &buf[0], len) != len)
						//rt_thread_delay(Msec(10));
						
						if (rt_device_write(scanner_dev, 0, &buf2[0], len) != len)
						{
							LOG_D("write err");
						}
						
						

				}
				break;
			}
			
			
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
					//uint8_t buf[4] = {0x54, 0x52, 0x0A};
					uint8_t buf[4] = {0x54, 0x52, 0x42, 0x0A};
					//if (rt_device_write(scanner_dev, 0, &buf[0]
					if(rt_device_write(scanner_dev, 0, &buf[0], 4) != 4)
					{
							LOG_E("write err");
					}
			}
			break;
			
			case SCANNER_TYPE_keyence:
			{
					scaner_trigger();
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
			
			case SCANNER_TYPE_leuze_BCL95:
			{
				if(mode == SCANNER_MODE_leuze_once)
				{
					uint8_t len = 4;
					const uint8_t buf[4] = {0x02, 0x2d, 0x0d, 0x0a};
					LOG_D("scanner send\n");
					if (rt_device_write(scanner_dev, 0, &buf[0], len) != len)
					{
							LOG_D("write err\n");
					}
				}
				else if(mode == SCANNER_MODE_leuze_continue)
				{
					uint8_t len = 13;
					uint8_t buf[13] = {0x02, 0x50, 0x54, 0x30, 0x33, 0x32, 0x30, 0x33, 0x34, 0x30, 0x30, 0x0D, 0x0A};
					LOG_D("scanner send\n");
					if (rt_device_write(scanner_dev, 0, &buf[0], len) != len)
					{
							LOG_D("write err\n");
					}
				}
				
				break;
			}
			
			
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
					uint8_t buf[4] = {0x54, 0x52, 0x42, 0x0A};
					if (rt_device_write(scanner_dev, 0, &buf[0], 4) != 4)
					{
							LOG_E("write err");
					}
			}
			break;
			case SCANNER_TYPE_keyence:
			{
					scaner_trigger();
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


uint8_t __scanner_stop(void *cmd, uint8_t type)
{
	uint8_t res = TRUE;
	
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
			break;
		}
		
		case SCANNER_TYPE_leuze_BCL95:
		{
			//02 50 54 30 33 32 30 33 34 30 30 0D 0A
			uint8_t len = 13;
			uint8_t buf[13] = {0x02, 0x50, 0x54 ,0x30, 0x33, 0x32, 0x30, 0x33, 0x34 , 0x30, 0x30, 0x0D ,0x0A};
			LOG_D("send", 20, buf, len);
			if (rt_device_write(scanner_dev, 0, &buf[0], len) != len)
			{
				LOG_D("write err");
			}
			rt_thread_delay(Msec(50));

			break;
		}
		
		case SCANNER_TYPE_leuze:
		{
			uint8_t len = 0;
			uint8_t buf[20];

			len = sizeof(LEUZE_CMD_once_stop);
			rt_memcpy(buf, LEUZE_CMD_once_stop, len);
			buf[len - 1] = '\r'; //结束符
			LOG_I("scanner send");
			LOG_HEX("send", 20, buf, len);
			if (rt_device_write(scanner_dev, -1, &buf[0], len) != len)
			{
				LOG_E("write err");
			}
			rt_thread_delay(Msec(10));
			len = sizeof(LEUZE_CMD_continue_stop);
			rt_memcpy(buf, LEUZE_CMD_continue_stop, len);
			buf[len - 1] = '\r'; //结束符
			LOG_I("scanner send");
			LOG_HEX("send", 20, buf, len);
			if (rt_device_write(scanner_dev, -1, &buf[0], len) != len)
			{
				LOG_E("write err");
			}
			break;
		}

		case SCANNER_TYPE_cognex:
		{
			break;
		}

		case SCANNER_TYPE_keyence:
		{
			break;
		}

		default:
		{
			LOG_E("unknwo scanner type!!!");
			res = FALSE;
			break;
		}
			
	}

	return res;
}


//set baud rate for scanner
static void scanner_dev_configure(struct serial_configure *cfg, uint8_t type)
{
	switch (type)
	{
		case SCANNER_TYPE_keyence:
		{
			cfg->baud_rate = BAUD_RATE_9600;
			cfg->data_bits = DATA_BITS_7;
			cfg->stop_bits = STOP_BITS_1;
			cfg->parity = PARITY_EVEN;
			break;
		}
			
		
		case SCANNER_TYPE_leuze_CR55:
		{
			cfg->baud_rate = BAUD_RATE_9600;
			break;
		}
		
		
		case SCANNER_TYPE_leuze:
		{
			break;
		}	
		
		case SCANNER_TYPE_cognex:
		{
			cfg->baud_rate = BAUD_RATE_115200;
			break;
		}
		
		case SCANNER_TYPE_leuze_BCL95:
		{
			cfg->baud_rate = BAUD_RATE_9600;
			break;
		}
			
		default:
		{
			cfg->baud_rate = BAUD_RATE_115200;
			break;
		}
			
	}
	
	cfg->bufsz = 1024;
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
			break;
		}
			

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
				
				break;
		}
		
		
		case SCANNER_TYPE_leuze_BCL95:
		{
			//reset: 02 48 0D 0A 
			//disable repeat: 02 50 54 30 30 32 30 38 36 41 30 0D 0A
			//                02 50 54 30 30 32 30 33 34 41 30 0D 0A
			//02 50 43 32 30 0D 0A
			
			uint8_t len = 7;
			const uint8_t buf[7] = {0x02, 0x50, 0x43, 0x32, 0x30, 0x0D, 0x0A};  //reset
			if (rt_device_write(scanner_dev, 0, &buf[0], len) != len)
			{
					LOG_E("write err");
					return FALSE;
			}
			
			rt_thread_delay(Msec(1000));
			
			len = 4;
			const uint8_t buf1[4] = {0x02, 0x48, 0x0D, 0x0A};  //reset
			if (rt_device_write(scanner_dev, 0, &buf1[0], len) != len)
			{
					LOG_E("write err");
					return FALSE;
			}
			
//			rt_thread_delay(Msec(300));
//			
//			len = 13;
//			const uint8_t buf2[13] = {0x02, 0x50, 0x54 ,0x30, 0x30, 0x32, 0x30, 0x38, 0x36 , 0x41, 0x30, 0x0D ,0x0A}; //mask repeat
//			rt_device_write(scanner_dev, 0, &buf2[0], len);
			
			
			break;
		}
		
		
		case SCANNER_TYPE_cognex:
		{
			LOG_I("configure to use cognex!\n");
			break;
		}
		
		
		default:
				LOG_E("unknwo scanner type!!!");
		return FALSE;
	}
	
	return TRUE;
}

#if 0 
//uart recv callback
static volatile uint16_t recv_bytes_number = 0;
static volatile uint16_t is_byte_receiving = 0;
static rt_err_t uart_recv_callback(rt_device_t dev, rt_size_t size)
{
    //uart recv interrupt
    //rt_sem_release(&scanner_rx_sem);
	recv_bytes_number++;
	is_byte_receiving = 30;
    return RT_EOK;
}
#endif

//uart recv callback
static uint8_t data_stream[256] = {0};
static uint8_t dp_pop = 0;
static uint8_t dp_push = 0;
void stream_init(void)
{
	rt_memset(data_stream, 0, 256);
	dp_push = 0;
	dp_pop = 0;
}

inline void stream_push(uint8_t dat)
{
	data_stream[dp_push] = dat;
	dp_push++;
}

inline uint8_t stream_pop(uint8_t *dat)
{
	if(dp_pop == dp_push)
	{
		return 0;
	}
	
	*dat = data_stream[dp_pop];
	dp_pop++;
	return 1;
}

void stream_clean(void)
{
	rt_memset(data_stream, 0, 256);
	dp_push = 0;
	dp_pop = 0;
}

static rt_err_t uart_recv_callback(rt_device_t dev, rt_size_t size)
{
    //uart recv interrupt
   
	
	#if 1
	static uint8_t dat = 0;
	size = rt_device_read(scanner_dev, 0, &dat, 1);
	stream_push(dat);
	#else
	rt_sem_release(&scanner_rx_sem);
	#endif
	
    return RT_EOK;
}

uint8_t scanner_dev_init(uint8_t types)
{
	struct serial_configure cfg = RT_SERIAL_CONFIG_DEFAULT;

	//scanner stream init...
	stream_init();
	LOG_D("scanner stream init...\n");
	
	//scanner initailze
	LOG_D("scanner dev init...\n");
	
	scanner_gpio_inint();
	__scanner_pwr_on(RT_NULL);
	scanner_dev_configure(&cfg, types);
	
	//create semaphore
	rt_sem_init(&scanner_rx_sem, "rx_sem", 0, RT_IPC_FLAG_PRIO);
	
	//find uart dev
	scanner_dev = rt_device_find(SCANNER_DEV_NAME);
	if (scanner_dev == RT_NULL)
	{
		LOG_D("find %s failed\n", SCANNER_DEV_NAME);
		return FALSE;
	}

	//configure and open uart 
	rt_device_control(scanner_dev, RT_DEVICE_CTRL_CONFIG, &cfg);
	if(rt_device_open(scanner_dev, RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX) != RT_EOK)
	{
		LOG_D("open %s failed\n", SCANNER_DEV_NAME);
		return FALSE;;
	}

	//set uart recv callback
	rt_device_set_rx_indicate(scanner_dev, uart_recv_callback);

		
	//set scanner to default
	rt_thread_delay(100);
	scanner_set_default(RT_NULL, types);

	//wait for scanner dev reaady
	rt_thread_delay(1000);
	LOG_D("scanner dev ready...\n");
	//scanner_dev_ctrl(SCANNER_CMD_start, SCANNER_TYPE_leuze_BCL95, SCANNER_MODE_leuze_once);  //
		
	return TRUE;
	
}


void scanner_dev_ctrl(uint8_t cmd, uint8_t type, uint8_t mode)
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

uint8_t scanner_dev_get_data(uint8_t *buf, uint8_t len, rt_tick_t timeout)
{
	#if 1
	uint8_t recv_len = 0;
	rt_tick_t current_tick = rt_tick_get();
	
	while(1)
	{
		recv_len %= len;
		if(!stream_pop(&buf[recv_len]))
		{
			break;
		}
		
		recv_len++;
		
		if((rt_tick_get() - current_tick) > timeout)
		{
			break;
		
		}
	
	
	}
	#else
	
	uint8_t recv_len = 0;
	rt_tick_t current_tick = rt_tick_get();
	while(rt_sem_take(&scanner_rx_sem, timeout) == RT_EOK)
	{

		recv_len = rt_device_read(scanner_dev, 0, &buf[0], len);
		if(!recv_len)
		{
			break;
		}
	}
	
	#endif

	return recv_len;
   
}


uint8_t scanner_dev_clean_stream(void)
{
	uint8_t dat = 0;

	while(rt_device_read(scanner_dev, 0, &dat, 1));
	stream_clean();
	

	return 1;

   
}



void scanner_dev_write_test(void)
{
	rt_device_write(scanner_dev, 0, "hello scanner write test", strlen( "hello scanner write test"));

}

MSH_CMD_EXPORT(scanner_dev_write_test, "scanner_dev_write_test");


void scanner_dev_start(void)
{
	scanner_dev_ctrl(SCANNER_CMD_start, SCANNER_TYPE_leuze_BCL95, 0);

}

MSH_CMD_EXPORT(scanner_dev_start, "scanner_dev_start");


void scaner_reset_fac(void)
{
	uint8_t len = 7;
	const uint8_t buf[7] = {0x02, 0x50, 0x43, 0x32, 0x30, 0x0D, 0x0A};  //reset
	if(rt_device_write(scanner_dev, 0, &buf[0], len) == len)
	{
		LOG_E("scaner_reset_fac ok");
	}

}

MSH_CMD_EXPORT(scaner_reset_fac, "scaner_reset_fac");

