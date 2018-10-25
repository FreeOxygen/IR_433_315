#include "key.h"
#include "display.h"
#include "RF.h"
#include "audio.h"
#define shortDwn (10) //短按计数
#define longDwn (50) //长按计数
//按键按下为0
sbit K1 = P3^3;
sbit K2 = P3^4;
sbit K3 = P3^5;
sbit K4 = P3^6;

typedef struct {
    u8 keySD : 1; //短按
    u8 keyLD : 1; //长按
    u8 keyFg : 1;
    u8 keyCn;
}key_t;

key_t xdata keyArry[4];
/**********************  按键初始化 ************************/
void KeyInit(void)
{
    u8 i = 0;
    P3M0 = 0x00;
    P3M1 = 0x00;
    //初始化所有按键
    for(i = 0; i < 4; i++)
    {
        keyArry[i].keySD = 0;
        keyArry[i].keyLD = 0;
        keyArry[i].keyFg = 0;
        keyArry[i].keyCn = 0;
    }
}
/**********************  按键扫描 ************************/
void KeyScan(void)
{
    //按键1***********************************
    if(!K1)
    {
        //按键按下
        if(keyArry[0].keyFg)
        {
            //按键按下开始计数
            keyArry[0].keyCn ++;
            //长按
            if(keyArry[0].keyCn > longDwn)
            {
                keyArry[0].keyLD = 1;
            }
        }
        else
        {
            //按键第一次被按下
            keyArry[0].keyFg = 1;
        }
    }
    else
    {
        //按键松开
       if((keyArry[0].keyLD == 0) && (keyArry[0].keyCn > shortDwn))
       {
           //短按
           keyArry[0].keySD = 1;
       }
        //清除按键值
        keyArry[0].keyFg = 0;
        keyArry[0].keyCn = 0;
        keyArry[0].keyLD = 0;
    }
    
    //按键2********************************
    if(!K2)
    {
        //按键按下
        if(keyArry[1].keyFg)
        {
            //按键按下开始计数
            keyArry[1].keyCn ++;
            //长按
            if(keyArry[1].keyCn > longDwn)
            {
                keyArry[1].keyLD = 1;
            }
        }
        else
        {
            //按键第一次被按下
            keyArry[1].keyFg = 1;
        }
    }
    else
    {
        //按键松开
       if((keyArry[1].keyLD == 0) && (keyArry[1].keyCn > shortDwn))
       {
           //短按
           keyArry[1].keySD = 1;
       }
        //清除按键值
        keyArry[1].keyFg = 0;
        keyArry[1].keyCn = 0;
        keyArry[1].keyLD = 0;
    }
    
    //按键3********************************
    if(!K3)
    {
        //按键按下
        if(keyArry[2].keyFg)
        {
            //按键按下开始计数
            keyArry[2].keyCn ++;
            //长按
            if(keyArry[2].keyCn > longDwn)
            {
                keyArry[2].keyLD = 1;
            }
        }
        else
        {
            //按键第一次被按下
            keyArry[2].keyFg = 1;
        }
    }
    else
    {
        //按键松开
        if((keyArry[2].keyLD == 0) && (keyArry[2].keyCn > shortDwn))
        {
            //短按
            keyArry[2].keySD = 1;
        }
        //清除按键值
        keyArry[2].keyFg = 0;
        keyArry[2].keyCn = 0;
        keyArry[2].keyLD = 0;
    }
    
    //按键4********************************
    if(!K4)
    {
        //按键按下
        if(keyArry[3].keyFg)
        {
            //按键按下开始计数
            keyArry[3].keyCn ++;
            //长按
            if(keyArry[3].keyCn > longDwn)
            {
                keyArry[3].keyLD = 1;
            }
        }
        else
        {
            //按键第一次被按下
            keyArry[3].keyFg = 1;
        }
    }
    else
    {
        //按键松开
        if((keyArry[3].keyLD == 0) && (keyArry[3].keyCn > shortDwn))
        {
            //短按
            keyArry[3].keySD = 1;
        }
        //清除按键值
        keyArry[3].keyFg = 0;
        keyArry[3].keyCn = 0;
        keyArry[3].keyLD = 0;
    }
}
/**********************  按键处理 ************************/
u8 temp = 0;
void KeyProcess(void)
{
    //**********K1
    if(keyArry[0].keySD)
    {
        //处理短按键
        keyArry[0].keySD = 0;
        if(g_Statu == s_normal)
        {
            //进入设置模式
            DisplayBlinkSet(3);
            g_Statu = s_IR_Lea;//进入IR学习模式
            AudioPlay(1);
        }
        
        // temp ++;
        // DisplaySet(temp);
        // RF_RX_Enable(0);
        // RF_TX_Switch_433_315(RF_433_Tx);
        // RF_TX();
    }
    
    //**********K2
    if(keyArry[1].keySD)
    {
        //处理短按键
        keyArry[1].keySD = 0;
        temp --;
        DisplaySet(temp);
        RF_RX_Enable(1);
    }
    
    //**********K3
    if(keyArry[2].keySD)
    {
        //处理短按键
        keyArry[2].keySD = 0;
        temp ++;
        DisplaySet(temp);
        AudioPlay(temp);
//        RF_RX_Enable(0);
//        RF_TX_Switch_433_315(RF_315_Tx);
//        RF_TX();
    }
    
    //**********K4
    if(keyArry[3].keySD)
    {
        //处理短按键
        keyArry[3].keySD = 0;
        temp --;
        DisplaySet(temp);
    }
    
    //长按键
    //**********k1
    if(keyArry[0].keyLD)
    {
        //处理短按键`
    }
    
    //**********k2
    if(keyArry[1].keyLD)
    {
        //处理短按键
    }
    
    //**********k3
    if(keyArry[2].keyLD)
    {
        //处理短按键
    }
    
    //**********k4
    if(keyArry[3].keyLD)
    {
        //处理短按键
    }
}