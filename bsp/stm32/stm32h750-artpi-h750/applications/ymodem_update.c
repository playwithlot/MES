/**
 * @file app_update_ymodem.c
 * @author {lzj} (you@domain.com)
 * @brief 使用ymodem协议接收文件并保存到外部flash中.
 * @version 0.1
 * @date 2020-09-17
 *
 * @copyright Copyright (c) 2020
 *
 */
#include "ymodem.h"
#include "dev_flash.h"

#define APP_1 0
#define APP_2 1

#define APP_MAX_SIZE (512 * 1024)             //允许的app最大字节数，不接收超过此大小的文件
#define APP_FLASH_INFO_ADDR (1 * 1024 * 1024) //保存在1M地址处
#define FLASH_APP1_ADDR (2 * 1024 * 1024)
#define FLASH_APP2_ADDR (3 * 1024 * 1024)
#define FLASH_PAGE_SIZE 4096 // flash页大小

#pragma pack(1)
typedef struct
{
    uint8_t name[80]; // app名字
    uint8_t isValid;  //代表此app是否有效, 0为无效，1为有效
    uint32_t len;     // app字节数
    uint32_t addr;    //保存在flash中的起始地址
} AppInfo_t;

typedef struct
{
    uint8_t currentApp; //下标，指向向当前的app(0为app[0])
    AppInfo_t app[2];
    uint16_t magic; //数据有效标志
} AppFlashInfo_t;
#pragma pack()

AppFlashInfo_t appInfo;

//接收文件使用时的统计变量等。
static size_t update_file_total_size, update_file_cur_size;
static uint32_t copyLen = 0;
static uint32_t write_to_flash_len = 0; //统计写到flash中的字符数
static uint32_t page = 0;
uint32_t crc32_checksum = 0;
static uint8_t tempBuf[FLASH_PAGE_SIZE];          //接收缓存，可以改为动态内存。
static uint32_t flash_buf_addr = FLASH_APP1_ADDR; //用来保存接收到的文件的flash地址

//从外部flash中读取app信息。
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
//保存app信息到外部flash
void SaveAppInfo(AppFlashInfo_t *info)
{
    dev_flash_write(APP_FLASH_INFO_ADDR, (uint8_t *)info, sizeof(AppFlashInfo_t));
}

//解析文件长度（字节数）
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

// FIXME: 回调函数均未进行crc校验处理。
// ymodem 传输开始回调函数
static enum rym_code ymodem_on_begin(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len)
{
    char *file_name, *file_size;

    /* calculate and store file size */
    file_name = (char *)&buf[0];
    file_size = (char *)&buf[rt_strlen(file_name) + 1];
    update_file_total_size = atol(file_size);   //文件大小，字节数
    if (update_file_total_size <= APP_MAX_SIZE) //文件不能太大
    {
        //接收过程使用的变量初始化。
        update_file_cur_size = 0;
        crc32_checksum = 0;
        copyLen = 0;
        page = 0;
        write_to_flash_len = 0;
        ReadAppInfo();
        //保存文件名及文件大小
        appInfo.app[!appInfo.currentApp].isValid = 0; //接收并校验完才置为有效
        flash_buf_addr = appInfo.app[!appInfo.currentApp].addr;
        rt_memcpy(&appInfo.app[!appInfo.currentApp].name, file_name, sizeof(appInfo.app[!appInfo.currentApp].name));
        appInfo.app[!appInfo.currentApp].name[sizeof(appInfo.app[!appInfo.currentApp].name) - 1] = '\0'; //确保最后一字节为字符品结束符
        appInfo.app[!appInfo.currentApp].len = update_file_total_size;
        SaveAppInfo(&appInfo);
    }
    return RYM_CODE_ACK;
}

//接收文件体数据
static enum rym_code ymodem_on_data(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len)
{
    if (update_file_total_size > APP_MAX_SIZE) //文件太大
    {
        return RYM_CODE_CAN; //终止传输
    }
    //接收文件
    if (update_file_cur_size + len <= update_file_total_size)
    {
        // crc32_checksum = CRC16(buf, len);
        //将文件写入到flash中保存
        //为了提速,凑够4096字节再写到flash中.
        if (copyLen + len < FLASH_PAGE_SIZE) //还未够4096
        {
            rt_memcpy(tempBuf + copyLen, buf, len);
            copyLen += len;
            if (update_file_cur_size + len == update_file_total_size) //最后一次数据已经接收到，即使不够4096字节也要保存
            {
                dev_flash_write(flash_buf_addr + page * FLASH_PAGE_SIZE, tempBuf, copyLen);
                write_to_flash_len += copyLen;
                copyLen = 0;
            }
        }
        else if (copyLen + len == FLASH_PAGE_SIZE) //刚好接收到4096字节
        {
            rt_memcpy(tempBuf + copyLen, buf, len);
            dev_flash_write(flash_buf_addr + page * FLASH_PAGE_SIZE, tempBuf, FLASH_PAGE_SIZE);
            write_to_flash_len += FLASH_PAGE_SIZE;
            page += 1;
            copyLen = 0;
        }
        else if (copyLen + len > FLASH_PAGE_SIZE) //超过4096字节,
        {
            rt_memcpy(tempBuf + copyLen, buf, FLASH_PAGE_SIZE - copyLen);
            dev_flash_write(flash_buf_addr + page * FLASH_PAGE_SIZE, tempBuf, FLASH_PAGE_SIZE);
            write_to_flash_len += FLASH_PAGE_SIZE;
            page += 1;
            //超过4096字节的部分
            rt_memcpy(tempBuf, buf + FLASH_PAGE_SIZE - copyLen, copyLen + len - FLASH_PAGE_SIZE);
            copyLen = copyLen + len - FLASH_PAGE_SIZE;
            if (update_file_cur_size + len == update_file_total_size) //最后一次数据已经接收到，即使不够4096字节也要保存
            {
                dev_flash_write(flash_buf_addr + page * FLASH_PAGE_SIZE, tempBuf, copyLen);
                write_to_flash_len += copyLen;
                copyLen = 0;
            }
        }
        update_file_cur_size += len; //已经接收到的长度
    }
    else
    {
        // crc32_checksum = CRC16(buf, update_file_total_size - update_file_cur_size);
        if (update_file_cur_size < update_file_total_size) //最后一包数据(此时有些多余的填充数据)
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
    // rym_recv_on_device此函数会将控制台阻塞住，此时rt_kprint()打印不了数据。
    if (!rym_recv_on_device(&rctx, rt_console_get_device(), RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                            ymodem_on_begin, ymodem_on_data, NULL, RT_TICK_PER_SECOND))
    {
        /* wait some time for terminal response finish */
        rt_thread_delay(RT_TICK_PER_SECOND);
        rt_kprintf("file len:%d bytes, save len:%d bytes.\r\n", update_file_total_size, write_to_flash_len);
        if (update_file_total_size == write_to_flash_len)
        {
            //接收文件成功
            rt_kprintf("Download firmware to flash OK.\n");
            dev_flash_read(appInfo.app[!appInfo.currentApp].addr + 4, (uint8_t *)&resetHandleAddr, 4);
            if (resetHandleAddr >= 0x24000000 && resetHandleAddr <= 0x24080000) //只判断复位向量是否在合理范围内
            {
                appInfo.app[!appInfo.currentApp].isValid = 1;
                appInfo.currentApp = !appInfo.currentApp; //切换下标
                // TODO: 重启？再跳转到新APP？
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

///////////////////////////////////////////////////////////从外部flash中复制数据到内部ram//////////////
#define INSIDE_RAM_ADDR 0x24000000
uint8_t copy2ram(uint8_t appIndex)
{
    uint8_t *ramAddr = (uint8_t *)INSIDE_RAM_ADDR;
    uint32_t readSizeCnt = 0;
    AppInfo_t *app;
    uint32_t appVector;

    ReadAppInfo(); //确保app信息与flash中的一致.
    if (appIndex == 1)
    {
        app = &appInfo.app[1];
    }
    else
    {
        app = &appInfo.app[0];
    }
    if (app->isValid != 1) // app 数据无效
    {
        rt_kprintf("app:%s, is invalid abandon copy;\r\n", app->name);
        return RT_ERROR;
    }
    dev_flash_read(app->addr + 4, (uint8_t *)&appVector, 4);                    //读出复位中断函数的地址值。
    if (appVector < INSIDE_RAM_ADDR || appVector > (INSIDE_RAM_ADDR + 0x80000)) //地址不在512K内，出错
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
        else //剩下不够1页
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
//跳转到ram中运行程序
void static jump2ram(void)
{
    rt_hw_interrupt_disable();
    // TODO: 清中断标志?
    SCB->VTOR = INSIDE_RAM_ADDR; // RT_APP_PART_ADDR & NVIC_VTOR_MASK;
    boot_jump(INSIDE_RAM_ADDR);
}

//挑选合法的app，复制到ram，并跳转到ram。
void Jump2app(void)
{
    uint8_t flag = 1;
    uint8_t appIndex;
    uint32_t vectorValue;
    ReadAppInfo(); //确保app信息与flash中的一致.
    appIndex = appInfo.currentApp;
    if (copy2ram(appIndex) == RT_ERROR)
    {
        appIndex = !appIndex;
        if (copy2ram(appIndex) == RT_ERROR)
        {
            flag = 0;
        }
    }
    if (flag == 1) //已经从flash中将数据讲到ram中
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

#if 1 //创建一个线程处理IAP主要功能
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
        if (SCB->VTOR != D1_AXISRAM_BASE) //当前程序不是运行在ram才能把外部flash程序搬运进来
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
                if (cnt % 50 == 0) //倒计时显示
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
