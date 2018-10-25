#include "display.h"

//显示段
sbit dis_1 = P2^1;
sbit dis_2 = P2^2;

//段码
BYTE code display[10] = {0xc0,0xf9,0xa4,0xB0,0x99,0x92,0x82,0xf8,0x80,0x98};

bit disControl = 0;//控制显示位
u8 xdata displayBuf[2] = {0};//后两位为闪烁存储
u8 xdata bk_flag; //闪烁控制
u16 bk_count; //闪烁计数
/********************** 显示初始化 ************************/
void DisplayInit(void)
{
    P2M0 = 0x00;
    P2M1 = 0x00;
    P0M0 = 0x00;
    P0M1 = 0x00;
    
    dis_1 = 1;
    dis_2 = 1;
    DisplaySet(00);
    bk_flag = 0;
}

/********************** 跟新显示 ************************/
void DisplaySet(u8 value)
{
    displayBuf[0] = display[value % 10];
    displayBuf[1] = display[value % 100 / 10];
}
/********************** 设置闪烁位 ************************/
void DisplayBlinkSet(u8 pos)
{
    //设置闪烁位
    bk_flag = pos;
}
/********************** 显示扫描函数 ************************/
bit bk;
void DisplayScan(void)
{
    bk_count ++;
    if(bk_count == 400)
    {
        bk_count = 0;
        bk = !bk;
    }
    if(disControl)
    {
        //显示
        dis_1 = 0;
        dis_2 = 1;
        if(bk_flag & 0x2)
        {
            //闪烁
            if(bk)
            {
                P0 = 0xFF;
            }
            else
            {
                P0 = displayBuf[disControl];
            }
        }
        else
        {
            P0 = displayBuf[disControl];
        }
        disControl = 0;
    }
    else
    {
        dis_1 = 1;
        dis_2 = 0;
        if(bk_flag & 0x1)
        {
            //闪烁
            if(bk)
            {
                P0 = 0xFF;
            }
            else
            {
                P0 = displayBuf[disControl];
            }
        }
        else
        {
            P0 = displayBuf[disControl];
        }
        disControl = 1;
    }
}