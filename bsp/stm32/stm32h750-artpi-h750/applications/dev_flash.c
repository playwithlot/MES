/**
 * @file dev_flash.c
 * @author King (lzj..@com)
 * @brief 外部spi flash操作接口转换，线程安全。
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "rtthread.h"

#ifdef USER_USING_SPI_FLASH

#include "dev_flash.h"
#include "rtdevice.h"
#include "iot_parameter.h"

#define LOG_TAG "dev_flash"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

static rt_device_t dev_spi_flash = RT_NULL;
struct rt_mutex dev_spi_flash_mutex;

int dev_flash_init(void)
{
    rt_err_t err;
    err = rt_mutex_init(&dev_spi_flash_mutex, "flash_ops", RT_IPC_FLAG_PRIO);
    if (err != RT_EOK)
    {
        LOG_E("init flash mutex failed");
    }
    //在其它地方已经把spi flash挂载到了对应的总线上。
    dev_spi_flash = rt_device_find(SPI_FLASH_NAME);
    if (dev_spi_flash == RT_NULL)
    {
        LOG_E("find %s failed", SPI_FLASH_NAME);
        return 1;
    }
    if (rt_device_open(dev_spi_flash, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        LOG_E("open %s failed", SPI_FLASH_NAME);
        return 1;
    }
    return 0;
}
INIT_ENV_EXPORT(dev_flash_init);

//主要用于系统重启时，确保写flash已经完成。
void dev_flash_take(void)
{
    rt_mutex_take(&dev_spi_flash_mutex, RT_WAITING_FOREVER);
}

uint32_t dev_flash_read(uint32_t pos, void *buf, uint32_t size)
{
    uint32_t len;
    rt_mutex_take(&dev_spi_flash_mutex, RT_WAITING_FOREVER);
    len = rt_device_read(dev_spi_flash, pos, buf, size);
    rt_mutex_release(&dev_spi_flash_mutex);
    return len;
}

uint32_t dev_flash_write(uint32_t pos, void *buf, uint32_t size)
{
    uint32_t len;
    rt_mutex_take(&dev_spi_flash_mutex, RT_WAITING_FOREVER);
    len = rt_device_write(dev_spi_flash, pos, buf, size);
    rt_mutex_release(&dev_spi_flash_mutex);
    return len;
}

#if 1 // spi flash测试
#include "finsh.h"
#if 0 // sfud_read()未加锁
uint8_t buf[10] = {0};
void flasht(void)
{
    static uint8_t test_cnt = 0;
    sfud_flash_t sfud_is25lp;
    rt_size_t size;
    uint8_t i;

    sfud_is25lp = rt_sfud_flash_find_by_dev_name(SPI_FLASH_NAME);
    if (sfud_is25lp == RT_NULL)
    {
        LOG_E("find spi flash dev failed");
    }
    if (sfud_is25lp == RT_NULL)
    {
        return;
    }
    {
        if (sfud_read(sfud_is25lp, 0, 10, buf) != SFUD_SUCCESS)
        {
            LOG_E("sfud read failed");
            return;
        }
        LOG_HEX("read", 10, buf, 10);
        for (i = 0; i < 10; i++)
        {
            buf[i] = test_cnt;
            test_cnt++;
        }
        LOG_HEX("will write", 10, buf, 10);
        if (sfud_erase_write(sfud_is25lp, 0, 10, buf) != SFUD_SUCCESS)
        {
            LOG_E("sfud write failed");
            return;
        }
        rt_memset(buf, 0, 10);
        if (sfud_read(sfud_is25lp, 0, 10, buf) != SFUD_SUCCESS)
        {
            LOG_E("sfud read failed");
            return;
        }
        LOG_HEX("read after write", 10, buf, 10);
    }
}
MSH_CMD_EXPORT(flasht, "spi flash test");
#else // rt_device_read()有加锁
uint8_t buf[20] = {0};
void flasht(uint32_t pos)
{
    static uint16_t test_cnt = 0;
    rt_size_t size;
    uint16_t i;
    uint32_t read_pos;

    //read_pos = pos >= 2 ? pos - 2 : pos;
    read_pos = pos;
    size = rt_device_read(dev_spi_flash, read_pos, buf, 15);
    LOG_HEX("read", 10, buf, size);
    for (i = 0; i < 10; i++)
    {
        buf[i] = test_cnt;
        test_cnt++;
    }
    LOG_HEX("will write", 10, buf, 10);
    size = rt_device_write(dev_spi_flash, pos, buf, 10);
    if (size == 0)
    {
        LOG_E("write err");
    }
    size = rt_device_read(dev_spi_flash, read_pos, buf, 15);
    LOG_HEX("after write", 10, buf, size);
}
FINSH_FUNCTION_EXPORT(flasht, "flash test");
#endif
#endif

#endif
