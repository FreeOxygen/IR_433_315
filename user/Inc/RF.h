#ifndef __RF_H_
#define __RF_H_
#include "main.h"
#define RF_433_Tx (0)
#define RF_315_Tx (1)

extern u8 RF_RX_Ok;
// extern bit RF_433_315_falg;    //发送433(0) 发送315(1) 标志位
extern u32 xdata RF_OUT_DATA_433;    //433 输出数据
extern u16 xdata RF_OUT_4T_433;      //433 输出4T
extern u32 xdata RF_OUT_DATA_315;    //315 输出数据
extern u16 xdata RF_OUT_4T_315;      //315 输出4T

void RF_RX_433(void);
void RF_433_Scan(void);

void RF_RX_315(void);
void RF_315_Scan(void);
void RF_RX_Enable(u8 en);
void RF_TX_Init(void);
void RF_TX(void);
void RF_TX_Switch_433_315(u8 sw);
#endif