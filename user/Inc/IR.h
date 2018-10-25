#ifndef __IR_H_
#define __IR_H_
#include "main.h"
extern bit B_IR_Press;
extern u32 xdata IR_RX_DATA;
void IR_RX_Enable(u8 en);
void IRScan(void);
void IR_RX_NEC(void);
#endif