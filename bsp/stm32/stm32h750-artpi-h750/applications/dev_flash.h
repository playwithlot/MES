#ifndef __DEV_FLASH_H__
#define __DEV_FLASH_H__

#include "rtthread.h"

//地址定义，注意勿出现重叠(由于目前中位机需要存放的参数较少，所以将1个扇区分配给一类参数)
#define SPI_FLASH_START_ADDR 0

//中位机自身的基本参数（ip，name等）
#define SPI_FLASH_IOT_PARA_ADDR SPI_FLASH_START_ADDR
#define SPI_FLASH_IOT_PARA_SIZE 4096

// PMT基本参数
#define SPI_FLASH_PMT_PARA_ADDR (SPI_FLASH_IOT_PARA_ADDR + SPI_FLASH_IOT_PARA_SIZE)
#define SPI_FLASH_PMT_PARA_SIZE 4096

// 下位机使用的基本参数
#define SPI_FLASH_LM_PARA_ADDR (SPI_FLASH_PMT_PARA_ADDR + SPI_FLASH_PMT_PARA_SIZE )
#define SPI_FLASH_LM_PARA_SIZE 4096

int dev_flash_init(void);
uint32_t dev_flash_read(uint32_t pos, void *buf, uint32_t size);
uint32_t dev_flash_write(uint32_t pos, void *buf, uint32_t size);
void dev_flash_take(void);

#endif
