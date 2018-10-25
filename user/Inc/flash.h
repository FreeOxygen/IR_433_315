#ifndef __FLASH_H_
#define __FLASH_H_
#include "main.h"

#define USED_ADDR (0xE000) //Flash开始地址 存储数量
#define USED_ADDR_DATA (0xE400) //存储数据地址
#define USED_IAP_ADDR_DATA (0x400) //存储数据地址
#define DATA_SIZE (sizeof(IR_RF))   //数据大小为 16

///flash 控制头文件
void IapIdle();
BYTE IapReadByte(WORD addr);
void IapProgramByte(WORD addr, BYTE dat);
void IapEraseSector(WORD addr);
BYTE write_flash_pr(WORD begin_addr, WORD counter,u8* array);
#endif
