//初始化PCA
#include "PCA.h"
#include "IR.h"
#include "RF.h"
/********************** PCA模块初始化  ************************/
u8  xdata PCA_CF;                           //存储PCA计时溢出次数


void PCAInit(void)
{
    //切换PCA引脚到
    P_SW1 &= ~(0x30); //切换PCA引脚
    P_SW1 |= (0x10);  //切换到P2.3-P2.6
    CCON = 0x00; //清除所有中断标志位,停止PCA计数
    CMOD = 0x01; //空闲模式继续计数,系统时钟 12 分频进行计数进行计数[3:2],是能计数溢出中断
    CL = 0x00; //清除计数
    CH = 0x00;
}

/********************** PCA模块开始计数  ************************/
void PCACountEnable(u8 en)
{
    if(en)
    {
        //开始计数
        CL = 0x00;
        CH = 0x00;
        PCA_CF = 0;
        CR = 1;
    }
    else
    {
        //停止计数
        CR = 0;
    }
}

/********************** PCA中断处理函数  ************************/
void PCA_isr() interrupt 7 using 1
{
    //IR接收
    if (CCF1)
    {
        CCF1 = 0; //清除标志位
        if (CF && ((CCAP1H & 0x80) == 0)) //发生溢出
        {
            CF = 0;
            PCA_CF++;
        }
        IR_RX_NEC();
    }
    //433 接收
    if (CCF2)
    {
        CCF2 = 0; //清除标志位
        if (CF && ((CCAP2H & 0x80) == 0)) //发生溢出
        {
            CF = 0;
            PCA_CF++;
        }
        RF_RX_433();
    }
    
    //315 接收
    if (CCF3)
    {
        CCF3 = 0; //清除标志位
        if (CF && ((CCAP3H & 0x80) == 0)) //发生溢出
        {
            CF = 0;
            PCA_CF++;
        }
        RF_RX_315();
    }
    if (CF)
    {
        CF = 0;
        PCA_CF++;                      //PCA计时溢出次数+1
    }
}