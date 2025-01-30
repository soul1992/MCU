 //---包含头文件---//
#include<reg52.h>
#include<intrins.h>

#ifndef __DS1302_H_
#define __DS1302_H_
#define uint unsigned int
#define uchar unsigned char

//DS1302时钟芯片与C51之间的引脚连接定义
sbit SCLK=P3^4;
sbit IO=P3^5;
sbit RST=P3^6;
void Conversion(bit c,uchar year,uchar month,uchar day);
void Conver_week(uchar year,uchar month,uchar day);
void write_byte(uchar dat);//写一个字节
void write_1302(uchar add,uchar dat);//向1302芯片写函数，指定写入地址，数据
uchar read_1302(uchar add);//从1302读数据函数，指定读取数据来源地址
uchar BCD_Decimal(uchar bcd);//BCD码转十进制函数，输入BCD，返回十进制
void ds1302_init(); //1302芯片初始化子函数(2010-01-07,12:00:00,week4)
void write_sfm(uchar add,uchar dat);//向LCD写时分秒,有显示位置加、现示数据，两个参数
void write_nyr(uchar add,uchar dat);//向LCD写年月日，有显示位置加数、显示数据，两个参数
void write_nl(uchar add,uchar dat);//向LCD写时分秒,有显示位置加、现示数据，两个参数
void write_week(uchar week);//写星期函数

#endif