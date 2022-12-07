#ifndef __CYC_CMD_HANDLE_H
#define __CYC_CMD_HANDLE_H


#include "rtthread.h"
#include "common.h"
#include "dev_can.h"
#include "dev_scanner.h"
#include "local_handle.h"



#define IS_POS_EMPTY     (1)
#define IS_POS_NOT_EMPTY (0)
#define IS_SMP_DONE      (1)
#define IS_SMP_NOT_DONE  (0)

#define SMP_SHAPE_CODE_MAX_SIZE   (20)

#define CACHE_TYPES_EMER          (1)
#define CACHE_TYPES_BASKET        (2)
#define CACHE_TYPES_CACHE_STORE   (3)



#define BASKET_NUM                  (3)
#define CACHE_NUM_EMER_STORE        (2)
#define CACHE_NUM_BASKET_SLOT       (13)
#define CACHE_NUM_CACHE_STORE       (10)

#define CACHE_POS_EMER_STORE_BASE   (0)
#define CACHE_POS_BASKET_BASE       (CACHE_POS_EMER_STORE_BASE+CACHE_NUM_EMER_STORE)
#define CACHE_POS_CACHE_STORE_BASE  (CACHE_POS_BASKET_BASE+(CACHE_NUM_BASKET_SLOT*BASKET_NUM))
#define CACHE_POS_MIN_CAR           (CACHE_POS_CACHE_STORE_BASE+CACHE_NUM_CACHE_STORE)
#define CACHE_POS_EMER_TR_CACHE     (CACHE_POS_MIN_CAR+1)
#define CACHE_POS_EMER_ADD_SMP      (CACHE_POS_MIN_CAR+2)
#define CACHE_POS_EMER_RECYC        (CACHE_POS_MIN_CAR+3)
#define CACHE_POS_NORMAL_TR_CACHE   (CACHE_POS_MIN_CAR+4) //37  55
#define CACHE_POS_NORMAL_ADD_SMP    (CACHE_POS_MIN_CAR+5)
#define CACHE_POS_NORMAL_RECYC      (CACHE_POS_MIN_CAR+6)
#define CACHE_POS_NUM               (CACHE_POS_MIN_CAR+7)





#define DEST_TRACK_TYPES_EMER    (0x01)
#define DEST_TRACK_TYPES_NORMAL  (0x02)
#define DEST_TRACK_TYPES_RECYC   (0x03)

#define DEST_TRACK_ADD_POS        (0x01)
#define DEST_TRACK_ADD_CACHE_POS  (0x02)
#define DEST_TRACK_RECYC_POS      (0x03)

#define BASKET_TYPES_EMER         (0x01)
#define BASKET_TYPES_NORMAL       (0x02)


#define LED_COLOR_OFF   (0)
#define LED_COLOR_G     (1)
#define LED_COLOR_Y     (2)
#define LED_COLOR_R     (3)
#define LED_COLOR_BLK   (4)

typedef struct basket
{
	uint8_t basket_pos;
	uint8_t slot_pos;
	uint8_t is_done;
	uint8_t len;  //shape code len
	uint8_t shape_code[SMP_SHAPE_CODE_MAX_SIZE];
	
} smp_shelf_t;

typedef struct cache_region
{
	uint8_t is_empty;
	smp_shelf_t smp;

} cache_region_t;


//typedef enum
//{
//	CYC_CMD_GET_VERSION = 0x0104,
//	CYC_CMD_CHECK_BASKET = 0x2904,
//	CYC_CMD_PUSH_CACHE_REGION = 0x4204,


//} MODULE2_CMD_TYPES;


void basket_sample_scan_handle(uint8_t basket_num);







int get_is_exsit_the_same_smp(smp_shelf_t *dest_smp, uint8_t skip_pos);
int get_basket_sample(smp_shelf_t *dest_smp, uint8_t basket_num);
int get_emer_store_sample(smp_shelf_t *dest_smp, uint8_t store_num);
int get_is_exsit_smp_of_cache_pos(uint8_t pos);
int get_is_smp_in_place(smp_shelf_t *sample, uint8_t pos);
int get_smp_cache_pos(smp_shelf_t *sample);
int pop_cache_pos(uint8_t src_pos, uint8_t dest_pos);
int push_cache_pos(uint8_t src_pos, uint8_t cache_types, uint8_t done);

#if 0
pop_cache_pos(CACHE_POS_MIN_CAR, CACHE_POS_NORMAL_TR_CACHE);
pop_cache_pos(CACHE_POS_MIN_CAR, CACHE_POS_EMER_TR_CACHE);
pop_cache_pos(CACHE_POS_MIN_CAR, CACHE_POS_EMER_TR_CACHE);
pop_cache_pos(src_pos, CACHE_POS_MIN_CAR);
pop_cache_pos(CACHE_POS_MIN_CAR, CACHE_POS_NORMAL_TR_CACHE);
pop_cache_pos(src_pos, CACHE_POS_MIN_CAR);

pop_cache_pos(src_pos, CACHE_POS_NORMAL_ADD_SMP);
pop_cache_pos(src_pos, CACHE_POS_NORMAL_ADD_SMP);
pop_cache_pos(src_pos, CACHE_POS_EMER_ADD_SMP);
pop_cache_pos(src_pos, CACHE_POS_EMER_ADD_SMP);

pop_cache_pos(src_pos, CACHE_POS_NORMAL_RECYC);
			pop_cache_pos(src_pos, CACHE_POS_NORMAL_RECYC);
				pop_cache_pos(src_pos, CACHE_POS_NORMAL_RECYC);
				pop_cache_pos(src_pos, CACHE_POS_EMER_RECYC);
			pop_cache_pos(src_pos, CACHE_POS_EMER_RECYC);
			pop_cache_pos(src_pos, CACHE_POS_EMER_RECYC);

#endif


#endif 



