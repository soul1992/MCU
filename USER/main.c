//#include "sys.h"
//#include "delay.h"
//#include "FreeRTOS.h"
//#include "task.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include "event_groups.h"
#include "lcd.h"
#include "touch.h"
#include "string.h"
 
/***********************************************************************
 *※※※※※需要技术支持请添加微信phdd2024获取
 *※※※※※支持各种单片机设计定制，1V1答疑指导，不只是做设计，还可提供嵌入式软件相关的技术指导和就业指导
 *此设计的说明CSDN博客:https://blog.csdn.net/weixin_41011452?type=blog
 *此程序的B站视频讲解 :https://space.bilibili.com/306931401?spm_id_from=333.1007.0.0
 * 作者: zs
 * 创建日期: 2025-02-08
 * 
 * 描述:
 * 本文件包含主程序代码，用于控制STM32F103ZET6单片机空白移植工程
 *
 * 版本: 1.0
 * 修改记录:
 * 版本      修改者      修改日期       修改内容
 * 1.0       [zs]   	 	2025-02-08     		创建
 ***********************************************************************/

#define START_TASK_PRIO				1				 	 //任务优先级
#define START_STK_SIZE				128				 //任务堆栈大小
TaskHandle_t StartTask_Handler;					 //任务句柄	
void start_task(void *pvParameters);		 //任务函数


#define LCD_TASK_PRIO		2		//任务优先级			
#define LCD_STK_SIZE 		512  	//任务堆栈大小	
TaskHandle_t LCDTask_Handler;	//任务句柄
void LCD_task(void *pvParameters);	//任务函数

#define LED0_TASK_PRIO				3 				 //任务优先级	
#define LED0_STK_SIZE					128				 //任务堆栈大小
TaskHandle_t LED0Task_Handler;					 //任务句柄	
void led0_task(void *p_arg);		 //任务函数

#define LED1_TASK_PRIO				3 				 //任务优先级	
#define LED1_STK_SIZE					128				 //任务堆栈大小
TaskHandle_t LED1Task_Handler;					 //任务句柄	
void led1_task(void *p_arg);		 //任务函数

EventGroupHandle_t EventGroupHandler;	//事件标志组句柄

#define EVENTBIT_0	(1<<0)				//事件位
#define EVENTBIT_1	(1<<1)

const  u8* kbd_menu[15]={"mima"," : ","lock","1","2","3","4","5","6","7","8","9","DEL","0","Enter",};//按键表
 u8 key;
 u8 err=0;

 int main(void)
 {
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();					//初始化延时函数
	uart_init(115200);     				//初始化串口
	LED_Init();		        			//初始化LED端口
	KEY_Init();							//初始化按键
	LCD_Init();							//初始化LCD
  tp_dev.init();			//初始化触摸屏 
	 
	if(!(tp_dev.touchtype&0x80))//如果是电阻屏
	{
		LCD_ShowString(0,30,200,16,16,(u8 *)"Adjust the LCD ?");
		POINT_COLOR=BLUE;
		LCD_ShowString(0,60,200,16,16,(u8 *)"yes:KEY1 no:KEY0");	
		while(1)
		{		
			key=KEY_Scan(0);
			if(key==KEY0_PRES)
				break;
			if(key==KEY1_PRES)
			{
				LCD_Clear(WHITE);
				TP_Adjust();  	 //屏幕校准 
				TP_Save_Adjdata();//保存校准参数
				break;				
			}
		}
   }
	
	AS608_load_keyboard(0,170,(u8**)kbd_menu);//加载虚拟键盘
	 
	
	Chinese_Show_one(65,0,0,16,0);
	Chinese_Show_one(85,0,2,16,0);
	Chinese_Show_one(105,0,4,16,0);
	Chinese_Show_one(125,0,6,16,0);
	Chinese_Show_one(145,0,8,16,0);
	Chinese_Show_one(165,0,10,16,0);
	 
 xTaskCreate(( TaskFunction_t) start_task,
                            (const char *) "start_task",
                            (uint16_t)START_STK_SIZE,
                            (void *) NULL,
                            (UBaseType_t) START_TASK_PRIO,
                            (TaskHandle_t *) &StartTask_Handler);
	 vTaskStartScheduler();														
		
 }
 
//开始任务任务函数
void start_task(void *pvParameters)
{		
		BaseType_t xReturn;
    taskENTER_CRITICAL();           //进入临界区

		EventGroupHandler=xEventGroupCreate();
		if(NULL!=EventGroupHandler)
		printf("EventGroupHandler事件创建成功\r\n");
    xReturn=xTaskCreate((TaskFunction_t )LCD_task,             
                (const char*    )"LCD_task",           
                (uint16_t       )LCD_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LCD_TASK_PRIO,        
                (TaskHandle_t*  )&LCDTask_Handler); 
   if(xReturn==pdPASS)
		 printf("LCD_TASK_PRIO任务创建成功\r\n");	
	 
	 xTaskCreate(( TaskFunction_t) led0_task,
                            (const char *) "led0_task",
                            (uint16_t)LED0_STK_SIZE,
                            (void *) NULL,
                            (UBaseType_t) LED0_TASK_PRIO,
                            (TaskHandle_t *) &LED0Task_Handler);
		if(xReturn==pdPASS)
			printf("LED0_TASK_PRIO任务创建成功\r\n");
		
		 xTaskCreate(( TaskFunction_t) led1_task,
                            (const char *) "led1_task",
                            (uint16_t)LED1_STK_SIZE,
                            (void *) NULL,
                            (UBaseType_t) LED1_TASK_PRIO,
                            (TaskHandle_t *) &LED1Task_Handler);
		if(xReturn==pdPASS)
			printf("LED1_TASK_PRIO任务创建成功\r\n");
		
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}

volatile uint8_t led_on_flag = 0; // LED任务控制标志

void LCD_task(void * pvParameters)
{
	while(1)
	{
		if(GET_NUM()) // 密码输入正确
		{
			printf("密码输入正确\r\n");
			LCD_ShowString(80,150,260,16,16,(u8 *)"password match"); 
			xEventGroupSetBits(EventGroupHandler, EVENTBIT_0);
			led_on_flag = 1;  // 让LED开始闪烁
		}
		else
		{
			printf("密码输入错误\r\n");
			LCD_ShowString(80,150,260,16,16,(u8 *)"password error");
			err++;
			if(err == 3)
			{
				xEventGroupSetBits(EventGroupHandler, EVENTBIT_1);
				vTaskSuspend(LED1Task_Handler);
				LCD_ShowString(0,100,260,16,16,(u8 *)"Lock 3 hours");
			}
		}					
		vTaskDelay(100 / portTICK_PERIOD_MS); // 延时10ms
	}	
}

void led1_task(void *p_arg)
{
	while(1)
	{
		if(led_on_flag) // 检查标志位是否允许LED闪烁
		{
			LED1 = !LED1; // 反转LED状态
			vTaskDelay(500 / portTICK_PERIOD_MS); // 500ms的闪烁
		}
		else
		{
			vTaskDelay(10 / portTICK_PERIOD_MS); // 任务挂起，防止CPU过载
		}
	}
}

void led0_task(void *p_arg)
{
	while(1)
	{
		if(xEventGroupGetBits(EventGroupHandler) & EVENTBIT_1) // 轮询事件位
		{
			LED0 = 0; // 让 LED0 常亮
		}
		vTaskDelay(10 / portTICK_PERIOD_MS); // 适当延时，防止 CPU 过载
	}
}


