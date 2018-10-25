
#include "STC8.H"
#include "intrins.h"
#include "flash.h"
#include "main.h"
#include "audio.h"
#include "display.h"
#include "controller.h"
#include "key.h"
#include "IR.h"
#include "PCA.h"
#include "RF.h"
#define	BRT	(65536 - FOSC / 115200 / 4) //波特率
#define Delay_10ms (10);

bit	busy;
//char *ID;
u8 g_Statu; //状态标志
IR_RF IR_RF_data; //数据
u8 total;
void UartIsr() interrupt 4 //串口中断
{
	if (TI)
	{
		TI = 0;
		busy = 0;
	}
	if (RI)
	{
		RI = 0;
	}
}

void UartInit() //串口初始化,定时器1做波特率发生器
{
	SCON = 0x50;
	TMOD = 0x00;
	TL1 = BRT;
	TH1 = BRT >> 8;
	TR1 = 1;
	AUXR = 0x40;
	busy = 0;
}

void UartSend(char dat) //串口发送数据
{
	while (busy);
	busy = 1;
	SBUF = dat;
}

void main()
{
    u8 temp = 0;
    u8 d_10ms = Delay_10ms;//延时10ms
	UartInit(); //串口初始化
    IP |= (PS);
    IPH |= (PSH);
	ES = 1; //串口1中断使能
    g_Statu = s_normal; //正常模式	
	audioInit();
    DisplayInit(); //显示初始化
    KeyInit();
    controlInit();
    PCAInit();
    IR_RX_Enable(1);//使能 IR接收
    PCACountEnable(1); //使能 PCA
    
    RF_RX_Enable(1);//使能 RF接收
    RF_TX_Init();
    EA = 1; //允许总中断
    AudioPlay(0);
    total = *(u8 code *)(USED_ADDR);//获取数据总数
    if(total > 100)//判断数据是否正确
    {
        total = 0;
    }
//    UartSend(total);
//    UartSend(0x5A);
	while (1)
    {
        //定时
        if(B_1ms)
        {
            B_1ms = 0;
            d_10ms --;
            if(d_10ms == 100)
            {
                d_10ms = 0;
            }
        }
        
        //按键检测
        if(d_10ms == 0)
        {
            d_10ms = Delay_10ms;
			KeyScan();
        }

        switch(g_Statu)
        {
            //正常模式
            case s_normal:
                //接收红外发送射频
                if(B_IR_Press == 1)
                {
                    //红外接收成功
                    B_IR_Press = 0;
//                    UartSend(((u8 *)&(IR_RX_DATA))[0]);
//                    UartSend(((u8 *)&(IR_RX_DATA))[1]);
//                    UartSend(((u8 *)&(IR_RX_DATA))[2]);
//                    UartSend(((u8 *)&(IR_RX_DATA))[3]);
                    for(temp = 0;temp < total;temp++)
                    {
                        //查找与接收相同红外码数据
//                        UartSend(*(u8 *)(USED_ADDR_DATA + (temp << 4)+0));
//                        UartSend(*(u8 *)(USED_ADDR_DATA + (temp << 4)+1));
//                        UartSend(*(u8 *)(USED_ADDR_DATA + (temp << 4)+2));
//                        UartSend(*(u8 *)(USED_ADDR_DATA + (temp << 4)+3));
                        if(IR_RX_DATA == (*(u32 code *)(USED_ADDR_DATA + (temp << 4))))
                        {
//                            UartSend(0x11);
//                            UartSend(0x5A);
                            IR_RF_data = *(IR_RF code *)(USED_ADDR_DATA + (temp << 4));
//                            UartSend(((u8 *)&IR_RF_data.IR_DATA)[0]);
//                            UartSend(((u8 *)&IR_RF_data.IR_DATA)[1]);
//                            UartSend(((u8 *)&IR_RF_data.IR_DATA)[2]);
//                            UartSend(((u8 *)&IR_RF_data.IR_DATA)[3]);
//                            if(IR_RX_DATA == IR_RF_data.IR_DATA)
//                            {
                                //发送射频
                                if(IR_RF_data.flag == 0)
                                {
                                    //433 发送
                                    RF_OUT_DATA_433 = IR_RF_data.RF_DATA;
                                    RF_OUT_4T_433 = IR_RF_data.RF_4T;
                                    RF_TX_Switch_433_315(RF_433_Tx);
                                }
                                else if(IR_RF_data.flag == 1)
                                {
                                    //315 发送
                                    RF_OUT_DATA_315 = IR_RF_data.RF_DATA;
                                    RF_OUT_4T_315 = IR_RF_data.RF_4T;
                                    RF_TX_Switch_433_315(RF_315_Tx);
                                }
                                RF_TX();
                                AudioPlay(4);
//                            }
                            break;
                        }
                    }
                }
            break;
            //红外学习
            case s_IR_Lea:
                if(B_IR_Press == 1)
                {
                    //红外接收成功
                    B_IR_Press = 0;
                    DisplayBlinkSet(1);//个位闪烁
                    IR_RF_data.IR_DATA = IR_RX_DATA;
//                    UartSend(((u8 *)&IR_RF_data.IR_DATA)[0]);
//                    UartSend(((u8 *)&IR_RF_data.IR_DATA)[1]);
//                    UartSend(((u8 *)&IR_RF_data.IR_DATA)[2]);
//                    UartSend(((u8 *)&IR_RF_data.IR_DATA)[3]);
                    RF_RX_Enable(1);//使能射频接收
                    AudioPlay(2);
                    g_Statu = s_RF_Lea; //进入射频学习
                }
            break;
            //射频学习
            case s_RF_Lea:
                if(RF_RX_Ok)
                {
                    //接收到数据
                    DisplayBlinkSet(0);//关闭闪烁
                    RF_RX_Enable(0);//关闭射频接收
                    if(RF_RX_Ok == 1)
                    {
                        //433 接收成功
                        IR_RF_data.RF_DATA = RF_OUT_DATA_433;
                        IR_RF_data.RF_4T = RF_OUT_4T_433;
                        IR_RF_data.flag = 0;
                    }
                    else if (RF_RX_Ok == 2)
                    {
                        //315 接收成功
                        IR_RF_data.RF_DATA = RF_OUT_DATA_315;
                        IR_RF_data.RF_4T = RF_OUT_4T_315;
                        IR_RF_data.flag = 1;
                    }
//                    UartSend(((u8 *)&IR_RF_data)[0]);
//                    UartSend(((u8 *)&IR_RF_data)[1]);
//                    UartSend(((u8 *)&IR_RF_data)[2]);
//                    UartSend(((u8 *)&IR_RF_data)[3]);
//                    UartSend(((u8 *)&IR_RF_data)[4]);
//                    UartSend(((u8 *)&IR_RF_data)[5]);
//                    UartSend(((u8 *)&IR_RF_data)[6]);
//                    UartSend(((u8 *)&IR_RF_data)[7]);
//                    UartSend(((u8 *)&IR_RF_data)[8]);
//                    UartSend(((u8 *)&IR_RF_data)[9]);
//                    UartSend(((u8 *)&IR_RF_data)[10]);
//                    UartSend(((u8 *)&IR_RF_data)[11]);
//                    UartSend(((u8 *)&IR_RF_data)[12]);
//                    UartSend(((u8 *)&IR_RF_data)[13]);
//                    UartSend(((u8 *)&IR_RF_data)[14]);
//                    UartSend(((u8 *)&IR_RF_data)[15]);
//                    UartSend(0x5A);
                    if(write_flash_pr((USED_IAP_ADDR_DATA + (total << 4)),DATA_SIZE,(BYTE*) &IR_RF_data))
                    {
                        //存储成功
                        total ++;
                        IapEraseSector(0x0);
                        IapProgramByte(0x0,total);
                        if(total != (*(u8 code *)(USED_ADDR)))
                        {
                            IapEraseSector(0x0);
                            IapProgramByte(0x0,total);
                        }
//                        UartSend(total);
//                        UartSend(0x5A);
                        AudioPlay(3);
//                        IR_RF_data = *(IR_RF code *)(USED_ADDR_DATA + ((total -1) << 4));
//                        UartSend(((u8 *)&IR_RF_data)[0]);
//                        UartSend(((u8 *)&IR_RF_data)[1]);
//                        UartSend(((u8 *)&IR_RF_data)[2]);
//                        UartSend(((u8 *)&IR_RF_data)[3]);
//                        UartSend(((u8 *)&IR_RF_data)[4]);
//                        UartSend(((u8 *)&IR_RF_data)[5]);
//                        UartSend(((u8 *)&IR_RF_data)[6]);
//                        UartSend(((u8 *)&IR_RF_data)[7]);
//                        UartSend(((u8 *)&IR_RF_data)[8]);
//                        UartSend(((u8 *)&IR_RF_data)[9]);
//                        UartSend(((u8 *)&IR_RF_data)[10]);
//                        UartSend(((u8 *)&IR_RF_data)[11]);
//                        UartSend(((u8 *)&IR_RF_data)[12]);
//                        UartSend(((u8 *)&IR_RF_data)[13]);
//                        UartSend(((u8 *)&IR_RF_data)[14]);
//                        UartSend(((u8 *)&IR_RF_data)[15]);
//                        UartSend(0x5A);
                    }
                    else
                    {
                        
                        UartSend(0x33);
                        //存储失败
                    }
                    g_Statu = s_normal;
                }
            break;
        }
        //处理函数
		KeyProcess();
        IRScan();
        RF_433_Scan();
        RF_315_Scan();
    }
}

