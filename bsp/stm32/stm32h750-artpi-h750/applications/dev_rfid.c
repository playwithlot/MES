#include "rtthread.h"
#include "rtdevice.h"
#include "dev_rfid.h"


#define LOG_TAG "rfid"
#define LOG_LVL LOG_LVL_INFO
#include <ulog.h>


#define RFID_DEV_NAME "uart2" 
#define RFID_DEV_BAUD BAUD_RATE_19200
static rt_device_t rfid_dev = RT_NULL; 	
static uint8_t command_frame_buf[20] = {0};
rfid_dev_t m_dev = {0};


static uint8_t _cal_check_sum(uint8_t *buf, uint8_t len)
{
    uint8_t vef = 0;
    uint8_t i;

    for (i = 0; i < len; i++)
    {
        vef += buf[i];
    }
    return vef;
}

/***********************************************************************
rfid command frame: head + addr[2] + len + cmd + data[n] + check_sum
len: len + cmd + data + check_sum
param buf: len + cmd + data[n]
************************************************************************/
static uint8_t rfid_send_command(uint8_t *buf, uint8_t len)
{
	uint8_t size = 0;
	uint8_t cmd_frame_len = 0;

	command_frame_buf[0] = 0x02;  //head
	command_frame_buf[1] = 0x00;  //addr1
	command_frame_buf[2] = 0x00;  //addr2
	
	cmd_frame_len = 3;
	for (uint8_t i = 0; i < len; i++)
	{
		if (buf[i] == 0x02 || buf[i] == 0x10 || buf[i] == 0x03)
		{
			command_frame_buf[cmd_frame_len] = 0x10;
			cmd_frame_len++;
		}
		
		command_frame_buf[cmd_frame_len] = buf[i];
		cmd_frame_len++;
		
	}
	

	command_frame_buf[cmd_frame_len] = _cal_check_sum(buf, len);  //check_sum
	cmd_frame_len++;
	command_frame_buf[cmd_frame_len] = 0x03;


	
	if(rfid_dev)
	{
		size = rt_device_write(rfid_dev, 0, &command_frame_buf[0], cmd_frame_len);
	}
	
	return size;
}



//uart recv callback
static uint16_t recv_bytes_number = 0;
static uint16_t is_byte_receiving = 0;
static rt_err_t rfid_dev_recv_callback(rt_device_t dev, rt_size_t size)
{
    //uart recv interrupt
    //rt_sem_release(&scanner_rx_sem);
	recv_bytes_number = size;
	is_byte_receiving = 10;
    return RT_EOK;
}

uint8_t rfid_dev_init(rfid_dev_t *dev)
{
	struct serial_configure cfg = RT_SERIAL_CONFIG_DEFAULT;
	
	//scanner initailze
	rt_kprintf("rfid dev init...\n");
	cfg.baud_rate = RFID_DEV_BAUD;
	dev->status = RFID_DEV_STATUS_INIT;
	
	rfid_dev = rt_device_find(RFID_DEV_NAME);
	
	if (rfid_dev == RT_NULL)
	{
		rt_kprintf("find %s failed!\n", RFID_DEV_BAUD);
		return 0;
	}
	
	rt_device_control(rfid_dev, RT_DEVICE_CTRL_CONFIG, &cfg);
	if(rt_device_open(rfid_dev, RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX) != RT_EOK)
	{
		rt_kprintf("rfid dev open failed!\n");
		return 0;
	}
	
	//set uart recv callback
	rt_device_set_rx_indicate(rfid_dev, rfid_dev_recv_callback);
	rt_kprintf("rfid dev ready...\n");
	dev->status = RFID_DEV_STATUS_IDLE;
	
	return 1;
	
}


uint16_t rfid_dev_get_data(uint8_t *buf, uint16_t maxlen)
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
			size = rt_device_read(rfid_dev, 0, &buf[recv_len], 1);
			recv_len++;
			
			if(!size)
			{
				recv_bytes_number = 0;
				break;
			}
						
		}
		
		rt_kprintf("rfid recv %d bytes: [%s]\n", recv_len, buf);		
	

	
	}
	
	return recv_len;
   
}


//param: keya (A:0x60 ~ B:0x61)
static void rfid_dev_vef_keya(uint8_t block, uint8_t keya)
{
	uint8_t cmd_buf[] = {0, 0, 0x0b, YZ_CMD_vef_keyA, keya, block, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    block %= 64;
	
    rfid_send_command((uint8_t*)cmd_buf, sizeof(cmd_buf));
}


void rfid_dev_control(rfid_dev_t *dev, uint8_t cmd, uint8_t *param)
{
	dev->send_cmd = cmd;
	switch(cmd)
	{
		case YZ_CMD_get_hardversion:
		{
			const uint8_t cmd_buf[] = {0x03, YZ_CMD_get_hardversion};
			rfid_send_command((uint8_t*)cmd_buf, sizeof(cmd_buf));
			break;
		}
		
		case YZ_CMD_scan_card:
		{
			const uint8_t cmd_buf[] = {0x04, YZ_CMD_scan_card, 0x52};
			rfid_send_command((uint8_t*)cmd_buf, sizeof(cmd_buf));
			break;
		}
		
		case YZ_CMD_anticoll:
		{
			const uint8_t cmd_buf[] = {0x04, YZ_CMD_anticoll, 0x04};
			rfid_send_command((uint8_t*)cmd_buf, sizeof(cmd_buf));
			break;
		}
		
		case YZ_CMD_select_card:
		{
			const uint8_t cmd_buf[] = {0x07, YZ_CMD_select_card, 0, 0, 0, 0};
			rfid_send_command((uint8_t*)cmd_buf, sizeof(cmd_buf));
			break;
		}
		
		case YZ_CMD_vef_keyA:
		{
			rfid_dev_vef_keya(param[0], param[1]);
			break;
		}
		
		
		case YZ_CMD_read_1_block:
		{
			//uint8_t buf[] = {0, 0, 0x04, YZ_CMD_read_1_block, start_block};
			const uint8_t cmd_buf[] = {0x04, YZ_CMD_read_1_block, param[0]};
			rfid_send_command((uint8_t*)cmd_buf, sizeof(cmd_buf));
			break;
		}
		
		case YZ_CMD_read_3_block:
		{
			//uint8_t buf[] = {0, 0, 0x04, YZ_CMD_read_3_block, start_block};
			const uint8_t cmd_buf[] = {0x04, YZ_CMD_read_3_block, param[0]};
			rfid_send_command((uint8_t*)cmd_buf, sizeof(cmd_buf));
			break;
		}

	}

}

void rfid_dev_start(rfid_dev_t *dev)
{
	uint8_t param[2] = {0};

	if(dev->status == RFID_DEV_STATUS_IDLE)
	{
		dev->status = RFID_DEV_STATUS_BUSY;
		rfid_dev_control(dev, YZ_CMD_scan_card, RT_NULL);
		
		rt_thread_delay(1);
		rfid_dev_control(dev, YZ_CMD_anticoll, RT_NULL);
		
		rt_thread_delay(1);
		rfid_dev_control(dev, YZ_CMD_select_card, RT_NULL);
		
		param[0] = 1;    //block 1
		param[1] = 0x60; //kera A
		rt_thread_delay(1);
		rfid_dev_control(dev, YZ_CMD_vef_keyA, param);
		
		param[0] = 1;
		rt_thread_delay(1);
		rfid_dev_control(dev, YZ_CMD_read_1_block, param);
	
	}
	else
	{
		rt_kprintf("rfid dev operate err! %d\n", dev->status);
	}
	
	

}
	

uint8_t rfid_get_frame(rfid_dev_t *dev, uint8_t *buf, uint16_t len)
{
	uint8_t index = 0;
	
	index = 1;
	dev->recv_buf[0] = buf[0];
	for(uint8_t i = 1; i < len-1; i++)
	{
		if(buf[i] != 0x10)
		{
			dev->recv_buf[index] = buf[i];
			index++;
		}	
	}
	
	dev->recv_buf[index++] = buf[len-1];

}



uint8_t rfid_recv_parse(rfid_dev_t *dev)
{
	uint8_t recv_len = 0;
	uint8_t recv_buf[20] = {0};
	
	if( dev->status == RFID_DEV_STATUS_BUSY && 
		(dev->send_cmd == YZ_CMD_read_1_block || 
	     dev->send_cmd == YZ_CMD_read_3_block) )
	{
		recv_len = rfid_dev_get_data(recv_buf, 128);
		rfid_get_frame(dev, recv_buf, recv_len);
		
		
	
	}
	else if( dev->status == RFID_DEV_STATUS_BUSY && 
			 dev->send_cmd == YZ_CMD_get_hardversion)
	{
		recv_len = rfid_dev_get_data(recv_buf, 128);
		rfid_get_frame(dev, recv_buf, recv_len);
	
	}

}



