 //---����ͷ�ļ�---//
#include<reg52.h>
#include<intrins.h>

#ifndef __DS1302_H_
#define __DS1302_H_
#define uint unsigned int
#define uchar unsigned char

//DS1302ʱ��оƬ��C51֮����������Ӷ���
sbit SCLK=P3^4;
sbit IO=P3^5;
sbit RST=P3^6;
void Conversion(bit c,uchar year,uchar month,uchar day);
void Conver_week(uchar year,uchar month,uchar day);
void write_byte(uchar dat);//дһ���ֽ�
void write_1302(uchar add,uchar dat);//��1302оƬд������ָ��д���ַ������
uchar read_1302(uchar add);//��1302�����ݺ�����ָ����ȡ������Դ��ַ
uchar BCD_Decimal(uchar bcd);//BCD��תʮ���ƺ���������BCD������ʮ����
void ds1302_init(); //1302оƬ��ʼ���Ӻ���(2010-01-07,12:00:00,week4)
void write_sfm(uchar add,uchar dat);//��LCDдʱ����,����ʾλ�üӡ���ʾ���ݣ���������
void write_nyr(uchar add,uchar dat);//��LCDд�����գ�����ʾλ�ü�������ʾ���ݣ���������
void write_nl(uchar add,uchar dat);//��LCDдʱ����,����ʾλ�üӡ���ʾ���ݣ���������
void write_week(uchar week);//д���ں���

#endif