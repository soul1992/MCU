#ifndef __SHOWKEY_H__
#define __SHOWKEY_H__	 
#include "stm32f10x.h"			     
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode);	//在指定位置显示一个字符串 
void Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u8 len);
#endif
