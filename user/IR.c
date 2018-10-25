///红外接收
#include "IR.h"
#include "PCA.h"
#include "display.h"

#define D_IR_SYNC_MAX       (30000)//(44117)//(15000/D_IR_sample) //SYNC max time
#define D_IR_SYNC_MIN       (19400)//(28529)//(9700 /D_IR_sample) //SYNC min time
#define D_IR_SYNC_DIVIDE    (24750)//(36397)//(12375/D_IR_sample) //decide data 0 or 1
#define D_IR_DATA_MAX       (6000)//(8824)//(3000 /D_IR_sample) //data max time
#define D_IR_DATA_MIN       (1200)//(1764)//(600  /D_IR_sample) //data min time
#define D_IR_DATA_DIVIDE    (3374)//(4961)//(1687 /D_IR_sample) //decide data 0 or 1
#define D_IR_BIT_NUMBER     32                  //bit number

u32 xdata count0;                       //记录上一次的捕获值
u32 xdata count1;                       //记录本次的捕获值
u32 xdata length;                       //存储信号的时间长度(count1 - count0)
u32 xdata PulseL;                       //存储信号的低电平时间
bit IR_flag; //红外接收标志位

u8  xdata IR_BitCnt;          //编码位数
u8  xdata IR_UserH;           //用户码(地址)高字节
u8  xdata IR_UserL;           //用户码(地址)低字节
u8  xdata IR_data;            //数据原码
u8  xdata IR_DataShit;        //数据移位
u8  xdata IR_code;            //红外键码
u16 xdata  UserCode;          //用户码
u32 xdata IR_RX_DATA;         //红外接收到数据

bit P_IR_RX_temp;       //Last sample
bit B_IR_Sync;          //已收到同步标志
bit B_IR_Press;         //安键动作发生
/********************** IR接收使能  ************************/
void IR_RX_Enable(u8 en)
{
    CCF1 = 0; //清除中断标志位
    CCAP1L = 0x00; //清除计数值
    CCAP1H = 0x00;
    //初始化所有变量
    count0 = 0;
    count1 = 0;
    length = 0;
    PulseL = 0;
    IR_flag = 0;
    B_IR_Press = 0;
    if(en == 1)
    {
        //使能IR接收
        CCAPM1 = PCA_MODE_DW;                              //PCA模块0为16位捕获模式(下降沿捕获)
    }
    else if(en == 0)
    {
        //关闭IR接收
        CCAPM1 = 0;
    }
}

/**********************  IR在捕获中断中检测波形 ************************/
void IR_RX_NEC(void)
{
    if(CCAPM1 == PCA_MODE_DW)
    {
        //下降沿捕获,下一次为上升沿捕获
        CCAPM1 = PCA_MODE_UP;
        count0 = count1;            //备份上一次的捕获值
        ((u8 *)&count1)[3] = CCAP1L;  //保存下降沿捕获值
        ((u8 *)&count1)[2] = CCAP1H;
        ((u8 *)&count1)[1] = PCA_CF;
        ((u8 *)&count1)[0] = 0;
        length = count1 - count0;   //检测波形的周期
        ((u8 *)&length)[0] = 0;
        IR_flag = 1;//接收到数据
    }
    else
    {
        //上升沿捕获,下一次捕获为下降沿捕获
        CCAPM1 = PCA_MODE_DW;
        ((u8 *)&PulseL)[3] = CCAP1L;  //保存上升沿时计数值
        ((u8 *)&PulseL)[2] = CCAP1H;
        ((u8 *)&PulseL)[1] = PCA_CF;
        ((u8 *)&PulseL)[0] = 0;
        PulseL = PulseL - count1;   //上升沿捕获计数值 - 下降沿捕获值 = 低电平时间
        ((u8 *)&PulseL)[0] = 0;
    }

}


/**********************  处理接收到数据  ************************/
void IRScan(void)
{
    if(IR_flag == 1)
    {
        IR_flag = 0;
        if(length > D_IR_SYNC_MAX)
        {
            B_IR_Sync = 0;  //large the Maxim SYNC time, then error
        }
        else if(length >= D_IR_SYNC_MIN)                    //SYNC
        {
            if(length >= D_IR_SYNC_DIVIDE)
            {
                B_IR_Sync = 1;                  //has received SYNC
                IR_BitCnt = D_IR_BIT_NUMBER;    //Load bit number
            }
        }
        else if(B_IR_Sync)                      //has received SYNC
        {
            if(length > D_IR_DATA_MAX)
            {
                B_IR_Sync=0;    //data samlpe time too large
            }
            else
            {
                IR_DataShit >>= 1;                  //data shift right 1 bit
                if(length >= D_IR_DATA_DIVIDE)  IR_DataShit |= 0x80;    //devide data 0 or 1
                if(--IR_BitCnt == 0)                //bit number is over?
                {
                    B_IR_Sync = 0;                  //Clear SYNC
                    if(~IR_DataShit == IR_data)     //判断数据正反码
                    {
                        IR_RX_DATA = ((u32)IR_UserH << 24) + ((u32)IR_UserL << 16) + (IR_data << 8) + IR_DataShit;
//                        UserCode = ((u16)IR_UserH << 8) + IR_UserL;
//                        IR_code      = IR_data;
                        B_IR_Press   = 1;           //数据有效
                    }
                }
                else if((IR_BitCnt & 7)== 0)        //one byte receive
                {
                    IR_UserL = IR_UserH;            //Save the User code high byte
                    IR_UserH = IR_data;             //Save the User code low byte
                    IR_data  = IR_DataShit;         //Save the IR data byte
                }
            }
        }
    }
    
//     if(B_IR_Press == 1)
//     {
//         B_IR_Press = 0;
// //        UartSend(((u8 *)&UserCode)[0]);
// //        UartSend(((u8 *)&UserCode)[1]);
// //        
// //        UartSend(((u8 *)&IR_RX_DATA)[0]);
// //        UartSend(((u8 *)&IR_RX_DATA)[1]);
// //        UartSend(((u8 *)&IR_RX_DATA)[2]);
// //        UartSend(((u8 *)&IR_RX_DATA)[3]);
//     // }
}





