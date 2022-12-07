#ifndef __RFID_READER_H__
#define __RFID_READER_H__

#include "rtthread.h"
#include "common.h"

#pragma pack(1)
typedef struct
{
	uint8_t recv_buf[128];
	uint8_t recv_len;
	uint8_t send_cmd;
	uint8_t status;
	
} rfid_dev_t;


typedef enum
{
	RFID_DEV_STATUS_IDLE = 0x00,
	RFID_DEV_STATUS_INIT,
	RFID_DEV_STATUS_BUSY,
	RFID_DEV_STATUS_DONE,
	RFID_DEV_STATUS_ERR,
	
} RFID_DEV_STATUS_TYPES;


typedef enum
{
	YZ_CMD_none = 0x00,
	YZ_CMD_get_hardversion = 0x16,
	YZ_CMD_scan_card = 0x46,
	YZ_CMD_anticoll = 0x47,
	YZ_CMD_select_card = 0x48,
	YZ_CMD_vef_keyA = 0x4a,
	YZ_CMD_read_3_block = 0x22,
	YZ_CMD_read_1_block = 0x4B,

} RFID_DEV_COMMAND_TYPES;




extern rfid_dev_t m_dev;

uint8_t rfid_dev_init(rfid_dev_t *dev);
uint16_t rfid_dev_get_data(uint8_t *buf, uint16_t maxlen);
void rfid_dev_control(rfid_dev_t *dev, uint8_t cmd, uint8_t *param);

#pragma pack()





#endif
