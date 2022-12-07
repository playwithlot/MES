#ifndef __DEV_FLASH_H__
#define __DEV_FLASH_H__

#include "rtthread.h"

//��ַ���壬ע��������ص�(����Ŀǰ��λ����Ҫ��ŵĲ������٣����Խ�1�����������һ�����)
#define SPI_FLASH_START_ADDR 0

//��λ������Ļ���������ip��name�ȣ�
#define SPI_FLASH_IOT_PARA_ADDR SPI_FLASH_START_ADDR
#define SPI_FLASH_IOT_PARA_SIZE 4096

// PMT��������
#define SPI_FLASH_PMT_PARA_ADDR (SPI_FLASH_IOT_PARA_ADDR + SPI_FLASH_IOT_PARA_SIZE)
#define SPI_FLASH_PMT_PARA_SIZE 4096

// ��λ��ʹ�õĻ�������
#define SPI_FLASH_LM_PARA_ADDR (SPI_FLASH_PMT_PARA_ADDR + SPI_FLASH_PMT_PARA_SIZE )
#define SPI_FLASH_LM_PARA_SIZE 4096

int dev_flash_init(void);
uint32_t dev_flash_read(uint32_t pos, void *buf, uint32_t size);
uint32_t dev_flash_write(uint32_t pos, void *buf, uint32_t size);
void dev_flash_take(void);

#endif
