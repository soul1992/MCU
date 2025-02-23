#include "sys.h"
#include "RC522.h"
#include "delay.h"
#include "usart.h"
#include "string.h"



#define   RC522_DELAY() delay_us( 20 )


unsigned char status;
unsigned char ID[4];
unsigned char CT[2];//卡类型0
unsigned char kahao1[4] = {0x43, 0xD4, 0x61, 0x11};
//unsigned char kahao1[4] = {0x47, 0x78, 0x2F, 0x63};
u8  KEY[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
 char cStr[4];



void RC522_Init( void )
{
	SPI1_Init();

	RC522_Reset_Disable();

	RC522_CS_Disable();

	PcdReset();

	M500PcdConfigISOType( 'A' );                                                    /* 设置工作方式 */
}


void SPI1_Init( void )
{
	GPIO_InitTypeDef  GPIO_InitStructure;	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE); //使能端口F时钟
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP; //输出模式
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1|GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_4;//管脚设置
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;//速度为50M
	GPIO_Init(GPIOF,&GPIO_InitStructure); //初始化结构体		     
	GPIO_SetBits(GPIOF,GPIO_Pin_1|GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_4);  //拉高

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 	 //上拉输入			   
	GPIO_Init(GPIOF, &GPIO_InitStructure);
}


/*
 * 函数名：SPI_RC522_SendByte
 * 描述  ：向RC522发送1 Byte 数据
 * 输入  ：byte，要发送的数据
 * 返回  : RC522返回的数据
 * 调用  ：内部调用
 */
void SPI_RC522_SendByte( u8 byte )
{
	u8 counter;

	for ( counter = 0; counter < 8; counter++ )
	{
		if ( byte & 0x80 )
			RC522_MOSI_1();
		else
			RC522_MOSI_0();

		RC522_DELAY();

		RC522_SCK_0();

		RC522_DELAY();

		RC522_SCK_1();

		RC522_DELAY();

		byte <<= 1;
	}
}


/*
 * 函数名：SPI_RC522_ReadByte
 * 描述  ：从RC522发送1 Byte 数据
 * 输入  ：无
 * 返回  : RC522返回的数据
 * 调用  ：内部调用
 */
u8 SPI_RC522_ReadByte( void )
{
	u8	counter;
	u8	SPI_Data;


	for ( counter = 0; counter < 8; counter++ )
	{
		SPI_Data <<= 1;

		RC522_SCK_0();

		RC522_DELAY();

		if ( RC522_MISO_GET() == 1 )
			SPI_Data |= 0x01;

		RC522_DELAY();

		RC522_SCK_1();

		RC522_DELAY();
	}

	return(SPI_Data);
}


/*
 * 函数名：ReadRawRC
 * 描述  ：读RC522寄存器
 * 输入  ：ucAddress，寄存器地址
 * 返回  : 寄存器的当前值
 * 调用  ：内部调用
 */
u8 ReadRawRC( u8 ucAddress )
{
	u8 ucAddr, ucReturn;


	ucAddr = ( (ucAddress << 1) & 0x7E) | 0x80;

	RC522_CS_Enable();

	SPI_RC522_SendByte( ucAddr );

	ucReturn = SPI_RC522_ReadByte();

	RC522_CS_Disable();

	return(ucReturn);
}


/*
 * 函数名：WriteRawRC
 * 描述  ：写RC522寄存器
 * 输入  ：ucAddress，寄存器地址
 *         ucValue，写入寄存器的值
 * 返回  : 无
 * 调用  ：内部调用
 */
void WriteRawRC( u8 ucAddress, u8 ucValue )
{
	u8 ucAddr;

	ucAddr = (ucAddress << 1) & 0x7E;

	RC522_CS_Enable();

	SPI_RC522_SendByte( ucAddr );

	SPI_RC522_SendByte( ucValue );

	RC522_CS_Disable();
}


/*
 * 函数名：SetBitMask
 * 描述  ：对RC522寄存器置位
 * 输入  ：ucReg，寄存器地址
 *         ucMask，置位值
 * 返回  : 无
 * 调用  ：内部调用
 */
void SetBitMask( u8 ucReg, u8 ucMask )
{
	u8 ucTemp;

	ucTemp = ReadRawRC( ucReg );

	WriteRawRC( ucReg, ucTemp | ucMask );      /* set bit mask */
}


/*
 * 函数名：ClearBitMask
 * 描述  ：对RC522寄存器清位
 * 输入  ：ucReg，寄存器地址
 *         ucMask，清位值
 * 返回  : 无
 * 调用  ：内部调用
 */
void ClearBitMask( u8 ucReg, u8 ucMask )
{
	u8 ucTemp;

	ucTemp = ReadRawRC( ucReg );

	WriteRawRC( ucReg, ucTemp & (~ucMask) ); /* clear bit mask */
}


/*
 * 函数名：PcdAntennaOn
 * 描述  ：开启天线
 * 输入  ：无
 * 返回  : 无
 * 调用  ：内部调用
 */
void PcdAntennaOn( void )
{
	u8 uc;

	uc = ReadRawRC( TxControlReg );

	if ( !(uc & 0x03) )
		SetBitMask( TxControlReg, 0x03 );
}


/*
 * 函数名：PcdAntennaOff
 * 描述  ：开启天线
 * 输入  ：无
 * 返回  : 无
 * 调用  ：内部调用
 */
void PcdAntennaOff( void )
{
	ClearBitMask( TxControlReg, 0x03 );
}


/*
 * 函数名：PcdRese
 * 描述  ：复位RC522
 * 输入  ：无
 * 返回  : 无
 * 调用  ：外部调用
 */
void PcdReset( void )
{
	RC522_Reset_Disable();

	delay_us( 1 );

	RC522_Reset_Enable();

	delay_us( 1 );

	RC522_Reset_Disable();

	delay_us( 1 );

	WriteRawRC( CommandReg, 0x0f );

	while ( ReadRawRC( CommandReg ) & 0x10 )
		;

	delay_us( 1 );

	WriteRawRC( ModeReg, 0x3D );            /* 定义发送和接收常用模式 和Mifare卡通讯，CRC初始值0x6363 */

	WriteRawRC( TReloadRegL, 30 );          /* 16位定时器低位 */
	WriteRawRC( TReloadRegH, 0 );           /* 16位定时器高位 */

	WriteRawRC( TModeReg, 0x8D );           /* 定义内部定时器的设置 */

	WriteRawRC( TPrescalerReg, 0x3E );      /* 设置定时器分频系数 */

	WriteRawRC( TxAutoReg, 0x40 );          /* 调制发送信号为100%ASK */
}


/*
 * 函数名：M500PcdConfigISOType
 * 描述  ：设置RC522的工作方式
 * 输入  ：ucType，工作方式
 * 返回  : 无
 * 调用  ：外部调用
 */
void M500PcdConfigISOType( u8 ucType )
{
	if ( ucType == 'A' )                    /* ISO14443_A */
	{
		ClearBitMask( Status2Reg, 0x08 );

		WriteRawRC( ModeReg, 0x3D );    /* 3F */

		WriteRawRC( RxSelReg, 0x86 );   /* 84 */

		WriteRawRC( RFCfgReg, 0x7F );   /* 4F */

		WriteRawRC( TReloadRegL, 30 );  /* tmoLength);// TReloadVal = 'h6a =tmoLength(dec) */

		WriteRawRC( TReloadRegH, 0 );

		WriteRawRC( TModeReg, 0x8D );

		WriteRawRC( TPrescalerReg, 0x3E );

		delay_us( 2 );

		PcdAntennaOn(); /* 开天线 */
	}
}


/*
 * 函数名：PcdComMF522
 * 描述  ：通过RC522和ISO14443卡通讯
 * 输入  ：ucCommand，RC522命令字
 *         pInData，通过RC522发送到卡片的数据
 *         ucInLenByte，发送数据的字节长度
 *         pOutData，接收到的卡片返回数据
 *         pOutLenBit，返回数据的位长度
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：内部调用
 */
char PcdComMF522( u8 ucCommand, u8 * pInData, u8 ucInLenByte, u8 * pOutData, u32 * pOutLenBit )
{
	char	cStatus		= MI_ERR;
	u8	ucIrqEn		= 0x00;
	u8	ucWaitFor	= 0x00;
	u8	ucLastBits;
	u8	ucN;
	u32	ul;

	switch ( ucCommand )
	{
	case PCD_AUTHENT:                                                               /* Mifare认证 */
		ucIrqEn		= 0x12;                                                 /* 允许错误中断请求ErrIEn  允许空闲中断IdleIEn */
		ucWaitFor	= 0x10;                                                 /* 认证寻卡等待时候 查询空闲中断标志位 */
		break;

	case PCD_TRANSCEIVE:                                                            /* 接收发送 发送接收 */
		ucIrqEn		= 0x77;                                                 /* 允许TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn */
		ucWaitFor	= 0x30;                                                 /* 寻卡等待时候 查询接收中断标志位与 空闲中断标志位 */
		break;

	default:
		break;
	}

	WriteRawRC( ComIEnReg, ucIrqEn | 0x80 );                                        /* IRqInv置位管脚IRQ与Status1Reg的IRq位的值相反 */
	ClearBitMask( ComIrqReg, 0x80 );                                                /* Set1该位清零时，CommIRqReg的屏蔽位清零 */
	WriteRawRC( CommandReg, PCD_IDLE );                                             /* 写空闲命令 */
	SetBitMask( FIFOLevelReg, 0x80 );                                               /* 置位FlushBuffer清除内部FIFO的读和写指针以及ErrReg的BufferOvfl标志位被清除 */

	for ( ul = 0; ul < ucInLenByte; ul++ )
		WriteRawRC( FIFODataReg, pInData [ul] );                                /* 写数据进FIFOdata */

	WriteRawRC( CommandReg, ucCommand );                                            /* 写命令 */


	if ( ucCommand == PCD_TRANSCEIVE )
		SetBitMask( BitFramingReg, 0x80 );                                      /* StartSend置位启动数据发送 该位与收发命令使用时才有效 */

	ul = 1000;                                                                      /* 根据时钟频率调整，操作M1卡最大等待时间25ms */

	do                                                                              /* 认证 与寻卡等待时间 */
	{
		ucN = ReadRawRC( ComIrqReg );                                           /* 查询事件中断 */
		ul--;
	}
	while ( (ul != 0) && (!(ucN & 0x01) ) && (!(ucN & ucWaitFor) ) );               /* 退出条件i=0,定时器中断，与写空闲命令 */

	ClearBitMask( BitFramingReg, 0x80 );                                            /* 清理允许StartSend位 */

	if ( ul != 0 )
	{
		if ( !( (ReadRawRC( ErrorReg ) & 0x1B) ) )                              /* 读错误标志寄存器BufferOfI CollErr ParityErr ProtocolErr */
		{
			cStatus = MI_OK;

			if ( ucN & ucIrqEn & 0x01 )                                     /* 是否发生定时器中断 */
				cStatus = MI_NOTAGERR;

			if ( ucCommand == PCD_TRANSCEIVE )
			{
				ucN = ReadRawRC( FIFOLevelReg );                        /* 读FIFO中保存的字节数 */

				ucLastBits = ReadRawRC( ControlReg ) & 0x07;            /* 最后接收到得字节的有效位数 */

				if ( ucLastBits )
					*pOutLenBit = (ucN - 1) * 8 + ucLastBits;       /* N个字节数减去1（最后一个字节）+最后一位的位数 读取到的数据总位数 */
				else
					*pOutLenBit = ucN * 8;                          /* 最后接收到的字节整个字节有效 */

				if ( ucN == 0 )
					ucN = 1;

				if ( ucN > MAXRLEN )
					ucN = MAXRLEN;

				for ( ul = 0; ul < ucN; ul++ )
					pOutData [ul] = ReadRawRC( FIFODataReg );
			}
		}else
			cStatus = MI_ERR;
/*			printf(ErrorReg); */
	}

	SetBitMask( ControlReg, 0x80 );        /* stop timer now */
	WriteRawRC( CommandReg, PCD_IDLE );

	return(cStatus);
}


/*
 * 函数名：PcdRequest
 * 描述  ：寻卡
 * 输入  ：ucReq_code，寻卡方式
 *                     = 0x52，寻感应区内所有符合14443A标准的卡
 *                     = 0x26，寻未进入休眠状态的卡
 *         pTagType，卡片类型代码
 *                   = 0x4400，Mifare_UltraLight
 *                   = 0x0400，Mifare_One(S50)
 *                   = 0x0200，Mifare_One(S70)
 *                   = 0x0800，Mifare_Pro(X))
 *                   = 0x4403，Mifare_DESFire
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdRequest( u8 ucReq_code, u8 * pTagType )
{
	char	cStatus;
	u8	ucComMF522Buf [MAXRLEN];
	u32	ulLen;

	ClearBitMask( Status2Reg, 0x08 );                                                       /* 清理指示MIFARECyptol单元接通以及所有卡的数据通信被加密的情况 */
	WriteRawRC( BitFramingReg, 0x07 );                                                      /*	发送的最后一个字节的 七位 */
	SetBitMask( TxControlReg, 0x03 );                                                       /* TX1,TX2管脚的输出信号传递经发送调制的13.56的能量载波信号 */

	ucComMF522Buf [0] = ucReq_code;                                                         /* 存入 卡片命令字 */

	cStatus = PcdComMF522( PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &ulLen );       /* 寻卡 */

	if ( (cStatus == MI_OK) && (ulLen == 0x10) )                                            /* 寻卡成功返回卡类型 */
	{
		*pTagType	= ucComMF522Buf [0];
		*(pTagType + 1) = ucComMF522Buf [1];
	}else
		cStatus = MI_ERR;

	return(cStatus);
}


/*
 * 函数名：PcdAnticoll
 * 描述  ：防冲撞
 * 输入  ：pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdAnticoll( u8 * pSnr )
{
	char	cStatus;
	u8	uc, ucSnr_check = 0;
	u8	ucComMF522Buf [MAXRLEN];
	u32	ulLen;

	ClearBitMask( Status2Reg, 0x08 );                                                       /* 清MFCryptol On位 只有成功执行MFAuthent命令后，该位才能置位 */
	WriteRawRC( BitFramingReg, 0x00 );                                                      /* 清理寄存器 停止收发 */
	ClearBitMask( CollReg, 0x80 );                                                          /* 清ValuesAfterColl所有接收的位在冲突后被清除 */

	ucComMF522Buf [0]	= 0x93;                                                         /* 卡片防冲突命令 */
	ucComMF522Buf [1]	= 0x20;

	cStatus = PcdComMF522( PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &ulLen );       /* 与卡片通信 */

	if ( cStatus == MI_OK )                                                                 /* 通信成功 */
	{
		for ( uc = 0; uc < 4; uc++ )
		{
			*(pSnr + uc)	= ucComMF522Buf [uc];                                   /* 读出UID */
			ucSnr_check	^= ucComMF522Buf [uc];
		}

		if ( ucSnr_check != ucComMF522Buf [uc] )
			cStatus = MI_ERR;
	}

	SetBitMask( CollReg, 0x80 );

	return(cStatus);
}


/*
 * 函数名：CalulateCRC
 * 描述  ：用RC522计算CRC16
 * 输入  ：pIndata，计算CRC16的数组
 *         ucLen，计算CRC16的数组字节长度
 *         pOutData，存放计算结果存放的首地址
 * 返回  : 无
 * 调用  ：内部调用
 */
void CalulateCRC( u8 * pIndata, u8 ucLen, u8 * pOutData )
{
	u8 uc, ucN;

	ClearBitMask( DivIrqReg, 0x04 );

	WriteRawRC( CommandReg, PCD_IDLE );

	SetBitMask( FIFOLevelReg, 0x80 );

	for ( uc = 0; uc < ucLen; uc++ )
		WriteRawRC( FIFODataReg, *(pIndata + uc) );

	WriteRawRC( CommandReg, PCD_CALCCRC );

	uc = 0xFF;

	do
	{
		ucN = ReadRawRC( DivIrqReg );
		uc--;
	}
	while ( (uc != 0) && !(ucN & 0x04) );

	pOutData [0]	= ReadRawRC( CRCResultRegL );
	pOutData [1]	= ReadRawRC( CRCResultRegM );
}


/*
 * 函数名：PcdSelect
 * 描述  ：选定卡片
 * 输入  ：pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdSelect( u8 * pSnr )
{
	char	ucN;
	u8	uc;
	u8	ucComMF522Buf [MAXRLEN];
	u32	ulLen;

	ucComMF522Buf [0]	= PICC_ANTICOLL1;
	ucComMF522Buf [1]	= 0x70;
	ucComMF522Buf [6]	= 0;

	for ( uc = 0; uc < 4; uc++ )
	{
		ucComMF522Buf [uc + 2]	= *(pSnr + uc);
		ucComMF522Buf [6]	^= *(pSnr + uc);
	}

	CalulateCRC( ucComMF522Buf, 8, &ucComMF522Buf [8] );

	ClearBitMask( Status2Reg, 0x08 );

	ucN = PcdComMF522( PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &ulLen );

	if ( (ucN == MI_OK) && (ulLen == 0x18) )
		ucN = MI_OK;
	else
		ucN = MI_ERR;

	return(ucN);
}


/*
 * 函数名：PcdAuthState
 * 描述  ：验证卡片密码
 * 输入  ：ucAuth_mode，密码验证模式
 *                     = 0x60，验证A密钥
 *                     = 0x61，验证B密钥
 *         u8 ucAddr，块地址
 *         pKey，密码
 *         pSnr，卡片序列号，4字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdAuthState( u8 ucAuth_mode, u8 ucAddr, u8 * pKey, u8 * pSnr )
{
	char	cStatus;
	u8	uc, ucComMF522Buf [MAXRLEN];
	u32	ulLen;

	ucComMF522Buf [0]	= ucAuth_mode;
	ucComMF522Buf [1]	= ucAddr;

	for ( uc = 0; uc < 6; uc++ )
		ucComMF522Buf [uc + 2] = *(pKey + uc);

	for ( uc = 0; uc < 6; uc++ )
		ucComMF522Buf [uc + 8] = *(pSnr + uc);

	cStatus = PcdComMF522( PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &ulLen );

	if ( (cStatus != MI_OK) || (!(ReadRawRC( Status2Reg ) & 0x08) ) )

	{
/*
 *              if(cStatus != MI_OK)
 *                              printf("666")	;
 *              else
 *                      printf("888");
 */
		cStatus = MI_ERR;
	}

	return(cStatus);
}


/*
 * 函数名：PcdWrite
 * 描述  ：写M1卡一块数据
 * 输入  ：u8 ucAddr，块地址
 *         pData，写入的数据，16字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdWrite( u8 ucAddr, u8 * pData )
{
	char	cStatus;
	u8	uc, ucComMF522Buf [MAXRLEN];
	u32	ulLen;

	ucComMF522Buf [0]	= PICC_WRITE;
	ucComMF522Buf [1]	= ucAddr;

	CalulateCRC( ucComMF522Buf, 2, &ucComMF522Buf [2] );

	cStatus = PcdComMF522( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &ulLen );

	if ( (cStatus != MI_OK) || (ulLen != 4) || ( (ucComMF522Buf [0] & 0x0F) != 0x0A) )
		cStatus = MI_ERR;

	if ( cStatus == MI_OK )
	{
		memcpy( ucComMF522Buf, pData, 16 );
		for ( uc = 0; uc < 16; uc++ )
			ucComMF522Buf [uc] = *(pData + uc);

		CalulateCRC( ucComMF522Buf, 16, &ucComMF522Buf [16] );

		cStatus = PcdComMF522( PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, &ulLen );

		if ( (cStatus != MI_OK) || (ulLen != 4) || ( (ucComMF522Buf [0] & 0x0F) != 0x0A) )
			cStatus = MI_ERR;
	}
	return(cStatus);
}


/*
 * 函数名：PcdRead
 * 描述  ：读取M1卡一块数据
 * 输入  ：u8 ucAddr，块地址
 *         pData，读出的数据，16字节
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdRead( u8 ucAddr, u8 * pData )
{
	char	cStatus;
	u8	uc, ucComMF522Buf [MAXRLEN];
	u32	ulLen;

	ucComMF522Buf [0]	= PICC_READ;
	ucComMF522Buf [1]	= ucAddr;

	CalulateCRC( ucComMF522Buf, 2, &ucComMF522Buf [2] );

	cStatus = PcdComMF522( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &ulLen );

	if ( (cStatus == MI_OK) && (ulLen == 0x90) )
	{
		for ( uc = 0; uc < 16; uc++ )
			*(pData + uc) = ucComMF522Buf [uc];
	}else
		cStatus = MI_ERR;

	return(cStatus);
}


/*
 * 函数名：PcdHalt
 * 描述  ：命令卡片进入休眠状态
 * 输入  ：无
 * 返回  : 状态值
 *         = MI_OK，成功
 * 调用  ：外部调用
 */
char PcdHalt( void )
{
	u8	ucComMF522Buf [MAXRLEN];
	u32	ulLen;

	ucComMF522Buf [0]	= PICC_HALT;
	ucComMF522Buf [1]	= 0;

	CalulateCRC( ucComMF522Buf, 2, &ucComMF522Buf [2] );
	PcdComMF522( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &ulLen );

	return(MI_OK);
}


void IC_CMT( u8 * UID, u8 * KEY, u8 RW, u8 * Dat )
{
	u8 ucArray_ID [4] = { 0 };              /* 先后存放IC卡的类型和UID(IC卡序列号) */

	PcdRequest( 0x52, ucArray_ID );         /* 寻卡 */

	PcdAnticoll( ucArray_ID );              /* 防冲撞 */

	PcdSelect( UID );                       /*选定卡 */

	PcdAuthState( 0x60, 0x10, KEY, UID );   /* 校验 */

	if ( RW )                               /* 读写选择，1是读，0是写 */
		PcdRead( 0x10, Dat );

	else
		PcdWrite( 0x10, Dat );

	PcdHalt();
}



void ShowID(unsigned char *p)	 //显示卡的卡号，以十六进制显示
{
    u8 num[9];
    u8 i;

    for(i=0; i<4; i++)
    {
        num[i*2]=p[i]/16;
        num[i*2]>9?(num[i*2]+='7'):(num[i*2]+='0');
        num[i*2+1]=p[i]%16;
        num[i*2+1]>9?(num[i*2+1]+='7'):(num[i*2+1]+='0');
    }
    num[8]=0;
    printf("ID>>>%s\r\n", num);
}

u8 shibieka(void)
{
	while(1)
	{
	    status = PcdRequest( PICC_REQALL, CT );                /* 寻卡 */
     if ( status != MI_OK )                                  /* 寻卡成功 */
     {
      
      continue;
     }
     status = PcdAnticoll( ID );                             /* 防冲撞 */
     
     status = PcdSelect( ID );                               /*选定卡片 */
     if ( status != MI_OK )                                  /*选定卡片成功 */
     { 
        
    if(ID[0]==kahao1[ 0 ]&&ID[ 1 ]==kahao1[ 1 ]&&ID[ 2 ]==kahao1[ 2 ]&&ID[ 3 ]==kahao1[ 3 ])
		{
		  printf("ok\r\n");
			
			delay_ms(1000);
			return 1;
		}
		else
			printf("no\r\n");
		  
			delay_ms(1000);
		  return 0;
		}
		
	}
 
}

