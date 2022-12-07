
#ifndef __CONNECT_H__
#define __CONNECT_H__

#include "rtthread.h"
#include "stdint.h"
#include "line.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//类类型(尽量取一些不常用的数字)
#define CLASS_TYPE_UNKNOWN 0
#define CLASS_TYPE_connect 83
#define CLASS_TYPE_tcp_pack 85
#define CLASS_TYPE_net_pack 87
#define CLASS_TYPE_exe 89
#define CLASS_TYPE_can_pro 91
#define CLASS_TYPE_scanner_ctr 95
#define CLASS_TYPE_pmt_ctr 97

#define CLASS_MEM_TYPE_static 0
#define CLASS_MEM_TYPE_malloc 1

#define CALL_BACK_CAUSE_init 0
#define CALL_BACK_CAUSE_create 1
#define CALL_BACK_CAUSE_del 2
#define CALL_BACK_CAUSE_connect_parent 3 //连接时本身被当为父连接
#define CALL_BACK_CAUSE_connect_son 4    //连接时本身被当为子连接

#pragma pack(1)
//基础类
typedef struct Obj_class_t
{
    uint8_t type:7; //类类型
    uint8_t mem_type:1; //此结构体变量自己本身的内存类型
}Obj_class_t;

struct Connect;
typedef struct Connect
{
    //关连类，用来将相关的内容连接起来，
    Obj_class_t ob;
    line p_line; //指向上层的连接(目前的处理默认只有一个上层！)
    line s_line; //指向下层的连接
} Connect_t;
#pragma pack()

void connect_init(void);
void con_init(Connect_t *con);
void con_del(Connect_t *con);
Connect_t *con_get_parent(Connect_t *con);
uint8_t con_create_connect(Connect_t *p, Connect_t *s);
void con_del_connect(Connect_t *con, uint8_t up_or_down);

#endif
