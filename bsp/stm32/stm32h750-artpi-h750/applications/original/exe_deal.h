#ifndef __EXE_DEAL_H__
#define __EXE_DEAL_H__

#include "rtthread.h"
#include "connect.h"
typedef struct ExeCmdName
{
    uint16_t cmd;
    char *name;
} ExeCmdName_t;

//执行命令可能存在的状态([0, 20]内为通用状态, (20, 255]为私有状态)
#define EXE_STATE_init 0
#define EXE_STATE_delet 1
#define EXE_STATE_done 2
#define EXE_STATE_can_start 21
#define EXE_STATE_start 22
#define EXE_STATE_timeout 23

#define EXE_SON_DEAL_FLAG_some_failed 0x01
#define EXE_SON_DEAL_FLAG_some_timeout 0x02
#define EXE_SON_DEAL_FLAG_running 0x04
#define EXE_SON_DEAL_FLAG_some_succeed 0x08

#pragma pack(1)
struct Exe_cls_t;
typedef struct Exe_cls_t
{
    Connect_t connect;
    uint8_t state;
    uint8_t last_state;
    uint32_t start_tick;
    uint32_t timeout_tick;
    uint8_t step;                  //自身的步骤
    uint8_t flag : 2;              //用于表示此命令与其它同级命令的处理关系。
    uint8_t is_force_stop : 2;     //用于表示是否为强制停止
    uint8_t is_err_deal : 2;       //用于表示是否已经进行了错误处理
    uint8_t is_take_res_lock : 1;  //占有资源
    uint8_t is_unrelease_lock : 1; //出错时不释放资源
    uint32_t res_lock;             //所需占有的资源
    uint16_t cmd;                  //用来标识执行体的功能。
    //子命令控制步骤
    uint8_t s_current_step;
    uint8_t s_next_step;
    uint8_t s_max_step;
    uint32_t err;
    uint8_t cnt;
    void *data;
    uint32_t temp_para; //可以试情况而作用。
    void (*state_change_cb)(struct Exe_cls_t *e, void *data);
    void (*state_deal)(struct Exe_cls_t *e, void *data);
    uint32_t (*done)(struct Exe_cls_t *e, uint32_t err);
    //错误处理
    uint32_t (*error_deal)(struct Exe_cls_t *e, void *flag);
    //收尾工作
    uint32_t (*tail_in_work_cb)(struct Exe_cls_t *e, void *data);
} Exe_cls_t;
#pragma pack()

void exe_init(void);
Exe_cls_t *exe_create(void);
void exe_delete(Exe_cls_t *e);
uint8_t exe_add2line(Exe_cls_t *e);
uint8_t exe_rm_form_line(Exe_cls_t *e);
uint8_t exe_inser2line_after(Exe_cls_t *e, Exe_cls_t *add);
uint8_t exe_transfer_lock(Exe_cls_t *from, Exe_cls_t *to, uint32_t lock);
void exe_deal_thread(void *p);
void exe_change_state(Exe_cls_t *e, uint8_t new_state);
void exe_all_stop(void);
void exe_all_son_stop(Exe_cls_t *e);
uint8_t exe_search(Exe_cls_t *e);
void exe_stop(Exe_cls_t *e);

#define EXE_CHANGE_EVENT_FLAG_change 1
extern struct rt_event exe_change_event;
#endif
