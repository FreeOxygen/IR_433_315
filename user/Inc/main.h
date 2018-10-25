#ifndef __MAIN_H_
#define __MAIN_H_
#include "intrins.h"
#include "STC8.H"

#define	FOSC	24000000UL //系统时钟频率使用 24MHz
typedef unsigned char BYTE;
typedef unsigned int WORD;

typedef unsigned char   u8;
typedef unsigned int    u16;
typedef unsigned long   u32;
void UartSend(char dat);

extern u8 g_Statu;
enum s
{
    s_normal = 0,//正常模式
    s_IR_Lea,    //红外学习
    s_RF_Lea,    //射频学习
};

typedef struct
{
    u32 IR_DATA; //红外数据
    u32 RF_DATA; //射频数据
    u16 RF_4T;   //射频4T时间
    u16 flag;    //标志位 0位为 433 与 315 选择
    u32 None;
}IR_RF,*pIR_RF;

#endif
