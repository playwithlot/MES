/**
 * @file app_update_ymodem.c
 * @author {lzj} (you@domain.com)
 * @brief ??ymodem????????????flash?.
 * @version 0.1
 * @date 2020-09-17
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "update_cmd_handle.h"

#define APP_1 0
#define APP_2 1

#define APP_MAX_SIZE (512 * 1024)             //???app?????,???????????
#define APP_FLASH_INFO_ADDR (1 * 1024 * 1024) //???1M???
#define FLASH_APP1_ADDR (2 * 1024 * 1024)
#define FLASH_APP2_ADDR (3 * 1024 * 1024)
#define FLASH_PAGE_SIZE 4096 // flash???

#pragma pack(1)
typedef struct
{
    uint8_t name[80]; // app??
    uint8_t isValid;  //???app????, 0???,1???
    uint32_t len;     // app???
    uint32_t addr;    //???flash??????
} AppInfo_t;

typedef struct
{
    uint8_t currentApp; //??,??????app(0?app[0])
    AppInfo_t app[2];
    uint16_t magic; //??????
} AppFlashInfo_t;
#pragma pack()

AppFlashInfo_t appInfo;

//??????????????
static size_t update_file_total_size, update_file_cur_size;
static uint32_t copyLen = 0;
static uint32_t write_to_flash_len = 0; //????flash?????
static uint32_t page = 0;
uint32_t crc32_checksum = 0;
static uint8_t tempBuf[FLASH_PAGE_SIZE];          //????,?????????
static uint32_t flash_buf_addr = FLASH_APP1_ADDR; //???????????flash??

//???flash???app???
void ReadAppInfo(void)
{
    dev_flash_read(APP_FLASH_INFO_ADDR, (uint8_t *)&appInfo, sizeof(appInfo));
    if (appInfo.magic != 0x55aa)
    {
        rt_memset(&appInfo, 0, sizeof(appInfo));
        appInfo.magic = 0x55aa;
        appInfo.app[0].addr = FLASH_APP1_ADDR;
        appInfo.app[1].addr = FLASH_APP2_ADDR;
        dev_flash_write(APP_FLASH_INFO_ADDR, (uint8_t *)&appInfo, sizeof(appInfo));
    }
}
//??app?????flash
void SaveAppInfo(AppFlashInfo_t *info)
{
    dev_flash_write(APP_FLASH_INFO_ADDR, (uint8_t *)info, sizeof(AppFlashInfo_t));
}

//??????(???)
static uint32_t atol(char *file_size)
{
    uint32_t ret = 0;
    int8_t temp = 0;
    char *p = file_size;

    while (*p != 0x20)
    {
        temp = (*p) - '0';
        if ((temp >= 0) && (temp <= 9))
        {
            ret = ret * 10 + temp;
            p++;
        }
        else
        {
            break;
        }
    }
    return ret;
}

// FIXME: ????????crc?????
// ymodem ????????
static enum rym_code ymodem_on_begin(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len)
{
    char *file_name, *file_size;

    /* calculate and store file size */
    file_name = (char *)&buf[0];
    file_size = (char *)&buf[rt_strlen(file_name) + 1];
    update_file_total_size = atol(file_size);   //????,???
    if (update_file_total_size <= APP_MAX_SIZE) //??????
    {
        //?????????????
        update_file_cur_size = 0;
        crc32_checksum = 0;
        copyLen = 0;
        page = 0;
        write_to_flash_len = 0;
        ReadAppInfo();
        //??????????
        appInfo.app[!appInfo.currentApp].isValid = 0; //???????????
        flash_buf_addr = appInfo.app[!appInfo.currentApp].addr;
        rt_memcpy(&appInfo.app[!appInfo.currentApp].name, file_name, sizeof(appInfo.app[!appInfo.currentApp].name));
        appInfo.app[!appInfo.currentApp].name[sizeof(appInfo.app[!appInfo.currentApp].name) - 1] = '\0'; //??????????????
        appInfo.app[!appInfo.currentApp].len = update_file_total_size;
        SaveAppInfo(&appInfo);
    }
    return RYM_CODE_ACK;
}

//???????
static enum rym_code ymodem_on_data(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len)
{
    if (update_file_total_size > APP_MAX_SIZE) //????
    {
        return RYM_CODE_CAN; //????
    }
    //????
    if (update_file_cur_size + len <= update_file_total_size)
    {
        // crc32_checksum = CRC16(buf, len);
        //??????flash???
        //????,??4096?????flash?.
        if (copyLen + len < FLASH_PAGE_SIZE) //???4096
        {
            rt_memcpy(tempBuf + copyLen, buf, len);
            copyLen += len;
            if (update_file_cur_size + len == update_file_total_size) //???????????,????4096??????
            {
                dev_flash_write(flash_buf_addr + page * FLASH_PAGE_SIZE, tempBuf, copyLen);
                write_to_flash_len += copyLen;
                copyLen = 0;
            }
        }
        else if (copyLen + len == FLASH_PAGE_SIZE) //?????4096??
        {
            rt_memcpy(tempBuf + copyLen, buf, len);
            dev_flash_write(flash_buf_addr + page * FLASH_PAGE_SIZE, tempBuf, FLASH_PAGE_SIZE);
            write_to_flash_len += FLASH_PAGE_SIZE;
            page += 1;
            copyLen = 0;
        }
        else if (copyLen + len > FLASH_PAGE_SIZE) //??4096??,
        {
            rt_memcpy(tempBuf + copyLen, buf, FLASH_PAGE_SIZE - copyLen);
            dev_flash_write(flash_buf_addr + page * FLASH_PAGE_SIZE, tempBuf, FLASH_PAGE_SIZE);
            write_to_flash_len += FLASH_PAGE_SIZE;
            page += 1;
            //??4096?????
            rt_memcpy(tempBuf, buf + FLASH_PAGE_SIZE - copyLen, copyLen + len - FLASH_PAGE_SIZE);
            copyLen = copyLen + len - FLASH_PAGE_SIZE;
            if (update_file_cur_size + len == update_file_total_size) //???????????,????4096??????
            {
                dev_flash_write(flash_buf_addr + page * FLASH_PAGE_SIZE, tempBuf, copyLen);
                write_to_flash_len += copyLen;
                copyLen = 0;
            }
        }
        update_file_cur_size += len; //????????
    }
    else
    {
        // crc32_checksum = CRC16(buf, update_file_total_size - update_file_cur_size);
        if (update_file_cur_size < update_file_total_size) //??????(???????????)
        {
            rt_memcpy(tempBuf + copyLen, buf, update_file_total_size - update_file_cur_size);
            copyLen += (update_file_total_size - update_file_cur_size);
            dev_flash_write(flash_buf_addr + page * FLASH_PAGE_SIZE, tempBuf, copyLen);
            write_to_flash_len += copyLen;
        }
    }
    return RYM_CODE_ACK;
}

void update(uint8_t argc, char **argv)
{
    struct rym_ctx rctx;
    uint32_t resetHandleAddr;
    rt_kprintf("Please select the application firmware file and use Ymodem to send.\n");
    // rym_recv_on_device???????????,??rt_kprint()???????
    if (!rym_recv_on_device(&rctx, rt_console_get_device(), RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                            ymodem_on_begin, ymodem_on_data, NULL, RT_TICK_PER_SECOND))
    {
        /* wait some time for terminal response finish */
        rt_thread_delay(RT_TICK_PER_SECOND);
        rt_kprintf("file len:%d bytes, save len:%d bytes.\r\n", update_file_total_size, write_to_flash_len);
        if (update_file_total_size == write_to_flash_len)
        {
            //??????
            rt_kprintf("Download firmware to flash OK.\n");
            dev_flash_read(appInfo.app[!appInfo.currentApp].addr + 4, (uint8_t *)&resetHandleAddr, 4);
            if (resetHandleAddr >= 0x24000000 && resetHandleAddr <= 0x24080000) //???????????????
            {
                appInfo.app[!appInfo.currentApp].isValid = 1;
                appInfo.currentApp = !appInfo.currentApp; //????
                // TODO: ????????APP?
            }
            else
            {
                rt_kprintf("Download firmware is invalid\r\n");
            }
        }
        else
        {
            appInfo.app[!appInfo.currentApp].isValid = 0;
            rt_kprintf("Download firmware to flash faile.\n");
        }
        SaveAppInfo(&appInfo);
    }
    else
    {
        /* wait some time for terminal response finish */
        rt_thread_delay(RT_TICK_PER_SECOND);
        rt_kprintf("Update firmware fail.\n");
        if (update_file_total_size > APP_MAX_SIZE)
        {
            rt_kprintf("file too large\r\n");
        }
    }
}
MSH_CMD_EXPORT(update, "Update user application firmware");
FINSH_FUNCTION_EXPORT(update, "Update user application firmware");

///////////////////////////////////////////////////////////???flash????????ram//////////////
#define INSIDE_RAM_ADDR 0x24000000
uint8_t copy2ram(uint8_t appIndex)
{
    uint8_t *ramAddr = (uint8_t *)INSIDE_RAM_ADDR;
    uint32_t readSizeCnt = 0;
    AppInfo_t *app;
    uint32_t appVector;

    ReadAppInfo(); //??app???flash????.
    if (appIndex == 1)
    {
        app = &appInfo.app[1];
    }
    else
    {
        app = &appInfo.app[0];
    }
    if (app->isValid != 1) // app ????
    {
        rt_kprintf("app:%s, is invalid abandon copy;\r\n", app->name);
        return RT_ERROR;
    }
    dev_flash_read(app->addr + 4, (uint8_t *)&appVector, 4);                    //?????????????
    if (appVector < INSIDE_RAM_ADDR || appVector > (INSIDE_RAM_ADDR + 0x80000)) //????512K?,??
    {
        rt_kprintf("app:%s, vector invalid abandon copy;\r\n", app->name);
        return RT_ERROR;
    }
    while (1)
    {
        if (app->len - readSizeCnt >= FLASH_PAGE_SIZE)
        {
            dev_flash_read(app->addr + readSizeCnt, tempBuf, FLASH_PAGE_SIZE);
            rt_memcpy(ramAddr + readSizeCnt, tempBuf, FLASH_PAGE_SIZE);
            readSizeCnt += FLASH_PAGE_SIZE;
        }
        else //????1?
        {
            dev_flash_read(app->addr + readSizeCnt, tempBuf, app->len - readSizeCnt);
            rt_memcpy(ramAddr + readSizeCnt, tempBuf, app->len - readSizeCnt);
            break;
        }
    }
    rt_kprintf("\r\napp:%s, copy succeed!!!\r\n", app->name);
    return RT_EOK;
}
MSH_CMD_EXPORT(copy2ram, "copy code to sram");

#include "rthw.h"
#include "stm32h750xx.h"
#include "stm32h7xx_hal.h"

void prinfo(void)
{
    ReadAppInfo();
    rt_kprintf("fun addr(h):%x, vector(h):%x\r\n", prinfo, SCB->VTOR);
    rt_kprintf("current app is:%d, addr(h):%x\r\n", appInfo.currentApp, appInfo.app[appInfo.currentApp].addr);
    rt_kprintf("app1: name:%s, len:%d, addr(h):%x\r\n", appInfo.app[0].name, appInfo.app[0].len, appInfo.app[0].addr);
    rt_kprintf("app2: name:%s, len:%d, addr(h):%x\r\n", appInfo.app[1].name, appInfo.app[1].len, appInfo.app[1].addr);
}
MSH_CMD_EXPORT(prinfo, "print app info");

__asm void boot_jump(uint32_t address)
{
    LDR SP, [R0];
    LDR PC, [ R0, #4 ];
}
//???ram?????
void static jump2ram(void)
{
    rt_hw_interrupt_disable();
    // TODO: ??????
    SCB->VTOR = INSIDE_RAM_ADDR; // RT_APP_PART_ADDR & NVIC_VTOR_MASK;
    boot_jump(INSIDE_RAM_ADDR);
}

//?????app,???ram,????ram?
void Jump2app(void)
{
    uint8_t flag = 1;
    uint8_t appIndex;
    uint32_t vectorValue;
    ReadAppInfo(); //??app???flash????.
    appIndex = appInfo.currentApp;
    if (copy2ram(appIndex) == RT_ERROR)
    {
        appIndex = !appIndex;
        if (copy2ram(appIndex) == RT_ERROR)
        {
            flag = 0;
        }
    }
    if (flag == 1) //???flash??????ram?
    {
        vectorValue = (*(uint32_t *)(INSIDE_RAM_ADDR + 4));
        if (vectorValue >= INSIDE_RAM_ADDR && vectorValue <= (INSIDE_RAM_ADDR + 0x80000))
        {
            rt_kprintf("jump to app, app: name:%s, len:%d, addr(h):%x\r\n", appInfo.app[appIndex].name, appInfo.app[appIndex].len, appInfo.app[appIndex].addr);
            rt_thread_mdelay(10);
            jump2ram();
        }
        else
        {
            rt_kprintf("vector invalid abandon jump\r\n");
        }
    }
    else
    {
        rt_kprintf("jump to app failed\r\n");
    }
}
MSH_CMD_EXPORT(Jump2app, "Jump to app");

#if 1 //????????IAP????
#define LOG_TAG "ymodem"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>
void _user_app_test_thread_entry(void *p)
{
    uint32_t tickCnt, cnt = 0;
    uint8_t jumpTime;
    uint8_t countDown;

    countDown = jumpTime = 5;
    if (SCB->VTOR == D1_AXISRAM_BASE)
    {
        LOG_I("user app running");
    }
    else
    {
        LOG_I("IAP runnig");
    }
    LOG_I("VTOR: %X ---------", SCB->VTOR);
    tickCnt = rt_tick_get();
    while (1)
    {
        if (SCB->VTOR != D1_AXISRAM_BASE) //?????????ram?????flash??????
        {
            if (tickCnt + Sec(jumpTime) < rt_tick_get())
            {
                rt_kprintf("Jump to app, count down: %2d S\r\n", 0);
                rt_kprintf("copy app to ram and try jump to app.\r\n");
                rt_thread_mdelay(10);
                Jump2app();
                break;
            }
            else
            {
                if (cnt % 50 == 0) //?????
                {
                    rt_kprintf("Jump to app, count down: %2d S\r", countDown);
                    countDown--;
                }
                cnt++;
            }

            rt_thread_mdelay(20);
        }
        else
        {
            rt_thread_mdelay(1000);
        }
    }
    LOG_I("IAP still runnig");
    while (1)
    {
        rt_thread_mdelay(1000);
    }
}

void IAP_thread_init(void)
{
    //rt_thread_t handle;

    if (SCB->VTOR == INSIDE_RAM_ADDR)
    {
        LOG_I("app");
    }
    else
    {
        LOG_I("IAP");
        /*
        handle = rt_thread_create("IAP", _user_app_test_thread_entry, RT_NULL,1024,15,20);
        if(handle != RT_NULL)
        {
        rt_thread_startup(handle);
        }
        */
    }
}

#endif
