#ifndef __IOT_VERSION_H__
#define __IOT_VERSION_H__

//硬件版本号(核心版)
#define HARD_VERSION	"MES85-V1.0 2022-10-24"
#define HARD_VERSION_A	1	
#define HARD_VERSION_B	0
#define HARD_VERSION_C	0
#define HARD_VERSION_D	0
//项目名称
//#define PRJ_NMAE  "chemiluminescence-200"
#define PRJ_NMAE  "MES85"

//每次程序重大改动等均应该更改此处，并在更改记录文件中说明更改内容。
#define SOFT_VERSION_A	1	
#define SOFT_VERSION_B	0
#define SOFT_VERSION_C	0
#define SOFT_VERSION_D	0

//固件识别码
#define ID_CODE_A 0
#define ID_CODE_B 0
#define ID_CODE_C 0
#define ID_CODE_D 0

void show_compile_time(void);

#endif

