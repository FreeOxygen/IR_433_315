#include "audio.h"
#include "STC8.H"
sbit SBT = P1^0; //触发
bit audio_en;   //语音使能位
u8 audio_count; //语音计数
//void Delay5ms()		//@24.000MHz
//{
//	unsigned char i, j;

//	_nop_();
//	_nop_();
//	i = 156;
//	j = 213;
//	do
//	{
//		while (--j);
//	} while (--i);
//}
/********************** 语音初始化 ************************/
void audioInit(void)
{
	P1M0 = 0x00;
    P1M1 = 0x00;
    P1 = 0xff;
//    P1 = ((10 << 1) | 1);
//    Delay5ms();
//    SBT = 0;
//    Delay5ms();
//    Delay5ms();
//    Delay5ms();
//    Delay5ms();
//    P1 = 0xff;
}

/********************** 语音处理 ************************/
void AudioSend(void)
{
    if(audio_en)
    {
        audio_count ++;
        if(audio_count == 200)
        {
            //计时6ms
            SBT = 0;
        }
        else if(audio_count == 232)
        {
            //拉高P1口
            P1 = 0xff;
            audio_count = 0;
            audio_en = 0;
        }
    }
}

/********************** 语音播放 ************************/
void AudioPlay(u8 play)
{
    P1 = ((play << 1) | 1);
    audio_count = 0;
    audio_en = 1;
}
