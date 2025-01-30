#include<reg52.h>
#include <string.h>
#include <intrins.h>
#include "DS1302.h"
#include "LCD1602.h"

#define uint unsigned int
#define uchar unsigned char
bit timerOn=0;							//�������ñ�־λ
bit baoshi=0;							//���㱨ʱ��־λ
uchar shangyimiao,bsn,temp_hour;			//��¼��һ��ʱ��
bit  p_r=0;		 						//ƽ��/����  =0��ʾƽ�꣬=1��ʾ����
extern data uchar year_moon,month_moon,day_moon;
extern void delay(uint xms);
//Уʱ������C51���������Ӷ���
sbit set=P2^4;  	//���ü�
sbit add=P2^5;  	//�Ӽ�
sbit dec=P2^6;  	//����
sbit seeNL_NZ=P2^7;	//�鿴ũ��/����
sbit buzzer=P2^3;	//�͵�ƽ����������ģ��
bit NZ_sdgb=1;
#define yh 0x80 //LCD��һ�еĳ�ʼλ��,��ΪLCD1602�ַ���ַ��λD7�㶨Ϊ1��100000000=80��
#define er 0x80+0x40 //LCD�ڶ��г�ʼλ�ã���Ϊ�ڶ��е�һ���ַ�λ�õ�ַ��0x40��
char nz_shi,nz_fen,setNZn;    	//�������ӱ���
char miao,shi,fen,ri,yue,nian,setn,temp;
extern char a,week;
extern bit c_moon;
uchar T_NL_NZ;							//������

/**************************************************************/

extern uchar code tab1[]; 	//����ʾ�Ĺ̶��ַ�
extern uchar code tab2[];		//ʱ����ʾ�Ĺ̶��ַ�
uchar code nlp[]={"NL:  -  -   PING"};	//ũ��ƽ����ʾ
uchar code nlr[]={"NL:  -  -   RUN "};	//ũ��������ʾ
uchar code NZd[]={"timer:   :      "};	//��ʾ���ӹ̶���
uchar code qk[]= {"                "};	//�����ʾ
uchar code tm[]= {"time"};
//------------------------------------
//ʱ������ʾ�Ӻ���
 void write_sfm(uchar add,uchar dat)//��LCDдʱ����,����ʾλ�üӡ���ʾ���ݣ���������
{
	uchar gw,sw;
	gw=dat%10;//ȡ�ø�λ����
	sw=dat/10;//ȡ��ʮλ����
	write_1602com(er+add);//er��ͷ�ļ��涨��ֵ0x80+0x40
	write_1602dat(0x30+sw);//����+30�õ������ֵ�LCD1602��ʾ��
	write_1602dat(0x30+gw);//����+30�õ������ֵ�LCD1602��ʾ��				
}

//-------------------------------------
//��������ʾ�Ӻ���
void write_nyr(uchar add,uchar dat)//��LCDд�����գ�����ʾλ�ü�������ʾ���ݣ���������
{
	uchar gw,sw;
	gw=dat%10;//ȡ�ø�λ����
	sw=dat/10;//ȡ��ʮλ����
	write_1602com(yh+add);//�趨��ʾλ��Ϊ��һ��λ��+add
	write_1602dat(0x30+sw);//����+30�õ������ֵ�LCD1602��ʾ��
	write_1602dat(0x30+gw);//����+30�õ������ֵ�LCD1602��ʾ��	
}


//------------------------------------
//ũ����ʾ�Ӻ���
void write_nl(uchar add,uchar dat)//��LCDдʱ����,����ʾλ�üӡ���ʾ���ݣ���������
{
	
	uchar gw,sw;
	//gw=dat%10;//ȡ�ø�λ����
	//sw=dat/10;//ȡ��ʮλ����
	gw=dat%16;//ȡ�ø�λ����
	sw=dat/16;//ȡ��ʮλ����
	write_1602com(er+add);//er��ͷ�ļ��涨��ֵ0x80+0x40
//	write_1602dat(0x30+sw);//����+30�õ������ֵ�LCD1602��ʾ��
//	write_1602dat(0x30+gw);//����+30�õ������ֵ�LCD1602��ʾ��	
	write_1602dat('0'+sw);//����+30�õ������ֵ�LCD1602��ʾ��
	write_1602dat('0'+gw);//����+30�õ������ֵ�LCD1602��ʾ��		
}

//-------------------------------------------
void write_week(uchar week)//д���ں���
{
	write_1602com(yh+0x0c);//�����ַ�����ʾλ��
	switch(week)
	{
		case 1:write_1602dat('M');//������Ϊ1ʱ����ʾ
			   write_1602dat('O');
			   write_1602dat('N');
			   break;
	   
		case 2:write_1602dat('T');//��������Ϊ2ʱ��ʾ
			   write_1602dat('U');
			   write_1602dat('E');
			   break;
		
		case 3:write_1602dat('W');//��������Ϊ3ʱ��ʾ
			   write_1602dat('E');
			   write_1602dat('D');
			   break;
		
		case 4:write_1602dat('T');//��������Ϊ4����ʾ
			   write_1602dat('H');
			   write_1602dat('U');
			   break;
		
		case 5:write_1602dat('F');//��������Ϊ5ʱ��ʾ
			   write_1602dat('R');
			   write_1602dat('I');
			   break;
		
		case 6:write_1602dat('S');//��������Ϊ6ʱ��ʾ
			   write_1602dat('T');
			   write_1602dat('A');
			   break;
		
		case 0:write_1602dat('S');//��������Ϊ7ʱ��ʾ
			   write_1602dat('U');
			   write_1602dat('N');
			   break;
	}
}

//****************����ɨ���йغ���**********************
void keyscan()
{
	if(TR1==1&&setn==0&&setNZn==0&&(add==0||dec==0))
	NZ_sdgb=0;
		if(seeNL_NZ==0)
		{
			delay(9);
			if(seeNL_NZ==0)
			{
				if((setn==0)&&(setNZn==0))								//��û�н����ʱģʽʱ�ſɰ���
				{
					buzzer=0;//����������һ��
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
				
	
		if(set==0)//---------------setΪ���ܼ������ü���--------------------
		{
			delay(10);//��ʱ������������
			if(set==0)//��ʱ���ٴ�ȷ�ϰ�������
			{
	    		buzzer=0;//����������һ��
	    		delay(20);
	    		buzzer=1;
				while(!set);
				if(T_NL_NZ==0x02)			//֤���Ƕ����ӽ�������
				{
					setNZn++;
					if(setNZn==4)			//�����趨�ɹ����˻ص�������ʾ����������
					{
						setNZn=0;
						setn=0;			
					}
					switch(setNZn)
					{
						case 0:						//������ʾ����ʱ��
							write_1602com(0x0c);
							write_1602com(er);		//ʱ����ʾ�̶�����д��λ��?
							for(a=0;a<16;a++)
							write_1602dat(NZd[a]);	//д��ʾʱ��̶����ţ�����ð��
							
							write_sfm(7,nz_shi);	//���� ʱ
							write_sfm(10,nz_fen);	//���� ��
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
						case 1:				   		//����ʱ�����˸		
							write_1602com(er+8);	//���ð�������һ�Σ�Сʱλ����ʾ���   //er+0x09;
					  	 	write_1602com(0x0f);	//���ù��Ϊ��˸
							break;
						case 2:						//���ӷֹ����˸	
							write_1602com(er+11);	//���ð�������һ�Σ�λ����ʾ���   //er+0x09;
					  	 	write_1602com(0x0f);	//���ù��Ϊ��˸
							break;
						case 3:						//���ӹ����˸	
					  	 	write_1602com(0x0c);	//���ù��Ϊ����˸
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
	
				else								//֤���Ƕ�ʱ�估���ڽ�������
				{
					if(T_NL_NZ==0)
					{
						setn++;
						if(setn==7)
							setn=0;			//���ð��������롢�֡�ʱ�����ڡ��ա��¡��ꡢ���أ�8������ѭ��
						switch(setn)
						{
							case 1: TR0=0;//�رն�ʱ��
							write_1602com(er+7);//���ð�������һ�Σ���λ����ʾ���   //er+0x09;
					  	 	write_1602com(0x0f);//���ù��Ϊ��˸
					  	 	break;
							case 2:  
							write_1602com(er+4);  //��2��fenλ����ʾ���   //er+0x06	
							break;
							case 3: 
							write_1602com(er+1);   //����3�Σ�shi
							break;
							case 4: write_1602com(yh+0x0a);//����4�Σ�ri
							break;
							case 5: write_1602com(yh+0x07);//����5�Σ�yue
							break;
							case 6: write_1602com(yh+0x04);//����6�Σ�nian
							break;
							case 0:
							write_1602com(0x0c);//��������7�Σ����ù�겻��˸
							TR0=1;//�򿪶�ʱ��
			        	 	temp=(miao)/10*16+(miao)%10;
						   	write_1302(0x8e,0x00);
					   		write_1302(0x80,0x00|temp);//miao����д��DS1302
					   		write_1302(0x8e,0x80);
			            	break;	
						}													
					}
				}
			}
		}
	//------------------------------�Ӽ�add----------------------------		
		if((setn!=0)&&(setNZn==0))//��set�������¡��ٰ����¼�����Ч�����������������㣩
		{
			if(add==0)  //�ϵ���
			{
				delay(10);
				if(add==0)
				{
				    buzzer=0;//����������һ��
				    delay(20);
				    buzzer=1;
					while(!add);
					switch(setn)
					{
						case 1:miao++;//���ü�����1�Σ�����
								if(miao>=60)
									miao=0;//�볬��59���ټ�1���͹���
								write_sfm(0x06,miao);//��LCD����ȷλ����ʾ"��"�趨�õ�����
								temp=(miao)/10*16+(miao)%10;//ʮ����ת����DS1302Ҫ���DCB��
							   	write_1302(0x8e,0x00); //����д����ֹд���� 
							   	write_1302(0x80,temp); //��DS1302��д��Ĵ���80Hд��������������BCD��
							   	write_1302(0x8e,0x80); //��д����
								write_1602com(er+7);//��Ϊ����Һ����ģʽ��д�����ݺ󣬹���Զ����ƣ�����Ҫָ������
								//write_1602com(0x0b);
								break;
						case 2:fen++;
								if(fen>=60)
									fen=0;
								write_sfm(0x03,fen);//��LCD����ȷλ����ʾ"��"�趨�õķ�����
								temp=(fen)/10*16+(fen)%10;//ʮ����ת����DS1302Ҫ���DCB��
							   	write_1302(0x8e,0x00);//����д����ֹд���� 
							   	write_1302(0x82,temp);//��DS1302��д�ּĴ���82Hд�������ķ�����BCD��
							   	write_1302(0x8e,0x80);//��д����
								write_1602com(er+4);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ����������д��ԭ����λ��
								break;
						case 3:shi++;
								if(shi>=24)
									shi=0;
								write_sfm(0x00,shi);//��LCD����ȷ��λ����ʾ"��"�趨�õ�Сʱ����
								temp=(shi)/10*16+(shi)%10;//ʮ����ת����DS1302Ҫ���DCB��
							   	write_1302(0x8e,0x00);//����д����ֹд���� 
							   	write_1302(0x84,temp);//��DS1302��дСʱ�Ĵ���84Hд��������Сʱ����BCD��
							   	write_1302(0x8e,0x80);//��д����
								write_1602com(er+1);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ��������Ҫ����λ
								break;
						/*
						case 4:week++;
								if(week==8)
									week=1;
					            write_1602com(yh+0x0C);//ָ��'��'�����������ʾλ��
									write_week(week);//ָ����������ʾ����
					            temp=(week)/10*16+(week)%10;//ʮ����ת����DS1302Ҫ���DCB��
							   	write_1302(0x8e,0x00);//����д����ֹд���� 
							   	write_1302(0x8a,temp);//��DS1302��д�ܼĴ���8aHд��������������BCD��
							   	write_1302(0x8e,0x80);//��д����
								   write_1602com(yh+0x0e);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ��������Ҫ����λ
								break;
						*/
						case 4:ri++;
								if(ri>=32)
									ri=1;
								Conver_week(nian,yue,ri);
								write_week(week);
								write_nyr(9,ri);//��LCD����ȷ��λ����ʾ"��"�趨�õ���������
								temp=(ri)/10*16+(ri)%10;//ʮ����ת����DS1302Ҫ���DCB��
							   	write_1302(0x8e,0x00);//����д����ֹд����
							   	write_1302(0x86,temp);//��DS1302��д���ڼĴ���86Hд����������������BCD��
							   	write_1302(0x8e,0x80);//��д����
								write_1602com(yh+10);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ��������Ҫ����λ
					
								break;
						case 5:yue++;
								if(yue>=13)
									yue=1;
								Conver_week(nian,yue,ri);
								write_week(week);
								write_nyr(6,yue);//��LCD����ȷ��λ����ʾ"��"�趨�õ��·�����
								temp=(yue)/10*16+(yue)%10;//ʮ����ת����DS1302Ҫ���DCB��
							   	write_1302(0x8e,0x00);//����д����ֹд����
							   	write_1302(0x88,temp);//��DS1302��д�·ݼĴ���88Hд���������·�����BCD��
							   	write_1302(0x8e,0x80);//��д����
								write_1602com(yh+7);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ��������Ҫ����λ
					
								break;
						case 6:nian++;
							 if(nian>=100)
								nian=0;
								Conver_week(nian,yue,ri);
								write_week(week);
								write_nyr(3,nian);//��LCD����ȷ��λ����ʾ"��"�趨�õ��������
					            temp=(nian)/10*16+(nian)%10;//ʮ����ת����DS1302Ҫ���DCB��
							   	write_1302(0x8e,0x00);//����д����ֹд����
							   	write_1302(0x8c,temp);//��DS1302��д��ݼĴ���8cHд���������������BCD��
							   	write_1302(0x8e,0x80);//��д����
								write_1602com(yh+4);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ��������Ҫ����λ
					
								break;
					}
				}
		
			}
			//------------------����dec�����书�ܲ���'�Ӽ�'ע��---------------
			if(dec==0)
			{
				delay(10);//����ʱ��������
				if(dec==0)
				{
			    	buzzer=0;//����������һ��
				    delay(20);
				    buzzer=1;
					while(!dec);
					switch(setn)
					{
						case 1:
							miao--;
							if(miao<0)
								miao=59;//�����ݼ���-1ʱ�Զ����59
							write_sfm(0x06,miao);//��LCD����ȷλ����ʾ�ı���µ�����
				            temp=(miao)/10*16+(miao)%10;//ʮ����ת����DS1302Ҫ���DCB��
						   	write_1302(0x8e,0x00); //����д����ֹд���� 
						   	write_1302(0x80,temp); //��DS1302��д��Ĵ���80Hд��������������BCD��
						   	write_1302(0x8e,0x80); //��д����
							write_1602com(er+7);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ����������д��ԭ����λ��
							//write_1602com(0x0b);
							break;
						case 2:
							fen--;
							if(fen<0)
							fen=59;
							write_sfm(3,fen);
							temp=(fen)/10*16+(fen)%10;//ʮ����ת����DS1302Ҫ���DCB��
						   	write_1302(0x8e,0x00);//����д����ֹд���� 
						   	write_1302(0x82,temp);//��DS1302��д�ּĴ���82Hд�������ķ�����BCD��
						   	write_1302(0x8e,0x80);//��д����
							write_1602com(er+4);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ����������д��ԭ����λ��
							break;
		
						case 3:
							shi--;
					  		if(shi<0)
							shi=23;
							write_sfm(0,shi);
							temp=(shi)/10*16+(shi)%10;//ʮ����ת����DS1302Ҫ���DCB��
						   	write_1302(0x8e,0x00);//����д����ֹд���� 
						   	write_1302(0x84,temp);//��DS1302��дСʱ�Ĵ���84Hд��������Сʱ����BCD��
						   	write_1302(0x8e,0x80);//��д����
							write_1602com(er+1);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ��������Ҫ����λ
							break;
						case 4:
							ri--;
							if(ri<=0)
							ri=31;
							Conver_week(nian,yue,ri);
							write_week(week);
							write_nyr(9,ri);
							temp=(ri)/10*16+(ri)%10;//ʮ����ת����DS1302Ҫ���DCB��
						   	write_1302(0x8e,0x00);//����д����ֹд����
						   	write_1302(0x86,temp);//��DS1302��д���ڼĴ���86Hд����������������BCD��
						   	write_1302(0x8e,0x80);//��д����
							write_1602com(yh+10);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ��������Ҫ����λ		
							break;
						case 5:
							yue--;
							if(yue<=0)
							yue=12;
							Conver_week(nian,yue,ri);
							write_week(week);
							write_nyr(6,yue);
							temp=(yue)/10*16+(yue)%10;//ʮ����ת����DS1302Ҫ���DCB��
						   	write_1302(0x8e,0x00);//����д����ֹд����
						   	write_1302(0x88,temp);//��DS1302��д�·ݼĴ���88Hд���������·�����BCD��
						   	write_1302(0x8e,0x80);//��д����
							write_1602com(yh+7);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ��������Ҫ����λ
				
							break;	
						case 6:
							nian--;
				 			if(nian<0)
							nian=99;
							Conver_week(nian,yue,ri);
							write_week(week);
							write_nyr(3,nian);
				         	temp=(nian)/10*16+(nian)%10;//ʮ����ת����DS1302Ҫ���DCB��
						   	write_1302(0x8e,0x00);//����д����ֹд����
						   	write_1302(0x8c,temp);//��DS1302��д��ݼĴ���8cHд���������������BCD��
						   	write_1302(0x8e,0x80);//��д����
							write_1602com(yh+4);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ��������Ҫ����λ
							break;
									
					}
				}
			}
		}
		if((setNZn!=0)&&(setn==0))
		{
			if(add==0)  //�ϵ���
			{
				delay(10);
				if(add==0)
				{
				    buzzer=0;//����������һ��
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
							write_sfm(10,nz_fen);//��LCD����ȷλ����ʾ"��"�趨�õķ�����
							write_1602com(er+11);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ����������д��ԭ����λ��
							break;
						case 1:
							nz_shi++;
							if(nz_shi>=24)
								nz_shi=0;
							write_sfm(7,nz_shi);//��LCD����ȷ��λ����ʾ"��"�趨�õ�Сʱ����
							write_1602com(er+8);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ��������Ҫ����λ
							break;
					}
				}
		
			}
			//------------------����dec�����书�ܲ���'�Ӽ�'ע��---------------
			if(dec==0)
			{
				delay(10);//����ʱ��������
				if(dec==0)
				{
			    	buzzer=0;//����������һ��
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
							write_1602com(er+11);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ����������д��ԭ����λ��
							break;
		
						case 1:
							nz_shi--;
					  		if(nz_shi<0)
							nz_shi=23;
							write_sfm(7,nz_shi);
							write_1602com(er+8);//��Ϊ����Һ����ģʽ��д�����ݺ�ָ���Զ���һ��������Ҫ����λ
							break;
					}
				}
			}
		}
}

//-------------------------------
void init(void)   //��ʱ�������������ú���
{
	TMOD=0x11; 		//ָ����ʱ/�������Ĺ�����ʽΪ3
	TH0=0; 			//��ʱ��T0�ĸ���λ=0
	TL0=0;  		//��ʱ��T0�ĵ���λ=0
	TH1=0x3C;
	TL1=0xB0;
	EA=1;  			//ϵͳ�����п��ŵ��ж�
	ET0=1; 			//����T0�ж�
	ET1=1;
	IT1=1;
	IT0=0;
	TR0=1; 			//�����жϣ�������ʱ��
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

//*******************������**************************
void main()
{
	P1=0xff;
	lcd_init();      //����Һ������ʼ���Ӻ���
	ds1302_init();   //����DS1302ʱ�ӵĳ�ʼ���Ӻ���
	init();          //���ö�ʱ�������������Ӻ���
    buzzer=1;		 //����������һ��
    delay(100);
    buzzer=0;
	while(1)  //����ѭ���������䣺
	{		
   	 	keyscan();      //���ü���ɨ���Ӻ���		
		if(timerOn==1)
			alarm();	//�������
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
void timer0() interrupt 1  //ȡ�ò���ʾ������ʱ��
{
  //��ȡ��ʱ�����������߸����ݣ�DS1302�Ķ��Ĵ�����д�Ĵ�����һ������
	
    miao = BCD_Decimal(read_1302(0x81));
	fen = BCD_Decimal(read_1302(0x83));
	shi  = BCD_Decimal(read_1302(0x85));
	ri  = BCD_Decimal(read_1302(0x87));
	yue = BCD_Decimal(read_1302(0x89));
	nian=BCD_Decimal(read_1302(0x8d));
    if(T_NL_NZ==1)							//��ʾũ��
	{
		uint nian_temp,temp12;
		temp12=nian;
		nian_temp=2000+(temp12&0xF0)*10+temp12&0x0F;
		if((nian_temp%400==0)||((nian_temp%100!=0)&&(nian_temp%4==0)))  //�ж��Ƿ�Ϊ����
			p_r=1;
		else
			p_r=0;
		Conversion(0,nian,yue,ri);
		write_1602com(er);//ʱ����ʾ�̶�����д��λ��?
		for(a=0;a<16;a++)
		{
			if(p_r==0)
				write_1602dat(nlp[a]);//д��ʾʱ��̶����ţ�����ð��
			else 
				write_1602dat(nlr[a]);
		}

	  	write_nl(3,year_moon);//ũ�� ��
		write_nl(6,month_moon);//ũ�� ��
		write_nl(9,day_moon);//ũ�� ��

		do
			keyscan();
		while(T_NL_NZ==1);

		write_1602com(er);//ʱ����ʾ�̶�����д��λ�ã��ӵ�2��λ�ú�ʼ��ʾ
		for(a=0;a<16;a++)
		{
			write_1602dat(qk[a]);//д��ʾʱ��̶����ţ�����ð��
		}

		write_1602com(er);//ʱ����ʾ�̶�����д��λ�ã��ӵ�2��λ�ú�ʼ��ʾ
		for(a=0;a<8;a++)
		{
			write_1602dat(tab2[a]);//д��ʾʱ��̶����ţ�����ð��
		}
	}

	if(T_NL_NZ==2)								//��ʾ����ʱ�䣬
	{
		write_1602com(er);//ʱ����ʾ�̶�����д��λ��?
		for(a=0;a<16;a++)
			write_1602dat(NZd[a]);//д��ʾʱ��̶����ţ�����ð��

	  	write_sfm(7,nz_shi);//ũ�� ��
		write_sfm(10,nz_fen);//ũ�� ��
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

		write_1602com(er);//ʱ����ʾ�̶�����д��λ�ã��ӵ�2��λ�ú�ʼ��ʾ
		for(a=0;a<16;a++)
		{
			write_1602dat(qk[a]);//д��ʾʱ��̶����ţ�����ð��
		}

		write_1602com(er);//ʱ����ʾ�̶�����д��λ�ã��ӵ�2��λ�ú�ʼ��ʾ
		for(a=0;a<8;a++)
		{
			write_1602dat(tab2[a]);//д��ʾʱ��̶����ţ�����ð��
		}
	}
	

	else
	{	
		//��ʾ�롢ʱ�������ݣ� 
			write_1602com(er+12);
			for(a=0;a<4;a++)
			{
				write_1602dat(tm[a]);
			}
	  	write_sfm(6,miao);//�룬�ӵڶ��е�8���ֺ�ʼ��ʾ������ʱ������ʾ�Ӻ�����
		write_sfm(3,fen);//�֣��ӵڶ��е�5���ַ���ʼ��ʾ
		write_sfm(0,shi);//Сʱ���ӵڶ��е�2���ַ���ʼ��ʾ
	}	
		//��ʾ�ա��¡������ݣ�
		write_nyr(9,ri);//���ڣ��ӵڶ��е�9���ַ���ʼ��ʾ
	   	write_nyr(6,yue);//�·ݣ��ӵڶ��е�6���ַ���ʼ��ʾ
		write_nyr(3,nian);//�꣬�ӵڶ��е�3���ַ���ʼ��ʾ
		Conver_week(nian,yue,ri);
		write_week(week);
}


unsigned char count1;

void timer1() interrupt 3  //ȡ�ò���ʾ������ʱ��
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
