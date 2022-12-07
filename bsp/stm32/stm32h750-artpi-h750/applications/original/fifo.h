 
#ifndef _FIFO_H_
#define _FIFO_H_

#include "rtthread.h" 

#pragma pack(1)
/**
 * @brief 队列结构体
 * 
 */
 typedef struct
{
    uint16_t ptr_write;
    uint16_t ptr_read;
    uint8_t is_full;
    uint8_t is_cover;//是否可覆盖写（即队列满时，读出1个数据，然后再写入新数据）
    // fifo中存储的元素数,不是字节大小
    uint16_t item_sum;
    // 元素大小.单位: 字节
    uint16_t item_size;
    void *fifo_ptr;
} Fifo;
#pragma pack()

void fifo_init(Fifo *fifo, uint8_t is_cover, uint8_t *buf, uint16_t item_sum, uint16_t item_size);
void fifo_clear(Fifo *fifo);
uint8_t fifo_writeable(Fifo *fifo);
uint8_t fifo_writeable(Fifo *fifo);
uint8_t fifo_write(Fifo *fifo, void *data);
uint8_t fifo_write_batch(Fifo *fifo, void *data, uint16_t item_num);
uint8_t fifo_readable(Fifo *fifo);
 
uint8_t fifo_read(Fifo *fifo, void *data);
uint8_t fifo_read_batch(Fifo *fifo, void *data, uint16_t item_num);
uint16_t fifo_readable_item_count(Fifo *fifo);
uint16_t fifo_writeable_item_count(Fifo *fifo);
 
#endif
 
 
