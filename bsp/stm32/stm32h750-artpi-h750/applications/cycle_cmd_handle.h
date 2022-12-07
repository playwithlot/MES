#ifndef __CYCLE_CMD_HANDLE_H
#define __CYCLE_CMD_HANDLE_H


#include "rtthread.h"
#include "common.h"
#include "dev_can.h"
#include "dev_scanner.h"

#define MES_BASKET_NUM 3




#define MES_CACHE_STORE_NUM      (10)
#define MES_NIM_CACHE_STORE_POS  (0)
#define MES_MIN_CAR_CACHE_POS    (MES_CACHE_STORE_NUM)
#define MES_TRACK_CACHE_POS      (MES_CACHE_STORE_NUM+1)
#define MES_TRACK_ADD_SMP_POS    (MES_CACHE_STORE_NUM+2)
#define MES_CACHE_REGION_NUM     (MES_CACHE_STORE_NUM+3)

#define MES_SAMPLE_NUM 10
#define MES_SHAPE_CODE_MAXLEN 20 

#define MES_WAITING_MAX  30000


typedef struct basket
{
	uint8_t basket_pos;
	uint8_t slot_pos;
	uint8_t is_done;
	uint8_t shape_code[3];
	
} sample_shelf_t;

typedef struct cache_region
{
	uint8_t locate_pos;
	uint8_t is_empty;
	sample_shelf_t sample;

} cache_region_t;

typedef enum
{
	MODULE_CMD_GET_VERSION = 0x0104,
	MODULE_CMD_CHECK_BASKET = 0x4204,
	MODULE_CMD_PUSH_CACHE_REGION = 0x4204,


} MODULE2_CMD_TYPES;

//extern void scan_sample_handle(void);

#endif 



