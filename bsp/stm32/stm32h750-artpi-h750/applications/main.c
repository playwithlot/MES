/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 * 2019-10-19     xuzhuoyi     add stm32f429-st-disco bsp
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "common.h"
#include "dev_flash.h"
#include "app.h"



#define LOG_TAG "main"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>


int main(void)
{
    uint32_t use;
    use = CCM_RAM_USE_END - CCM_RAM_BGING;
    LOG_I("main thread start running.");
    LOG_I("cmm ram use size:%d", use);
		
		app_initailze();
	
    return RT_EOK;
}

