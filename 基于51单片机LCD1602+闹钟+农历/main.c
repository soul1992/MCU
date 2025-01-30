#include<reg52.h>
#include <string.h>
#include <intrins.h>
#include "DS1302.h"
#include "LCD1602.h"

#define uint unsigned int
#define uchar unsigned char
bit timerOn=0;							//闹钟启用标志位
bit baoshi=0;							//整点报时标志位
uchar shangyimiao,bsn,temp_hour;			//记录上一秒时间
bit  p_r=0;		 						//平年/润年  =0表示平年，=1表示润年
extern data uchar year_moon,month_moon,day_moon;
extern void delay(uint xms);
//校时按键与C51的引脚连接定义
sbit set=P2^4;  	//设置键
sbit add=P2^5;  	//加键
sbit dec=P2^6;  	//减键
sbit seeNL_NZ=P2^7;	//查看农历/闹钟
sbit buzzer=P2^3;	//低电平驱动蜂鸣器模块
bit NZ_sdgb=1;
#define yh 0x80 //LCD第一行的初始位置,因为LCD1602字符地址首位D7恒定为1（100000000=80）
#define er 0x80+0x40 //LCD第二行初始位置（因为第二行第一个字符位置地址是0x40）
char nz_shi,nz_fen,setNZn;    	//定义闹钟变量
char miao,shi,fen,ri,yue,nian,setn,temp;
extern char a,week;
extern bit c_moon;
uchar T_NL_NZ;							//计数器

/**************************************************************/

extern uchar code tab1[]; 	//年显示的固定字符
extern uchar code tab2[];		//时间显示的固定字符
uchar code nlp[]={"NL:  -  -   PING"};	//农历平年显示
uchar code nlr[]={"NL:  -  -   RUN "};	//农历润年显示
uchar code NZd[]={"timer:   :      "};	//显示闹钟固定点
uchar code qk[]= {"                "};	//清空显示
uchar code tm[]= {"time"};
//------------------------------------
//时分秒显示子函数
 void write_sfm(uchar add,uchar dat)//向LCD写时分秒,有显示位置加、现示数据，两个参数
{
	uchar gw,sw;
	gw=dat%10;//取得个位数字
	sw=dat/10;//取得十位数字
	write_1602com(er+add);//er是头文件规定的值0x80+0x40
	write_1602dat(0x30+sw);//数字+30得到该数字的LCD1602显示码
	write_1602dat(0x30+gw);//数字+30得到该数字的LCD1602显示码				
}

//-------------------------------------
//年月日显示子函数
void write_nyr(uchar add,uchar dat)//向LCD写年月日，有显示位置加数、显示数据，两个参数
{
	uchar gw,sw;
	gw=dat%10;//取得个位数字
	sw=dat/10;//取得十位数字
	write_1602com(yh+add);//设定显示位置为第一个位置+add
	write_1602dat(0x30+sw);//数字+30得到该数字的LCD1602显示码
	write_1602dat(0x30+gw);//数字+30得到该数字的LCD1602显示码	
}


//------------------------------------
//农历显示子函数
void write_nl(uchar add,uchar dat)//向LCD写时分秒,有显示位置加、现示数据，两个参数
{
	
	uchar gw,sw;
	//gw=dat%10;//取得个位数字
	//sw=dat/10;//取得十位数字
	gw=dat%16;//取得个位数字
	sw=dat/16;//取得十位数字
	write_1602com(er+add);//er是头文件规定的值0x80+0x40
//	write_1602dat(0x30+sw);//数字+30得到该数字的LCD1602显示码
//	write_1602dat(0x30+gw);//数字+30得到该数字的LCD1602显示码	
	write_1602dat('0'+sw);//数字+30得到该数字的LCD1602显示码
	write_1602dat('0'+gw);//数字+30得到该数字的LCD1602显示码		
}

//-------------------------------------------
void write_week(uchar week)//写星期函数
{
	write_1602com(yh+0x0c);//星期字符的显示位置
	switch(week)
	{
		case 1:write_1602dat('M');//星期数为1时，显示
			   write_1602dat('O');
			   write_1602dat('N');
			   break;
	   
		case 2:write_1602dat('T');//星期数据为2时显示
			   write_1602dat('U');
			   write_1602dat('E');
			   break;
		
		case 3:write_1602dat('W');//星期数据为3时显示
			   write_1602dat('E');
			   write_1602dat('D');
			   break;
		
		case 4:write_1602dat('T');//星期数据为4是显示
			   write_1602dat('H');
			   write_1602dat('U');
			   break;
		
		case 5:write_1602dat('F');//星期数据为5时显示
			   write_1602dat('R');
			   write_1602dat('I');
			   break;
		
		case 6:write_1602dat('S');//星期数据为6时显示
			   write_1602dat('T');
			   write_1602dat('A');
			   break;
		
		case 0:write_1602dat('S');//星期数据为7时显示
			   write_1602dat('U');
			   write_1602dat('N');
			   break;
	}
}

//****************键盘扫描有关函数**********************
void keyscan()
{
	if(TR1==1&&setn==0&&setNZn==0&&(add==0||dec==0))
	NZ_sdgb=0;
		if(seeNL_NZ==0)
		{
			delay(9);
			if(seeNL_NZ==0)
			{
				if((setn==0)&&(setNZn==0))								//在没有进入调时模式时才可按动
				{
					buzzer=0;//蜂鸣器短响一次
		    		delay(20);
		    		buzzer=1;
		
					if(TR1==1)
					{
						TR1=0;		
					}
					else
					{			
						T_NL_NZ++;
						NZ_sdgb=1;
						if(T_NL_NZ==3)
						{
							setn=0;
							setNZn=0;
							T_NL_NZ=0;	
						}
					}			
				}
				while(seeNL_NZ==0);
			}
		}
				
	
		if(set==0)//---------------set为功能键（设置键）--------------------
		{
			delay(10);//延时，用于消抖动
			if(set==0)//延时后再次确认按键按下
			{
	    		buzzer=0;//蜂鸣器短响一次
	    		delay(20);
	    		buzzer=1;
				while(!set);
				if(T_NL_NZ==0x02)			//证明是对闹钟进行设置
				{
					setNZn++;
					if(setNZn==4)			//闹钟设定成功，退回到正常显示并开启闹钟
					{
						setNZn=0;
						setn=0;			
					}
					switch(setNZn)
					{
						case 0:						//正常显示日期时间
							write_1602com(0x0c);
							write_1602com(er);		//时间显示固定符号写入位置?
							for(a=0;a<16;a++)
							write_1602dat(NZd[a]);	//写显示时间固定符号，两个冒号
							
							write_sfm(7,nz_shi);	//闹钟 时
							write_sfm(10,nz_fen);	//闹钟 分
							write_1602com(er+13);
							if(timerOn==1)
							{
								write_1602dat('O');
								write_1602dat('N');
								write_1602dat(' ');
							}
							else
							{
								write_1602dat('O');
								write_1602dat('F');
								write_1602dat('F');
							}
							break;
						case 1:				   		//闹钟时光标闪烁		
							write_1602com(er+8);	//设置按键按动一次，小时位置显示光标   //er+0x09;
					  	 	write_1602com(0x0f);	//设置光标为闪烁
							break;
						case 2:						//闹钟分光标闪烁	
							write_1602com(er+11);	//设置按键按动一次，位置显示光标   //er+0x09;
					  	 	write_1602com(0x0f);	//设置光标为闪烁
							break;
						case 3:						//闹钟光标闪烁	
					  	 	write_1602com(0x0c);	//设置光标为不闪烁
							write_1602com(er+13);
							if(timerOn==1)
							{
								write_1602dat('O');
								write_1602dat('N');
								write_1602dat(' ');
							}
							else
							{
								write_1602dat('O');
								write_1602dat('F');
								write_1602dat('F');
							}
							break;	
					}	
				}
	
				else								//证明是对时间及日期进行设置
				{
					if(T_NL_NZ==0)
					{
						setn++;
						if(setn==7)
							setn=0;			//设置按键共有秒、分、时、星期、日、月、年、返回，8个功能循环
						switch(setn)
						{
							case 1: TR0=0;//关闭定时器
							write_1602com(er+7);//设置按键按动一次，秒位置显示光标   //er+0x09;
					  	 	write_1602com(0x0f);//设置光标为闪烁
					  	 	break;
							case 2:  
							write_1602com(er+4);  //按2次fen位置显示光标   //er+0x06	
							break;
							case 3: 
							write_1602com(er+1);   //按动3次，shi
							break;
							case 4: write_1602com(yh+0x0a);//按动4次，ri
							break;
							case 5: write_1602com(yh+0x07);//按动5次，yue
							break;
							case 6: write_1602com(yh+0x04);//按动6次，nian
							break;
							case 0:
							write_1602com(0x0c);//按动到第7次，设置光标不闪烁
							TR0=1;//打开定时器
			        	 	temp=(miao)/10*16+(miao)%10;
						   	write_1302(0x8e,0x00);
					   		write_1302(0x80,0x00|temp);//miao数据写入DS1302
					   		write_1302(0x8e,0x80);
			            	break;	
						}													
					}
				}
			}
		}
	//------------------------------加键add----------------------------		
		if((setn!=0)&&(setNZn==0))//当set按下以下。再按以下键才有效（按键次数不等于零）
		{
			if(add==0)  //上调键
			{
				delay(10);
				if(add==0)
				{
				    buzzer=0;//蜂鸣器短响一次
				    delay(20);
				    buzzer=1;
					while(!add);
					switch(setn)
					{
						case 1:miao++;//设置键按动1次，调秒
								if(miao>=60)
									miao=0;//秒超过59，再加1，就归零
								write_sfm(0x06,miao);//令LCD在正确位置显示"加"设定好的秒数
								temp=(miao)/10*16+(miao)%10;//十进制转换成DS1302要求的DCB码
							   	write_1302(0x8e,0x00); //允许写，禁止写保护 
							   	write_1302(0x80,temp); //向DS1302内写秒寄存器80H写入调整后的秒数据BCD码
							   	write_1302(0x8e,0x80); //打开写保护
								write_1602com(er+7);//因为设置液晶的模式是写入数据后，光标自动右移，所以要指定返回
								//write_1602com(0x0b);
								break;
						case 2:fen++;
								if(fen>=60)
									fen=0;
								write_sfm(0x03,fen);//令LCD在正确位置显示"加"设定好的分数据
								temp=(fen)/10*16+(fen)%10;//十进制转换成DS1302要求的DCB码
							   	write_1302(0x8e,0x00);//允许写，禁止写保护 
							   	write_1302(0x82,temp);//向DS1302内写分寄存器82H写入调整后的分数据BCD码
							   	write_1302(0x8e,0x80);//打开写保护
								write_1602com(er+4);//因为设置液晶的模式是写入数据后，指针自动加一，在这里是写回原来的位置
								break;
						case 3:shi++;
								if(shi>=24)
									shi=0;
								write_sfm(0x00,shi);//令LCD在正确的位置显示"加"设定好的小时数据
								temp=(shi)/10*16+(shi)%10;//十进制转换成DS1302要求的DCB码
							   	write_1302(0x8e,0x00);//允许写，禁止写保护 
							   	write_1302(0x84,temp);//向DS1302内写小时寄存器84H写入调整后的小时数据BCD码
							   	write_1302(0x8e,0x80);//打开写保护
								write_1602com(er+1);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
								break;
						/*
						case 4:week++;
								if(week==8)
									week=1;
					            write_1602com(yh+0x0C);//指定'加'后的周数据显示位置
									write_week(week);//指定周数据显示内容
					            temp=(week)/10*16+(week)%10;//十进制转换成DS1302要求的DCB码
							   	write_1302(0x8e,0x00);//允许写，禁止写保护 
							   	write_1302(0x8a,temp);//向DS1302内写周寄存器8aH写入调整后的周数据BCD码
							   	write_1302(0x8e,0x80);//打开写保护
								   write_1602com(yh+0x0e);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
								break;
						*/
						case 4:ri++;
								if(ri>=32)
									ri=1;
								Conver_week(nian,yue,ri);
								write_week(week);
								write_nyr(9,ri);//令LCD在正确的位置显示"加"设定好的日期数据
								temp=(ri)/10*16+(ri)%10;//十进制转换成DS1302要求的DCB码
							   	write_1302(0x8e,0x00);//允许写，禁止写保护
							   	write_1302(0x86,temp);//向DS1302内写日期寄存器86H写入调整后的日期数据BCD码
							   	write_1302(0x8e,0x80);//打开写保护
								write_1602com(yh+10);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
					
								break;
						case 5:yue++;
								if(yue>=13)
									yue=1;
								Conver_week(nian,yue,ri);
								write_week(week);
								write_nyr(6,yue);//令LCD在正确的位置显示"加"设定好的月份数据
								temp=(yue)/10*16+(yue)%10;//十进制转换成DS1302要求的DCB码
							   	write_1302(0x8e,0x00);//允许写，禁止写保护
							   	write_1302(0x88,temp);//向DS1302内写月份寄存器88H写入调整后的月份数据BCD码
							   	write_1302(0x8e,0x80);//打开写保护
								write_1602com(yh+7);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
					
								break;
						case 6:nian++;
							 if(nian>=100)
								nian=0;
								Conver_week(nian,yue,ri);
								write_week(week);
								write_nyr(3,nian);//令LCD在正确的位置显示"加"设定好的年份数据
					            temp=(nian)/10*16+(nian)%10;//十进制转换成DS1302要求的DCB码
							   	write_1302(0x8e,0x00);//允许写，禁止写保护
							   	write_1302(0x8c,temp);//向DS1302内写年份寄存器8cH写入调整后的年份数据BCD码
							   	write_1302(0x8e,0x80);//打开写保护
								write_1602com(yh+4);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
					
								break;
					}
				}
		
			}
			//------------------减键dec，各句功能参照'加键'注释---------------
			if(dec==0)
			{
				delay(10);//调延时，消抖动
				if(dec==0)
				{
			    	buzzer=0;//蜂鸣器短响一次
				    delay(20);
				    buzzer=1;
					while(!dec);
					switch(setn)
					{
						case 1:
							miao--;
							if(miao<0)
								miao=59;//秒数据减到-1时自动变成59
							write_sfm(0x06,miao);//在LCD的正确位置显示改变后新的秒数
				            temp=(miao)/10*16+(miao)%10;//十进制转换成DS1302要求的DCB码
						   	write_1302(0x8e,0x00); //允许写，禁止写保护 
						   	write_1302(0x80,temp); //向DS1302内写秒寄存器80H写入调整后的秒数据BCD码
						   	write_1302(0x8e,0x80); //打开写保护
							write_1602com(er+7);//因为设置液晶的模式是写入数据后，指针自动加一，在这里是写回原来的位置
							//write_1602com(0x0b);
							break;
						case 2:
							fen--;
							if(fen<0)
							fen=59;
							write_sfm(3,fen);
							temp=(fen)/10*16+(fen)%10;//十进制转换成DS1302要求的DCB码
						   	write_1302(0x8e,0x00);//允许写，禁止写保护 
						   	write_1302(0x82,temp);//向DS1302内写分寄存器82H写入调整后的分数据BCD码
						   	write_1302(0x8e,0x80);//打开写保护
							write_1602com(er+4);//因为设置液晶的模式是写入数据后，指针自动加一，在这里是写回原来的位置
							break;
		
						case 3:
							shi--;
					  		if(shi<0)
							shi=23;
							write_sfm(0,shi);
							temp=(shi)/10*16+(shi)%10;//十进制转换成DS1302要求的DCB码
						   	write_1302(0x8e,0x00);//允许写，禁止写保护 
						   	write_1302(0x84,temp);//向DS1302内写小时寄存器84H写入调整后的小时数据BCD码
						   	write_1302(0x8e,0x80);//打开写保护
							write_1602com(er+1);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
							break;
						case 4:
							ri--;
							if(ri<=0)
							ri=31;
							Conver_week(nian,yue,ri);
							write_week(week);
							write_nyr(9,ri);
							temp=(ri)/10*16+(ri)%10;//十进制转换成DS1302要求的DCB码
						   	write_1302(0x8e,0x00);//允许写，禁止写保护
						   	write_1302(0x86,temp);//向DS1302内写日期寄存器86H写入调整后的日期数据BCD码
						   	write_1302(0x8e,0x80);//打开写保护
							write_1602com(yh+10);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位		
							break;
						case 5:
							yue--;
							if(yue<=0)
							yue=12;
							Conver_week(nian,yue,ri);
							write_week(week);
							write_nyr(6,yue);
							temp=(yue)/10*16+(yue)%10;//十进制转换成DS1302要求的DCB码
						   	write_1302(0x8e,0x00);//允许写，禁止写保护
						   	write_1302(0x88,temp);//向DS1302内写月份寄存器88H写入调整后的月份数据BCD码
						   	write_1302(0x8e,0x80);//打开写保护
							write_1602com(yh+7);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
				
							break;	
						case 6:
							nian--;
				 			if(nian<0)
							nian=99;
							Conver_week(nian,yue,ri);
							write_week(week);
							write_nyr(3,nian);
				         	temp=(nian)/10*16+(nian)%10;//十进制转换成DS1302要求的DCB码
						   	write_1302(0x8e,0x00);//允许写，禁止写保护
						   	write_1302(0x8c,temp);//向DS1302内写年份寄存器8cH写入调整后的年份数据BCD码
						   	write_1302(0x8e,0x80);//打开写保护
							write_1602com(yh+4);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
							break;
									
					}
				}
			}
		}
		if((setNZn!=0)&&(setn==0))
		{
			if(add==0)  //上调键
			{
				delay(10);
				if(add==0)
				{
				    buzzer=0;//蜂鸣器短响一次
				    delay(20);
				    buzzer=1;
					while(!add);
					switch(setNZn)
					{
						case 3:
							timerOn=!timerOn;			
							write_1602com(er+13);
							if(timerOn==1)
							{
								write_1602dat('O');
								write_1602dat('N');
								write_1602dat(' ');
							}
							else
							{
								write_1602dat('O');
								write_1602dat('F');
								write_1602dat('F');
							}
							break;
						case 2:
							nz_fen++;
							if(nz_fen>=60)
								nz_fen=0;
							write_sfm(10,nz_fen);//令LCD在正确位置显示"加"设定好的分数据
							write_1602com(er+11);//因为设置液晶的模式是写入数据后，指针自动加一，在这里是写回原来的位置
							break;
						case 1:
							nz_shi++;
							if(nz_shi>=24)
								nz_shi=0;
							write_sfm(7,nz_shi);//令LCD在正确的位置显示"加"设定好的小时数据
							write_1602com(er+8);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
							break;
					}
				}
		
			}
			//------------------减键dec，各句功能参照'加键'注释---------------
			if(dec==0)
			{
				delay(10);//调延时，消抖动
				if(dec==0)
				{
			    	buzzer=0;//蜂鸣器短响一次
				    delay(20);
				    buzzer=1;
					while(!dec);
					switch(setNZn)
					{
						case 3:
							timerOn=!timerOn;			
							write_1602com(er+13);
							if(timerOn==1)
							{
								write_1602dat('O');
								write_1602dat('N');
								write_1602dat(' ');
							}
							else
							{
								write_1602dat('O');
								write_1602dat('F');
								write_1602dat('F');
							}
							break;
						case 2:
							nz_fen--;
							if(nz_fen<0)
								nz_fen=59;
							write_sfm(10,nz_fen);
							write_1602com(er+11);//因为设置液晶的模式是写入数据后，指针自动加一，在这里是写回原来的位置
							break;
		
						case 1:
							nz_shi--;
					  		if(nz_shi<0)
							nz_shi=23;
							write_sfm(7,nz_shi);
							write_1602com(er+8);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
							break;
					}
				}
			}
		}
}

//-------------------------------
void init(void)   //定时器、计数器设置函数
{
	TMOD=0x11; 		//指定定时/计数器的工作方式为3
	TH0=0; 			//定时器T0的高四位=0
	TL0=0;  		//定时器T0的低四位=0
	TH1=0x3C;
	TL1=0xB0;
	EA=1;  			//系统允许有开放的中断
	ET0=1; 			//允许T0中断
	ET1=1;
	IT1=1;
	IT0=0;
	TR0=1; 			//开启中断，启动定时器
	TR1=0;
}


void alarm(void)
{
	if(shi==nz_shi&&fen==nz_fen&&miao==0&&NZ_sdgb==1)
	{
	    TR1=1;
	}
	if(((shi==nz_shi)&&(fen==(nz_fen+1)))||NZ_sdgb==0)
	{
		TR1=0;
		buzzer=1;
	}
	if((shi==nz_shi)&&(fen>nz_fen))
	{
		TR1=0;
		buzzer=1;
		NZ_sdgb=1;
	}
}


void ZD_baoshi(void)
{
	buzzer=0;
	delay(5);
	buzzer=1;
	bsn++;
	if(bsn==temp_hour)
	{
		bsn=0;
		baoshi=0;
	}
}

//*******************主函数**************************
void main()
{
	P1=0xff;
	lcd_init();      //调用液晶屏初始化子函数
	ds1302_init();   //调用DS1302时钟的初始化子函数
	init();          //调用定时计数器的设置子函数
    buzzer=1;		 //蜂鸣器长响一次
    delay(100);
    buzzer=0;
	while(1)  //无限循环下面的语句：
	{		
   	 	keyscan();      //调用键盘扫描子函数		
		if(timerOn==1)
			alarm();	//闹钟输出
		if((fen==0)&&(miao==0))
		{
			if(shi>12)
				temp_hour=shi-12;
			else
			{
				if(shi==0)
					temp_hour=12;
				else
					temp_hour=shi;
			}
			shangyimiao=miao;
			baoshi=1;
		}
		if(baoshi==1)
		{
			ZD_baoshi();
			do 
				keyscan();
			while(shangyimiao==miao);	
			shangyimiao=miao;
		}
    }
}
void timer0() interrupt 1  //取得并显示日历和时间
{
  //读取秒时分周日月年七个数据（DS1302的读寄存器与写寄存器不一样）：
	
    miao = BCD_Decimal(read_1302(0x81));
	fen = BCD_Decimal(read_1302(0x83));
	shi  = BCD_Decimal(read_1302(0x85));
	ri  = BCD_Decimal(read_1302(0x87));
	yue = BCD_Decimal(read_1302(0x89));
	nian=BCD_Decimal(read_1302(0x8d));
    if(T_NL_NZ==1)							//显示农历
	{
		uint nian_temp,temp12;
		temp12=nian;
		nian_temp=2000+(temp12&0xF0)*10+temp12&0x0F;
		if((nian_temp%400==0)||((nian_temp%100!=0)&&(nian_temp%4==0)))  //判断是否为闰年
			p_r=1;
		else
			p_r=0;
		Conversion(0,nian,yue,ri);
		write_1602com(er);//时间显示固定符号写入位置?
		for(a=0;a<16;a++)
		{
			if(p_r==0)
				write_1602dat(nlp[a]);//写显示时间固定符号，两个冒号
			else 
				write_1602dat(nlr[a]);
		}

	  	write_nl(3,year_moon);//农历 年
		write_nl(6,month_moon);//农历 月
		write_nl(9,day_moon);//农历 日

		do
			keyscan();
		while(T_NL_NZ==1);

		write_1602com(er);//时间显示固定符号写入位置，从第2个位置后开始显示
		for(a=0;a<16;a++)
		{
			write_1602dat(qk[a]);//写显示时间固定符号，两个冒号
		}

		write_1602com(er);//时间显示固定符号写入位置，从第2个位置后开始显示
		for(a=0;a<8;a++)
		{
			write_1602dat(tab2[a]);//写显示时间固定符号，两个冒号
		}
	}

	if(T_NL_NZ==2)								//显示闹钟时间，
	{
		write_1602com(er);//时间显示固定符号写入位置?
		for(a=0;a<16;a++)
			write_1602dat(NZd[a]);//写显示时间固定符号，两个冒号

	  	write_sfm(7,nz_shi);//农历 年
		write_sfm(10,nz_fen);//农历 月
		write_1602com(er+13);
		if(timerOn==1)
		{
			write_1602dat('O');
			write_1602dat('N');
			write_1602dat(' ');
		}
		else
		{
			write_1602dat('O');
			write_1602dat('F');
			write_1602dat('F');
		}

		do
			keyscan();
		while(T_NL_NZ==2);

		write_1602com(er);//时间显示固定符号写入位置，从第2个位置后开始显示
		for(a=0;a<16;a++)
		{
			write_1602dat(qk[a]);//写显示时间固定符号，两个冒号
		}

		write_1602com(er);//时间显示固定符号写入位置，从第2个位置后开始显示
		for(a=0;a<8;a++)
		{
			write_1602dat(tab2[a]);//写显示时间固定符号，两个冒号
		}
	}
	

	else
	{	
		//显示秒、时、分数据： 
			write_1602com(er+12);
			for(a=0;a<4;a++)
			{
				write_1602dat(tm[a]);
			}
	  	write_sfm(6,miao);//秒，从第二行第8个字后开始显示（调用时分秒显示子函数）
		write_sfm(3,fen);//分，从第二行第5个字符后开始显示
		write_sfm(0,shi);//小时，从第二行第2个字符后开始显示
	}	
		//显示日、月、年数据：
		write_nyr(9,ri);//日期，从第二行第9个字符后开始显示
	   	write_nyr(6,yue);//月份，从第二行第6个字符后开始显示
		write_nyr(3,nian);//年，从第二行第3个字符后开始显示
		Conver_week(nian,yue,ri);
		write_week(week);
}


unsigned char count1;

void timer1() interrupt 3  //取得并显示日历和时间
{
	TH1=0x3C;
	TL1=0xB0;
	count1++;
	if(count1==10)
	{
		count1=0;
		buzzer=!buzzer;
	}
}
