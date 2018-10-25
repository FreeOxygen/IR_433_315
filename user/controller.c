///时间调度
#include "controller.h"
#include "display.h"
#include "audio.h"

#define SysTick     10000       // 次/秒, 系统滴答频率, 在4000~16000之间
#define Cut_1ms     10          //1ms  = SysTick / 1000
/****************************** 自动定义宏 ***********************************/

#define Timer0_Reload   (65536UL - ((FOSC + SysTick/2) / SysTick))     //Timer 0 中断频率, 在config.h中指定系统滴答频率, 在4000~16000之间.

/*****************************************************************************/
bit B_1ms;          //1ms标志

u8 cnt_1ms;        //1ms基本计时

///控制初始化函数
void controlInit(void)
{
    AUXR &= ~(0x80); //清除定时器0 时钟位
    AUXR |= 0x80;    //设置定时器0为1T时钟
    TH0 = (u8)(Timer0_Reload / 256);
    TL0 = (u8)(Timer0_Reload % 256);
    IPH &= ~(PT0H); IP &= ~(PT0); //设置定时器优先级为最低
    ET0 = 1;    //定时器0中断允许
    TR0 = 1;    //定时器0使能

    cnt_1ms = Cut_1ms; //设置1ms时基
    EA = 1;     //打开总中断
}

/********************** Timer0中断函数 ************************/
void timer0 (void) interrupt 1
{
    if(--cnt_1ms == 0)
    {
        cnt_1ms = Cut_1ms;
        B_1ms = 1;      //1ms标志
        DisplayScan();  //显示刷新
        AudioSend(); //语音刷新
    }
}
