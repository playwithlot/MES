#ifndef __CRC16_H__
#define __CRC16_H__

#include "rtthread.h"

uint16_t CRC16_Verify(uint16_t InitValue, uint8_t *ptr, uint32_t len);
uint16_t CRC16_modbus(uint8_t *ptr, uint32_t len);
uint8_t crc8(uint8_t *ptr, uint32_t len);

#endif


