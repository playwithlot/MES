#ifndef __MES_FLOWS_H
#define __MES_FLOWS_H


#include "rtthread.h"
#include "common.h"
#include "dev_can.h"
#include "dev_scanner.h"

#define MES85_BASKET_NUM 3
#define MES_CACHE_REGION_NUM 10 
#define MES85_SAMPLE_NUM 10
#define MES85_SHAPE_CODE_MAXLEN 20 



typedef struct basket
{
	uint8_t basket_pos;
	uint8_t slot_pos;
	uint8_t shape_code[3];
	
} sample_shelf_t;

typedef struct cache_region
{
	uint8_t locate_pos;
	uint8_t is_empty;
	uint8_t shape_code[3];

} cache_region_t;

typedef enum
{
	MODULE_CMD_GET_VERSION = 0x0104,
	MODULE_CMD_CHECK_BASKET = 0x4204,
	MODULE_CMD_PUSH_CACHE_REGION = 0x4204,


} MODULE2_CMD_TYPES;



#endif 



