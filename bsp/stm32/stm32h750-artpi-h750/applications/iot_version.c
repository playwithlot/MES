
#include "iot_version.h"
#include "rtthread.h"
#include "stm32h750xx.h"

//此文件应该设置为每次均编译
const unsigned char compile_date[sizeof(__DATE__) + 1] = __DATE__;
const unsigned char compile_time[sizeof(__TIME__) + 1] = __TIME__;

static unsigned char uid[12] = {0};

//输出版本等基本信息。
void show_compile_time(void)
{
    rt_memcpy(&uid[0], (rt_uint8_t *)UID_BASE, 12);
    rt_kprintf("compile time:%s, %s\r\n", compile_date, compile_time);
    rt_kprintf("project name: %s\r\n", PRJ_NMAE);
    rt_kprintf("hardware borde name: %s\r\n", HARD_VERSION);
    rt_kprintf("software version: V%d.%d.%d.%d\r\n", SOFT_VERSION_A, SOFT_VERSION_B, SOFT_VERSION_C, SOFT_VERSION_D);
    rt_kprintf("hardware version: V%d.%d.%d.%d\r\n", HARD_VERSION_A, HARD_VERSION_B, HARD_VERSION_C, HARD_VERSION_D);
    rt_kprintf("id code: V%d.%d.%d.%d\r\n", ID_CODE_A, ID_CODE_B, ID_CODE_C, ID_CODE_D);
    rt_kprintf("mcu uid(h):%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x\r\n", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5],
               uid[6], uid[7], uid[8], uid[9], uid[10], uid[11]);
    rt_kprintf("tick per second is:%d\r\n", RT_TICK_PER_SECOND);
    rt_kprintf("now system tick:%d, seconds:%d\r\n", rt_tick_get(), rt_tick_get() / RT_TICK_PER_SECOND);
    rt_kprintf("Commands entered with care may cause unrecoverable failures!!!\r\n\r\n");
}
