/**********************************
����ͷ�ļ�
**********************************/
#include<reg52.h>
#include<intrins.h>

#ifndef __LCD_H_
#define __LCD_H_
#define uint unsigned int
#define uchar unsigned char

//Һ��������C51֮����������Ӷ��壨��ʾ�����߽�C51��P0�ڣ�
sbit en=P2^2;
sbit rw=P2^1;  
sbit rs=P2^0; 

void write_1602com(uchar com);//****Һ��д��ָ���****
void write_1602dat(uchar dat);//***Һ��д�����ݺ���****
void lcd_init();//***Һ����ʼ������****
#endif