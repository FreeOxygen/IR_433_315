//315  /  433 接收与发送
#include "RF.h"
#include "PCA.h"

//#define D_433_SY_H_MAX (4412)
#define mOffset  10          //接收数据偏移
#define RF_TX_H (0)     //输出高
#define RF_TX_L (1)     //输出低
sbit RF_OUT_433 = P4^3; //433 输出引脚
sbit RF_OUT_315 = P4^4; //315 输出引脚

u8 RF_RX_Ok;//RF接收成功 0 接收未完成, 1 433 接收成功 2 315 接收成功

bit RF_433_315_falg;    //发送433(0) 发送315(1) 标志位
u32 xdata RF_OUT_DATA_433;    //433 输出数据
u16 xdata RF_OUT_4T_433;      //433 输出4T

u32 xdata RF_OUT_DATA_315;    //315 输出数据
u16 xdata RF_OUT_4T_315;      //315 输出4T


u32 xdata count0_433;                       //记录上一次的捕获值
u32 xdata count1_433;                       //记录本次的捕获值
u32 xdata length_433;                       //存储信号的时间长度(count1 - count0)
u32 xdata PulseH_433;                       //存储信号的低电平时间
bit RF_433_RX_BIT_OK; //433一个脉冲接收完成
bit RF_433_RX_OK; //433 数据接收完成
u16 xdata RF_433_4T_Temp; //临时接收4T的时间
bit RF_433_flag; //射频接收标志位
u32 xdata RF_433_Data; //数据为24位,最后24位
u8 RF_433_RX_Count; //接收位数计数


u32 xdata count0_315;                       //记录上一次的捕获值
u32 xdata count1_315;                       //记录本次的捕获值
u32 xdata length_315;                       //存储信号的时间长度(count1 - count0)
u32 xdata PulseH_315;                       //存储信号的低电平时间
bit RF_315_RX_BIT_OK; //315一个脉冲接收完成
bit RF_315_RX_OK; //315 数据接收完成
u16 xdata RF_315_4T_Temp; //临时接收4T的时间
bit RF_315_flag; //射频接收标志位
u32 xdata RF_315_Data; //数据为24位,最后24位
u8 RF_315_RX_Count; //接收位数计数
/********************** 射频接收使能  ************************/
void RF_RX_Enable(u8 en)
{
    //清除所有标志位
    //433
    CCF2 = 0; //清除中断标志位
    CCAP2L = 0x00; //清除计数值
    CCAP2H = 0x00;
    //初始化所有变量
    //433
    count0_433 = 0;
    count1_433 = 0;
    length_433 = 0;
    PulseH_433 = 0;
    RF_433_RX_OK = 0;
    RF_433_flag = 0;
    
    //315
    CCF3 = 0;
    CCAP3L = 0x00; //清除计数值
    CCAP3H = 0x00;
    //315
    count0_315 = 0;
    count1_315 = 0;
    length_315 = 0;
    PulseH_315 = 0;
    RF_315_RX_OK = 0;
    RF_315_flag = 0;
    
    if(en == 1)
    {
        //开启接收
        //433 接收
        CCAPM2 = PCA_MODE_DW;                              //PCA模块0为16位捕获模式(下降沿捕获)
        //315 接收
        CCAPM3 = PCA_MODE_DW;                              //PCA模块0为16位捕获模式(下降沿捕获)
        RF_RX_Ok = 0;
        RF_OUT_DATA_433 = 0;
        RF_OUT_DATA_315 = 0;
    }
    else if(en == 0)
    {
        //关闭接收
        //433 接收
        CCAPM2 = 0;
        //315 接收
        CCAPM3 = 0;
    }
}
//###################### 433 ################################
/********************** 433接收  ************************/
void RF_RX_433(void)
{
    if(CCAPM2 == PCA_MODE_DW)
    {
        //上升沿捕获, 下一次下降沿捕获 (或得整个波形周期)
        CCAPM2 = PCA_MODE_UP;
        count0_433 = count1_433;            //备份上一次的捕获值
        ((u8 *)&count1_433)[3] = CCAP2L;  //保存上升沿到达的时间
        ((u8 *)&count1_433)[2] = CCAP2H;
        ((u8 *)&count1_433)[1] = PCA_CF;
        ((u8 *)&count1_433)[0] = 0;
        length_433 = count1_433 - count0_433;   //检测波形的周期
        ((u8 *)&length_433)[0] = 0;
        RF_433_RX_BIT_OK = 1;//接收到数据
    }
    else
    {
        //下降沿捕获, 下一次上升沿捕获 (得到高电平时间)
        CCAPM2 = PCA_MODE_DW;
        ((u8 *)&PulseH_433)[3] = CCAP2L;  //保存下降沿到达的时间
        ((u8 *)&PulseH_433)[2] = CCAP2H;
        ((u8 *)&PulseH_433)[1] = PCA_CF;
        ((u8 *)&PulseH_433)[0] = 0;
        PulseH_433 = PulseH_433 - count1_433;   //下降沿捕获计数值 - 上升沿捕获值 = 高电平时间
        ((u8 *)&PulseH_433)[0] = 0;
    }
}

/**********************  433 检测  ************************/
void RF_433_Scan(void)
{
    u8 duty = 0;
    if(RF_433_RX_BIT_OK == 1)
    {
        RF_433_RX_BIT_OK = 0;
        duty = (PulseH_433 * 100) / (length_433);
        if(RF_433_flag == 0)
        {
            //判断同步位
            if(duty <= 8)
            {
                //同步位
                RF_433_4T_Temp = (PulseH_433 & 0xFFFF);//获取4T的时间
                RF_433_flag = 1;
                RF_433_RX_Count = 24;
                RF_433_Data = 0;
            }
        }
        else if(RF_433_flag == 1)
        {
            //检测到同步位,开始接收数据
            //接收数据第一位
            if((duty <= (25 + mOffset)) && (duty >= (25 - mOffset)))
            {
                //4T高电平 数据0
                RF_433_Data >>= 1;
                if(--RF_433_RX_Count == 0)
                {
                    //数据接收完成
                    RF_433_flag = 0;
                    RF_433_RX_OK = 1;
                    RF_OUT_4T_433 = ~RF_433_4T_Temp;
                }
            }
            else if((duty <= (75 + mOffset)) && (duty >= (75 - mOffset)))
            {
                //12T高电平 数据1
                RF_433_Data >>= 1;
                RF_433_Data |= 0x800000;
                if(--RF_433_RX_Count == 0)
                {
                    //数据接收完成
                    RF_433_flag = 0;
                    RF_433_RX_OK = 1;
                    RF_OUT_4T_433 = ~RF_433_4T_Temp;
                }
            }
            else
            {
                //数据不正确
                RF_433_flag = 0;
            }
        }
    }
    
    if(RF_433_RX_OK == 1)
    {
        RF_433_RX_OK = 0;
        if(RF_OUT_DATA_433 == RF_433_Data)
        {
            //两次数据相同,接收完成,关闭射频接收
            RF_RX_Ok = 1;
            RF_RX_Enable(0);
        }
        else
        {
            //数据不同,存储最新数据
            RF_OUT_DATA_433 = RF_433_Data;
        }
//        UartSend(((u8 *)&RF_OUT_DATA_433)[0]);
//        UartSend(((u8 *)&RF_OUT_DATA_433)[1]);
//        UartSend(((u8 *)&RF_OUT_DATA_433)[2]);
//        UartSend(((u8 *)&RF_OUT_DATA_433)[3]);
    }
}

//###################### 315 ################################
/********************** 315接收  ************************/
void RF_RX_315(void)
{
    if(CCAPM3 == PCA_MODE_DW)
    {
        //上升沿捕获, 下一次下降沿捕获 (或得整个波形周期)
        CCAPM3 = PCA_MODE_UP;
        count0_315 = count1_315;            //备份上一次的捕获值
        ((u8 *)&count1_315)[3] = CCAP3L;  //保存上升沿到达的时间
        ((u8 *)&count1_315)[2] = CCAP3H;
        ((u8 *)&count1_315)[1] = PCA_CF;
        ((u8 *)&count1_315)[0] = 0;
        length_315 = count1_315 - count0_315;   //检测波形的周期
        ((u8 *)&length_315)[0] = 0;
        RF_315_RX_BIT_OK = 1;//接收到数据
    }
    else
    {
        //下降沿捕获, 下一次上升沿捕获 (得到高电平时间)
        CCAPM3 = PCA_MODE_DW;
        ((u8 *)&PulseH_315)[3] = CCAP3L;  //保存下降沿到达的时间
        ((u8 *)&PulseH_315)[2] = CCAP3H;
        ((u8 *)&PulseH_315)[1] = PCA_CF;
        ((u8 *)&PulseH_315)[0] = 0;
        PulseH_315 = PulseH_315 - count1_315;   //下降沿捕获计数值 - 上升沿捕获值 = 高电平时间
        ((u8 *)&PulseH_315)[0] = 0;
    }
}

/**********************  315 检测  ************************/
void RF_315_Scan(void)
{
    u8 duty = 0;
    if(RF_315_RX_BIT_OK == 1)
    {
        RF_315_RX_BIT_OK = 0;
        duty = (PulseH_315 * 100) / (length_315);
        if(RF_315_flag == 0)
        {
            //判断同步位
            if(duty <= 8)
            {
                //同步位
                RF_315_4T_Temp = (PulseH_315 & 0xFFFF);//获取4T的时间
                RF_315_flag = 1;
                RF_315_RX_Count = 24;
                RF_315_Data = 0;
            }
        }
        else if(RF_315_flag == 1)
        {
            //检测到同步位,开始接收数据
            //接收数据第一位
            if((duty <= (25 + mOffset)) && (duty >= (25 - mOffset)))
            {
                //4T高电平 数据0
                RF_315_Data >>= 1;
                if(--RF_315_RX_Count == 0)
                {
                    //数据接收完成
                    RF_315_flag = 0;
                    RF_315_RX_OK = 1;
                    RF_OUT_4T_315 = ~RF_315_4T_Temp;
                }
            }
            else if((duty <= (75 + mOffset)) && (duty >= (75 - mOffset)))
            {
                //12T高电平 数据1
                RF_315_Data >>= 1;
                RF_315_Data |= 0x800000;
                if(--RF_315_RX_Count == 0)
                {
                    //数据接收完成
                    RF_315_flag = 0;
                    RF_315_RX_OK = 1;
                    RF_OUT_4T_315 = ~RF_315_4T_Temp;
                }
            }
            else
            {
                //数据不正确
                RF_315_flag = 0;
            }
        }
    }
    
    if(RF_315_RX_OK == 1)
    {
        RF_315_RX_OK = 0;
        if(RF_OUT_DATA_315 == RF_315_Data)
        {
            //两次数据相同,接收完成,关闭射频接收
            RF_RX_Ok = 2;
            RF_RX_Enable(0);
        }
        else
        {
            //数据不同,存储最新数据
            RF_OUT_DATA_315 = RF_315_Data;
        }
//        UartSend(((u8 *)&RF_315_Data)[0]);
//        UartSend(((u8 *)&RF_315_Data)[1]);
//        UartSend(((u8 *)&RF_315_Data)[2]);
//        UartSend(((u8 *)&RF_315_Data)[3]);
//        UartSend(0xAA);
    }
}


//############################### 发送程序 ###############
/**********************  初始化发送 ************************/
//初始化定时器2 用作发送定时
void RF_TX_Init(void)
{
    //配置输出端口
    P4M0 = 0x00;
    P4M1 = 0x00;
    RF_OUT_433 = RF_TX_L;
    RF_OUT_315 = RF_TX_L;
}

//选择发送433 和 315  0发送433 1 发送 315
void RF_TX_Switch_433_315(u8 sw)
{
    if(sw)
    {
        RF_433_315_falg = 1;
    }
    else
    {
        RF_433_315_falg = 0;
    }
}

/********************** 433 发送  ************************/
u8 tx_count ; //发送位
u8 t_count; // 
bit tx_bit;
bit syn;
u8 temp_t;
u16 RF_TX_4T;
u32 RF_OUT_DATA;
void RF_TX(void)
{
    AUXR &= ~(0x1C); //清除定时器2相关配置
    if(RF_433_315_falg)
    {
        //315 发送
        RF_TX_4T = RF_OUT_4T_315;
        RF_OUT_DATA = RF_OUT_DATA_315;
        RF_OUT_315 = RF_TX_H;
    }
    else
    {
        //433 发送
        RF_TX_4T = RF_OUT_4T_433;
        RF_OUT_DATA = RF_OUT_DATA_433;
        RF_OUT_433 = RF_TX_H;
    }
    
    T2L = (RF_TX_4T & 0xFF);     //设置发送4T时钟
    T2H = (RF_TX_4T >> 8);
    t_count = 0;
    tx_count = 0;
    temp_t = 0;
    tx_bit = (RF_OUT_DATA >> tx_count) & 0x1;
    tx_count ++;
    syn = 0;
    AUXR |= 0x10;                                //启动定时器 12 分频
    IE2 |= ET2;                                  //使能定时器中断
}

/********************** RF发送中断 ************************/
void TM2_Isr() interrupt 12 using 1
{
    AUXINTIF &= ~T2IF;               //清中断标志
    t_count ++;
    
    if(t_count == 4)
    {
        if(tx_count < 24)
        {
            tx_bit = (RF_OUT_DATA >> tx_count) & 0x1;
            tx_count ++;
            t_count = 0;
            if(RF_433_315_falg)
            {
                //315 发送
                RF_OUT_315 = RF_TX_H;
            }
            else
            {
                //433 发送
                RF_OUT_433 = RF_TX_H;
            }
        }
        else if(syn == 0)
        {
            syn = 1;
            t_count = 0;
            if(RF_433_315_falg)
            {
                //315 发送
                RF_OUT_315 = RF_TX_H;
            }
            else
            {
                //433 发送
                RF_OUT_433 = RF_TX_H;
            }
        }
    }
    
    //发送数据
    if(syn == 0)
    {
        if(tx_bit)
        {
            //发送1
            if(t_count < 3)
            {
                if(RF_433_315_falg)
                {
                    //315 发送
                    RF_OUT_315 = RF_TX_H;
                }
                else
                {
                    //433 发送
                    RF_OUT_433 = RF_TX_H;
                }
            }
            else
            {
                if(RF_433_315_falg)
                {
                    //315 发送
                    RF_OUT_315 = RF_TX_L;
                }
                else
                {
                    //433 发送
                    RF_OUT_433 = RF_TX_L;
                }
            }
        }
        else
        {
            //发送 0 
            if(t_count < 1)
            {
                if(RF_433_315_falg)
                {
                    //315 发送
                    RF_OUT_315 = RF_TX_H;
                }
                else
                {
                    //433 发送
                    RF_OUT_433 = RF_TX_H;
                }
            }
            else
            {
                if(RF_433_315_falg)
                {
                    //315 发送
                    RF_OUT_315 = RF_TX_L;
                }
                else
                {
                    //433 发送
                    RF_OUT_433 = RF_TX_L;
                }
            }
        }
    }
    else
    {
        //发送同步帧
        if(t_count < 1)
        {
            if(RF_433_315_falg)
            {
                //315 发送
                RF_OUT_315 = RF_TX_H;
            }
            else
            {
                //433 发送
                RF_OUT_433 = RF_TX_H;
            }
        }
        else
        {
            if(RF_433_315_falg)
            {
                //315 发送
                RF_OUT_315 = RF_TX_L;
            }
            else
            {
                //433 发送
                RF_OUT_433 = RF_TX_L;
            }
            
            if(t_count == 32)
            {
                temp_t ++;
                if(temp_t > 5)
                {
                    AUXR &= ~(0x1C); //关闭定时器
                    IE2 &= ~ET2;
                }
                else
                {
                    t_count = 0;
                    tx_count = 0;
                    tx_bit = (RF_OUT_DATA >> tx_count) & 0x1;
                    tx_count ++;
                    syn = 0;
                    
                    if(RF_433_315_falg)
                    {
                        //315 发送
                        RF_OUT_315 = RF_TX_H;
                    }
                    else
                    {
                        //433 发送
                        RF_OUT_433 = RF_TX_H;
                    }
                }
            }
        }
    }
}