#include "bsp_as608.h"

uint32_t AS608_Addr = 0xFFFFFFFF;             /*指纹识别模块默认地址*/

static void AS608_SendData(uint8_t data);
static void AS608_PackHead(void);
static void SendFlag(uint8_t flag);
static void SendLength(uint16_t length);
static void Sendcmd(uint8_t cmd);
static void SendCheck(uint16_t check);

static void AS608_NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* 嵌套向量中断控制器组选择 */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* 配置USART为中断源 */
  NVIC_InitStructure.NVIC_IRQChannel = USARTx_IRQn;
  /* 抢断优先级*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  /* 子优先级 */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  /* 使能中断 */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* 初始化配置NVIC */
  NVIC_Init(&NVIC_InitStructure);
    
  /* 配置中断源：TouchOut线 */
  NVIC_InitStructure.NVIC_IRQChannel = EXTIx_IRQn;
    /* 抢断优先级*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  /* 配置子优先级 */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  //使能
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* 初始化配置NVIC */
  NVIC_Init(&NVIC_InitStructure); 
}

static void AS608_USART_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructuer;
    USART_InitTypeDef USART_InitStructuer;
    
    USARTx_GPIO_CLK_Cmd(RCC_USARTx_GPIO_CLK,ENABLE);
    USARTx_CLK_Cmd(RCC_USARTx_CLK,ENABLE);
    
    GPIO_InitStructuer.GPIO_Pin = USARTx_TX_GPIO_PIN;
    GPIO_InitStructuer.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructuer.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(USARTx_TX_GPIO_PORT,&GPIO_InitStructuer);
    
    GPIO_InitStructuer.GPIO_Pin = USARTx_RX_GPIO_PIN;
    GPIO_InitStructuer.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(USARTx_RX_GPIO_PORT,&GPIO_InitStructuer);
    
    USART_InitStructuer.USART_BaudRate = 57600;
    USART_InitStructuer.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructuer.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
    USART_InitStructuer.USART_Parity = USART_Parity_No;
    USART_InitStructuer.USART_StopBits = USART_StopBits_1;
    USART_InitStructuer.USART_WordLength = USART_WordLength_8b;
    USART_Init(USARTx,&USART_InitStructuer);
    
    USART_ITConfig(USARTx,USART_IT_RXNE,ENABLE);
    
    USART_Cmd(USARTx,ENABLE);
}

static void EXTI_WEAK_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructuer;
    EXTI_InitTypeDef EXTI_InitStructuer;
    
    EXTIx_CLK_Cmd(EXTIx_CLK,ENABLE);
    
    GPIO_InitStructuer.GPIO_Pin = EXTIx_GPIO_PIN;
    GPIO_InitStructuer.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(EXTIx_GPIO_PORT,&GPIO_InitStructuer);
    
    GPIO_EXTILineConfig(EXYIx_GPIOSOURCE_PORT,EXTIx_GPIOSOURCE_PIN);
    
    EXTI_InitStructuer.EXTI_Line = EXTIx_LINE;
    EXTI_InitStructuer.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructuer.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructuer.EXTI_LineCmd  = ENABLE;
    EXTI_Init(&EXTI_InitStructuer);
}

void AS608_Config(void)
{
  AS608_NVIC_Config();
	AS608_USART_Config();
 	EXTI_WEAK_GPIO_Config();
}

//串口发送字节函数
static void AS608_SendData(uint8_t data)
{
  USART_SendData(USARTx,data);
  while(USART_GetFlagStatus(USARTx,USART_FLAG_TXE) == RESET);
}

//发送命令包头格式函数
static void AS608_PackHead(void)
{ 
  /*包头*/
  AS608_SendData(0xEF);
  AS608_SendData(0x01);	
  
  /*指纹模块地址*/
  AS608_SendData(AS608_Addr>>24);
  AS608_SendData(AS608_Addr>>16);	
  AS608_SendData(AS608_Addr>>8);
  AS608_SendData(AS608_Addr);	
}

//发送包标识
static void SendFlag(uint8_t flag)
{
  AS608_SendData(flag);
}

//发送包长度
static void SendLength(uint16_t length)
{
	AS608_SendData(length>>8);
  AS608_SendData(length);
}

//发送指令码
static void Sendcmd(uint8_t cmd)
{
	AS608_SendData(cmd);
}

//发送校验和
static void SendCheck(uint16_t check)
{
	AS608_SendData(check>>8);
	AS608_SendData(check);
}


void PS_Identify(void)
{
    uint16_t temp;

    AS608_PackHead();
    SendFlag(0x01);
    SendLength(0x03);
    Sendcmd(0x11);                
    temp = 0x01+0x03+0x11;
    SendCheck(temp);

    delay_ms(500);
}

