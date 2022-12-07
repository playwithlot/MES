#include "rtthread.h"
#include "rtdevice.h"
#include "drv_gpio.h"
#include "dev_pmt.h"

#include "dev_flash.h"
#include "CRC16.h"
#include "stdio.h"
#define LOG_TAG "pmt"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>


static rt_device_t pmt_dev = RT_NULL; 	
static pmt_param_t pmt_dev_info = {0};

static void pmt_param_reset_default(pmt_param_t *dev_param)
{
    rt_memset(dev_param->version, 0, sizeof(dev_param->version));
    dev_param->resolution = 17;
    dev_param->gating_time = 1;
}

void pmt_param_save(pmt_param_t *dev_param)
{
    uint16_t crc;
    uint16_t len = ((uint32_t)&dev_param->crc - (uint32_t)&dev_param);

    crc = CRC16_modbus((uint8_t *)&dev_param, len);
    dev_param->crc = crc;
    len += sizeof(dev_param->crc);
    dev_flash_write(SPI_FLASH_PMT_PARA_ADDR, (uint8_t *)&dev_param, len);
}

void pmt_param_get(pmt_param_t *dev_param)
{
    uint16_t crc;
    uint16_t len = ((uint32_t)&dev_param->crc - (uint32_t)&dev_param);

    dev_flash_read(SPI_FLASH_PMT_PARA_ADDR, (uint8_t *)&dev_param, len + sizeof(dev_param->crc));
    crc = CRC16_modbus((uint8_t *)&dev_param, len);
    if (crc != dev_param->crc)
    {
        LOG_E("pmt reset default");
        pmt_param_reset_default(dev_param);
        pmt_param_save(dev_param);
    }
}


static uint8_t pmt_send_command(uint8_t *buf, uint8_t len)
{
	uint8_t size = 0;
	
	if(pmt_dev)
	{
		size = rt_device_write(pmt_dev, 0, &buf[0], len);
	}
	
	return size;
}

void pmt_cmd_get_version(void)
{
    uint8_t cmd[3] = {0};

    cmd[0] = '?'; 
    cmd[1] = '\r';
    cmd[2] = '\n';

    pmt_send_command(cmd, 3);

}

void pmt_cmd_test_communicate(void)
{
	uint8_t cmd[3] = {0};

    cmd[0] = 'Q'; 
    cmd[1] = '\r';
    cmd[2] = '\n';

    pmt_send_command(cmd, 3);

}

void pmt_cmd_set_gating_time(uint8_t gating_time)
{
	uint8_t cmd[10] = {0};
	
    rt_snprintf((char *)&cmd[0], 10, "T%d\r\n", gating_time);
    pmt_send_command(cmd, rt_strlen((char*)cmd));

}

void pmt_cmd_set_resolution(uint16_t resol)
{
	uint8_t cmd[10] = {0};
	
    rt_snprintf((char *)&cmd[0], 10, "C%d\r\n", resol);
    pmt_send_command(cmd, rt_strlen((char*)cmd));

}


void pmt_cmd_get_counter_continue(void)
{
	uint8_t cmd[3] = {0};

    cmd[0] = 'S'; 
    cmd[1] = '\r';
    cmd[2] = '\n';

    pmt_send_command(cmd, 3);

}

void pmt_cmd_get_counter_once(void)
{
	uint8_t cmd[3] = {0};

    cmd[0] = 'J'; 
    cmd[1] = '\r';
    cmd[2] = '\n';

    pmt_send_command(cmd, 3);

}


void pmt_cmd_re_get_counter(void)
{
	uint8_t cmd[3] = {0};

    cmd[0] = 'R'; 
    cmd[1] = '\r';
    cmd[2] = '\n';

    pmt_send_command(cmd, 3);

}

void pmt_cmd_stop_counter(void)
{
	uint8_t cmd[3] = {0};

    cmd[0] = 'E'; 
    cmd[1] = '\r';
    cmd[2] = '\n';

    pmt_send_command(cmd, 3);

}


//uart recv callback
static uint16_t recv_bytes_number = 0;
static uint16_t is_byte_receiving = 0;
static rt_err_t pmt_dev_recv_callback(rt_device_t dev, rt_size_t size)
{
    //uart recv interrupt
    //rt_sem_release(&scanner_rx_sem);
	recv_bytes_number = size;
	is_byte_receiving = 10;
    return RT_EOK;
}

uint8_t pmt_dev_init(void)
{
	uint8_t cnt = 0;
	uint8_t res = 0;
	struct serial_configure cfg = RT_SERIAL_CONFIG_DEFAULT;
	
	
	//scanner initailze
	rt_kprintf("pmt dev ready...\n");
	cfg.baud_rate = PMT_DEV_BAUD;
	
	while (1) 
	{
		pmt_dev = rt_device_find(PMT_DEV_NAME);
		if (pmt_dev == RT_NULL)
		{
			rt_kprintf("find %s failed\n", PMT_DEV_NAME);
			rt_thread_delay(Msec(100));
		}
		else
		{
			rt_device_control(pmt_dev, RT_DEVICE_CTRL_CONFIG, &cfg);
			if(rt_device_open(pmt_dev, RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX) != RT_EOK)
			{
				rt_thread_delay(Msec(100));
				continue;
			}

			//set uart recv callback
			rt_device_set_rx_indicate(pmt_dev, pmt_dev_recv_callback);
			res = 1;
			break;
		}
		
		cnt++;
		if(cnt > 10)
		{
			res = 0;
			break;
		}
		
	}
	
	//set scanner 
	if(res)
	{
		rt_kprintf("pmt dev ready...\n");
	}
	else
	{
		rt_kprintf("pmt dev fail...\n");
	}
	
	return res;
	
}


void pmt_dev_control(uint8_t cmd, pmt_info_t *info)
{
	info->status.current_cmd = cmd;
	switch(cmd)
	{
		case PMT_CTR_CMD_get_version:
		{
			pmt_cmd_get_version();
			break;
		}
		
		case PMT_CTR_CMD_set_resolution:
		{
			pmt_cmd_set_resolution(info->param.resolution);
			break;
		}
		
		case PMT_CTR_CMD_set_gating_time:
		{
			pmt_cmd_set_gating_time(info->param.gating_time);
			break;
		}
		
		case PMT_CTR_CMD_test_comm:
		{
			pmt_cmd_test_communicate();
			break;
		}
		
		case PMT_CTR_CMD_start_continue:
		{
			pmt_cmd_get_counter_continue();
			break;
		}
		
		case PMT_CTR_CMD_start_once:
		{
			pmt_cmd_get_counter_once();
			break;
		}
		
		case PMT_CTR_CMD_re_start_once:
		{
			pmt_cmd_re_get_counter();
			break;
		}
		
		case PMT_CTR_CMD_stop_and_clean:
		{
			pmt_cmd_stop_counter();
			break;
		}

	}

}

uint16_t pmt_dev_get_data(uint8_t *buf, uint16_t maxlen)
{
	uint16_t size = 0;
	uint16_t recv_len = 0;
	
	if(is_byte_receiving)
	{
		is_byte_receiving--;
	}

	if( recv_bytes_number && (!is_byte_receiving) )
	{
		while (1)
		{
			//rt_sem_take(&scanner_rx_sem, RT_WAITING_FOREVER);
			size = rt_device_read(pmt_dev, 0, &buf[recv_len], 1);
			recv_len++;
			
			if(!size)
			{
				recv_bytes_number = 0;
				break;
			}
						
		}
		
		rt_kprintf("pmt recv %d bytes: [%s]\n", recv_len, buf);		
	

	
	}
	
	return recv_len;
   
}


void pmt_dev_recv_parse(pmt_info_t *info, uint8_t *buf, uint8_t len)
{
	if(len == 7 && buf[0] == 0x56) //version info
	{
		rt_memcpy(info->param.version, buf, len);
		info->status.current_cmd = PMT_CTR_CMD_get_version;
	}
	else if(len == 1 && buf[0] == 0x21) //communicate test
	{
		info->status.is_online = buf[0];
		info->status.current_cmd = PMT_CTR_CMD_test_comm;
	}
	else if(len == 3 && buf[0] == 0x54) //gating time
	{
		info->param.gating_time = *((uint16_t*)&buf[1]);
		info->status.current_cmd = PMT_CTR_CMD_set_gating_time;
	}
	else if(len == 2 && buf[0] == 0x43) //poulse resolution
	{
		info->param.resolution = buf[1];
		info->status.current_cmd = PMT_CTR_CMD_set_resolution;
	}
	else if(len == 6 && buf[0] == 0xff) //poulse count
	{
		info->status.pulse_count = *((uint16_t*)&buf[1]);
		info->status.current_cmd = PMT_CTR_CMD_start_once;
	}
	else if(len == 1 && buf[0] == 0x45) //stop and clean count
	{
		info->status.current_cmd = PMT_CTR_CMD_stop_and_clean;
	}

}


void pmt_dev_write_test(void)
{
	rt_device_write(pmt_dev, 0, "hello pmt write test", strlen( "hello scanner write test"));

}

MSH_CMD_EXPORT(pmt_dev_write_test, "pmt_dev_write_test");
