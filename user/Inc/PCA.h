#ifndef __PCA_H_
#define __PCA_H_
#include "main.h"

#define PCA_MODE_DW (0x11)  //下降沿捕获
#define PCA_MODE_UP (0x21)  //上升沿捕获
#define PCA_MODE_ED (0x31)  //边沿捕获捕获
extern u8  xdata PCA_CF;

void PCACountEnable(u8 en);
void PCAInit(void);
#endif