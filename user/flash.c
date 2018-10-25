#include "STC8.H"
#include "intrins.h"
#include "flash.h"
#include "main.h"

#define CMD_IDLE    0               //空闲模式
#define CMD_READ    1               //IAP字节读命令
#define CMD_PROGRAM 2               //IAP字节编程命令
#define CMD_ERASE   3               //IAP扇区擦除命令

#define WT_30M          0x80
#define WT_24M          0x81
#define WT_20M          0x82
#define WT_12M          0x83
#define WT_6M           0x84
#define WT_3M           0x85
#define WT_2M           0x86
#define WT_1M           0x87

#define ERROR   0
#define OK      1
#define USED_BYTE_QTY_IN_ONE_SECTOR   512  //扇区大小
BYTE xdata protect_buffer[USED_BYTE_QTY_IN_ONE_SECTOR];

void IapIdle()
{
    IAP_CONTR = 0;                              //关闭IAP功能
    IAP_CMD = CMD_IDLE;                                //清除命令寄存器
    IAP_TRIG = 0;                               //清除触发寄存器
    IAP_ADDRH = 0x80;                           //将地址设置到非IAP区域
    IAP_ADDRL = 0;
}

/*----------------------------
从ISP/IAP/EEPROM区域读取一字节
----------------------------*/
BYTE IapReadByte(WORD addr)
{
    BYTE dat;

    IAP_CONTR = WT_24M;                         //使能IAP
    IAP_CMD = CMD_READ;                                //设置IAP读命令
    IAP_ADDRL = addr;                           //设置IAP低地址
    IAP_ADDRH = addr >> 8;                      //设置IAP高地址
    IAP_TRIG = 0x5a;                            //写触发命令(0x5a)
    IAP_TRIG = 0xa5;                            //写触发命令(0xa5)
    _nop_();
    dat = IAP_DATA;                             //读IAP数据
    IapIdle();                                  //关闭IAP功能

    return dat;
}

/*----------------------------
写一字节数据到ISP/IAP/EEPROM区域
----------------------------*/
void IapProgramByte(WORD addr, BYTE dat)
{
    IAP_CONTR = WT_24M;                         //使能IAP
    IAP_CMD = CMD_PROGRAM;                                //设置IAP写命令
    IAP_ADDRL = addr;                           //设置IAP低地址
    IAP_ADDRH = addr >> 8;                      //设置IAP高地址
    IAP_DATA = dat;                             //写IAP数据
    IAP_TRIG = 0x5a;                            //写触发命令(0x5a)
    IAP_TRIG = 0xa5;                            //写触发命令(0xa5)
    _nop_();
    IapIdle();                                  //关闭IAP功能
}

/*----------------------------
扇区擦除 512字节一个扇区
----------------------------*/
void IapEraseSector(WORD addr)
{
    IAP_CONTR = WT_24M;                         //使能IAP
    IAP_CMD = CMD_ERASE;                                //设置IAP擦除命令
    IAP_ADDRL = addr;                           //设置IAP低地址
    IAP_ADDRH = addr >> 8;                      //设置IAP高地址
    IAP_TRIG = 0x5a;                            //写触发命令(0x5a)
    IAP_TRIG = 0xa5;                            //写触发命令(0xa5)
    _nop_();                                    //
    IapIdle();                                  //关闭IAP功能
}


/* 写数据进 数据Flash存储器, 只在同一个扇区内写，不保留原有数据 */
/* begin_addr,被写数据Flash开始地址；counter,连续写多少个字节； array[]，数据来源   */
//BYTE sequential_write_flash_in_one_sector(WORD begin_addr, WORD counter, BYTE array[])
//{
//    WORD i = 0;
//    WORD in_sector_begin_addr = 0;
//    WORD sector_addr = 0;

//    /* 判是否是有效范围,此函数不允许跨扇区操作 */
//    if(counter > USED_BYTE_QTY_IN_ONE_SECTOR)
//        return  ERROR;
//    in_sector_begin_addr = begin_addr & 0x01ff;
//    if((in_sector_begin_addr + counter) > USED_BYTE_QTY_IN_ONE_SECTOR)
//        return ERROR;

//    /* 擦除 要修改/写入 的扇区 */
//    IapEraseSector(begin_addr);

//    for(i=0; i<counter; i++)
//    {
//        /* 写一个字节 */
//        IapProgramByte(begin_addr, array[i]);
//        /*  比较对错 */
//        if (SendData(IapReadByte(begin_addr)) != array[i])
//        {
//            IapIdle();
//            return ERROR;
//        }
//        begin_addr++;
//    }
//    IapIdle();
//    return  OK;
//}

/* 写数据进数据Flash存储器(EEPROM), 只在同一个扇区内写，保留同一扇区中不需修改的数据    */
/* begin_addr,被写数据Flash开始地址；counter,连续写多少个字节； array[]，数据来源       */
BYTE write_flash_pr(WORD begin_addr, WORD counter, BYTE* array)
{
    WORD i = 0;
    WORD in_sector_begin_addr = 0;
    WORD sector_addr = 0;
    WORD byte_addr = 0;
    /* 判是否是有效范围,此函数不允许跨扇区操作 */
    if(counter > USED_BYTE_QTY_IN_ONE_SECTOR)
        return ERROR;
    in_sector_begin_addr = begin_addr & 0x01ff;
    /* 假定从扇区的第0个字节开始，到USED_BYTE_QTY_IN_ONE_SECTOR-1个字节结束,后面部分不用,程序易编写 */
    if((in_sector_begin_addr + counter) > USED_BYTE_QTY_IN_ONE_SECTOR)
        return ERROR;

    /* 将该扇区数据 0 - (USED_BYTE_QTY_IN_ONE_SECTOR-1) 字节数据读入缓冲区保护 */
    sector_addr = (begin_addr & 0xfe00);
    byte_addr = sector_addr;

    //读取整页数据
    for(i = 0; i < USED_BYTE_QTY_IN_ONE_SECTOR; i++)
    {
        protect_buffer[i] = IapReadByte(byte_addr++);
    }

    /* 将要写入的数据写入保护缓冲区的相应区域,其余部分保留 */
    for(i = 0; i < counter; i++)
    {
        protect_buffer[in_sector_begin_addr++] = array[i];
    }
    
//    for(i = 0; i < USED_BYTE_QTY_IN_ONE_SECTOR; i++)
//    {
//        UartSend(protect_buffer[i]);
//    }
    /* 擦除 要修改/写入 的扇区 */
    IapEraseSector(sector_addr);

    /* 将保护缓冲区的数据写入 Data Flash, EEPROM */
    byte_addr = sector_addr;
    for(i = 0; i< USED_BYTE_QTY_IN_ONE_SECTOR; i++)
    {
        /* 写一个字节 */
        IapProgramByte(byte_addr + i, protect_buffer[i]);
        /*  比较对错 */
        if (IapReadByte(byte_addr + i) != protect_buffer[i])
        {
            IapIdle();
            return ERROR;
        }
//        byte_addr++;
    }
    IapIdle();
    return OK;
}

