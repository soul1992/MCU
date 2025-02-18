#ifndef __BSP_AS608_H
#define __BSP_AS608_H

#include "stm32f10x.h"
#include "delay.h"

//波特率定义...


#define USARTx_IRQn                              USART2_IRQn
#define USARTx_IRQHandler                        USART2_IRQHandler
#define EXTIx_IRQn                               EXTI9_5_IRQn
#define EXTIx_IRQHandler                         EXTI9_5_IRQHandler

#define USARTx_GPIO_CLK_Cmd                      RCC_APB2PeriphClockCmd
#define USARTx_CLK_Cmd                           RCC_APB1PeriphClockCmd
#define RCC_USARTx_GPIO_CLK                      RCC_APB2Periph_GPIOA
#define RCC_USARTx_CLK                           RCC_APB1Periph_USART2
#define USARTx                                   USART2
#define USARTx_TX_GPIO_PORT                      GPIOA
#define USARTx_TX_GPIO_PIN                       GPIO_Pin_2                    //指纹模块Rx接口
#define USARTx_RX_GPIO_PORT                      GPIOA
#define USARTx_RX_GPIO_PIN                       GPIO_Pin_3                    //指纹模块Tx接口

#define EXTIx_CLK_Cmd                            RCC_APB2PeriphClockCmd
#define EXTIx_CLK                                (RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO)
#define EXTIx_GPIO_PORT                          GPIOA
#define EXTIx_GPIO_PIN                           GPIO_Pin_5                    //指纹模块WAK接口
#define EXYIx_GPIOSOURCE_PORT                    GPIO_PortSourceGPIOA
#define EXTIx_GPIOSOURCE_PIN                     GPIO_PinSource5
#define EXTIx_LINE                               EXTI_Line5


void AS608_Config(void);

void PS_Identify(void);


#endif /*__BSP_AS608_H*/
