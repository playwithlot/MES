

#ifndef __WRITE_CMD_HANDLE_H
#define __WRITE_CMD_HANDLE_H


#include "rtthread.h"
#include "common.h"
#include "dev_can.h"
#include "dev_scanner.h"


#define MODULE_SAVE_FALSGS 1

#define MODULE_RW_FAC_PARAM         (0x0201)
#define MODULE_RW_MT_PARAM_BASE     (0x0103)
#define MODULE_RW_FUNTIONAL_SW      (0x2803)
#define MODULE_RW_POS_PARAM_BASE    (0x2a03)
#define MODULE_RW_CMD_OFFSET        (0x0100)





void cmd_write_resrc_param_handle(void *args);
void cmd_write_pos_param_handle(void *args);
void cmd_write_fsw_handle(void *args);

void cmd_write_pos_param_reset_handle(void *args);
void cmd_write_fsw_param_reset_handle(void *args);
void cmd_write_resrc_param_reset_handle(void *args);
void cmd_write_fsw_param_update_handle(void *args);
void cmd_write_pos_param_update_handle(void *args);
void cmd_write_resrc_param_update_handle(void *args);


#endif 








