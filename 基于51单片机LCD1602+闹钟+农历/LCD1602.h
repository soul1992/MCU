/**********************************
包含头文件
**********************************/
#include<reg52.h>
#include<intrins.h>

#ifndef __LCD_H_
#define __LCD_H_
#define uint unsigned int
#define uchar unsigned char

//液晶屏的与C51之间的引脚连接定义（显示数据线接C51的P0口）
sbit en=P2^2;
sbit rw=P2^1;  
sbit rs=P2^0; 

void write_1602com(uchar com);//****液晶写入指令函数****
void write_1602dat(uchar dat);//***液晶写入数据函数****
void lcd_init();//***液晶初始化函数****
#endif