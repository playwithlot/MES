/**
 * @file test.c
 * @author lzj (lzj..@com)
 * @brief 测试用文件。
 * @version 0.1
 * @date 2021-07-27
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "rtthread.h"
#include "rtdevice.h"
#include "drv_common.h"
#include "drv_gpio.h"
#include "iot_parameter.h"
#include "dev_flash.h"
#include "monitor.h"



#define LOG_TAG "test"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

#define LED0_PIN_NUM GET_PIN(E, 3)
#define LED1_PIN_NUM GET_PIN(F, 1)
#define LED2_PIN_NUM GET_PIN(F, 2)
#define LED3_PIN_NUM GET_PIN(F, 3)



static struct rt_thread monitor_hand;

extern void phy_linkchange(void);
void monitor_thread(void *p)
{
    rt_base_t led_level = PIN_LOW;
    rt_tick_t led_tick = 0;
    rt_tick_t phy_link_tick = 0;
    rt_pin_mode(LED0_PIN_NUM, PIN_MODE_OUTPUT);
    rt_pin_write(LED0_PIN_NUM, PIN_HIGH);
    rt_pin_mode(LED1_PIN_NUM, PIN_MODE_OUTPUT);
    rt_pin_write(LED1_PIN_NUM, PIN_HIGH);
    rt_pin_mode(LED2_PIN_NUM, PIN_MODE_OUTPUT);
    rt_pin_write(LED2_PIN_NUM, PIN_HIGH);
    rt_pin_mode(LED3_PIN_NUM, PIN_MODE_OUTPUT);
    rt_pin_write(LED3_PIN_NUM, PIN_HIGH);

    rt_thread_delay(Msec(500));
    while (1)
    {
        if (Msec(200) < rt_tick_get() - led_tick)
        {
            rt_pin_write(LED0_PIN_NUM, led_level);
            rt_pin_write(LED1_PIN_NUM, !led_level);
            rt_pin_write(LED2_PIN_NUM, led_level);
            rt_pin_write(LED3_PIN_NUM, !led_level);
            led_level = !led_level;
            led_tick = rt_tick_get();
        }
        if (iot_para.status.is_need_reboot) //需要重启
        {
            dev_flash_take(); //占用flash，防止其它线程再操作flash。
            LOG_E("system will reboot after 3 second!!!");
            rt_thread_delay(Sec(3));
            rt_hw_cpu_reset();
        }
        if (Msec(1000) < rt_tick_get() - phy_link_tick)
        {
            phy_linkchange();//检查网络phy状态
            phy_link_tick = rt_tick_get();
        }
        rt_thread_delay(Msec(20));
    }
}




void create_monitor_thread(void)
{
	rt_err_t err;

	err = rt_thread_init(&monitor_hand, "monitor", monitor_thread, RT_NULL,
											 (void *)MONITOR_THREAD_STACK_ADDR,
											 MONITOR_THREAD_STACK_SIZE,
											 MONITOR_THREAD_PRIORITY, 
											 MONITOR_THREAD_TIME_SLINCE);
	if (err == RT_EOK)
	{
			rt_thread_startup(&monitor_hand);
	}


}



