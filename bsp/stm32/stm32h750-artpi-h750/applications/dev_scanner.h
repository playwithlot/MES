#ifndef __SCANNER_H__
#define __SCANNER_H__

#include "rtthread.h"
#include "rtthread.h"
#include "rtdevice.h"
#include "drv_gpio.h"
#include "common.h"

/******************************************************
searia:BAUD RATE=9600, check= 8,odd= N, stop= 1.

get version:
02 56 0D 0A

reset to fac setting:
02 50 43 32 30 0D 0A
02 50 43 32 30 0D 0A

start continue scan cmd:
02 50 54 30 33 32 30 33 34 30 31 0D 0A

stop continue scan cmd:
02 50 54 30 33 32 30 33 34 30 30 0D 0A

start scan once:
02 2B 0D 0A

stop scan noce:
02 2D 0D 0A

enable read sucessful probability check:
02 4A 50 2B 0D 0A

disable read sucessful probability check:
02 4A 50 2D 0D 0A

mask repeat code: 
02 50 54 30 30 32 30 38 36 41 30 0D 0A

dont mask repeat code:
02 50 54 30 30 32 30 38 36 30 30 0D 0A
***************************************************/

//scanner gpio init
#define SCANNER_TRIG_PIN GET_PIN(B, 5)
#define SCANNER_PWR_PIN GET_PIN(B, 6)

//scanner io port param
#define RX_BUF_SIZE 256
#define SCANNER_DEV_NAME "uart6" 
#define SCANNER_DEV_BAUD BAUD_RATE_115200




typedef enum
{
	//ɨ��ǹ����
	SCANNER_TYPE_cognex = 0,     //������
	SCANNER_TYPE_keyence = 1,    //����ʿ
	SCANNER_TYPE_leuze = 2,      //�¹����ײ�
	SCANNER_TYPE_leuze_CR55 = 3, //�¹����ײ�
	SCANNER_TYPE_leuze_BCL95 = 4, //�¹����ײ�

} SCANNER_TYPEDS_DEFINE;


#define USER_SCANNER_TYPES SCANNER_TYPE_cognex


typedef enum
{
	//ɨ��ǹ����ģʽ
	SCANNER_MODE_leuze_once = 0,     //����ģʽ(����ɨ�裬ɨ����ֹͣ)
	SCANNER_MODE_leuze_continue      //����ģʽ������ɨ�裬ɨ���󷵻��룬Ȼ�����ɨ��

} SCANNER_WORK_TYPES;


typedef enum
{
	SCANNER_CMD_start = 0x01,
	SCANNER_CMD_stop = 0x02,
	SCANNER_CMD_pwr_on = 0x03,
	SCANNER_CMD_pwr_off = 0x04,
	SCANNER_CMD_start_10 = 0x05

} SCANNER_RECV_CMD_TYPES;



extern uint8_t scanner_dev_init(uint8_t types);
extern void scanner_dev_ctrl(uint8_t cmd, uint8_t type, uint8_t mode);
extern uint8_t scanner_dev_get_data(uint8_t *buf, uint8_t len, rt_tick_t timeout);
extern uint8_t scanner_dev_clean_stream(void);

#endif
