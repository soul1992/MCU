
#include "sys.h"
#include "delay.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"

#include "usart.h"

#include "key.h"
#include "lcd.h"
#include "touch.h"

#include "PWM.h"
#include "Servo.h"
#include "bsp_as608.h"
#include "RC522.h"
 
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

#define START_TASK_PRIO				1				 	//任务优先级
#define START_STK_SIZE				128				//任务堆栈大小
TaskHandle_t StartTask_Handler;					//任务句柄	
void start_task(void *pvParameters);		//任务函数


#define LCD_TASK_PRIO		2								//任务优先级			
#define LCD_STK_SIZE 		512  						//任务堆栈大小	
TaskHandle_t LCDTask_Handler;						//任务句柄
void LCD_task(void *pvParameters);			//任务函数

#define AS608_TASK_PRIO		2							//任务优先级
#define AS608_STK_SIZE 		512  					//任务堆栈大小	
TaskHandle_t AS608Task_Handler;					//任务句柄
void AS608_task(void *pvParameters);		//任务函数

#define RFID_TASK_PRIO		2							//任务优先级
#define RFID_STK_SIZE 		512						//任务堆栈大小	
TaskHandle_t RFIDTask_Handler;					//任务句柄
void RFID_task(void *pvParameters);			//任务函数

#define SG90_TASK_PRIO				3				 	//任务优先级	
#define SG90_STK_SIZE					128				//任务堆栈大小
TaskHandle_t SG90Task_Handler;					//任务句柄	
void SG90_task(void *p_arg);		 				//任务函数

const  u8* kbd_menu[15]={"mima"," : ","lock","1","2","3","4","5","6","7","8","9","DEL","0","Enter",};//按键表
 u8 key;
 u8 err=0;

 int main(void)
 {
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();					//初始化延时函数
	uart_init(115200);     				//初始化串口
	 
	AS608_Config(); 
	 
	KEY_Init();							//初始化按键
	LCD_Init();							//初始化LCD
	Servo_Init();
  tp_dev.init();			//初始化触摸屏 
	 
	RC522_Init();
	Servo_SetAngle(0);
	if(!(tp_dev.touchtype&0x80))//如果是电阻屏
	{
		Chinese_Show_one(60,30,12,16,0);
		Chinese_Show_one(80,30,14,16,0);
		Chinese_Show_one(100,30,16,16,0);
		Chinese_Show_one(120,30,18,16,0);
		Chinese_Show_one(140,30,20,16,0);
		Chinese_Show_one(160,30,22,16,0);
		POINT_COLOR=GREEN;
		LCD_ShowString(60,45,200,16,16,(u8 *)"YES:Plese Press K1");	
		LCD_ShowString(60,65,200,16,16,(u8 *)"N O:Plese Press K0");	
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
	
	Virtual_keyboard(0,170,(u8**)kbd_menu);//加载虚拟键盘
	 
	
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

    xReturn=xTaskCreate((TaskFunction_t )LCD_task,             
                (const char*    )"LCD_task",           
                (uint16_t       )LCD_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LCD_TASK_PRIO,        
                (TaskHandle_t*  )&LCDTask_Handler);
		if(xReturn==pdPASS)
		printf("LCD_task任务创建成功\r\n");										
	 
		xReturn=xTaskCreate((TaskFunction_t )AS608_task,             
                (const char*    )"AS608_task",           
                (uint16_t       )AS608_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )AS608_TASK_PRIO,        
                (TaskHandle_t*  )&AS608Task_Handler);
		if(xReturn==pdPASS)
		printf("AS608_task任务创建成功\r\n");								

		xReturn=xTaskCreate((TaskFunction_t )RFID_task,             
                (const char*    )"RFID_task",           
                (uint16_t       )RFID_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )RFID_TASK_PRIO,        
                (TaskHandle_t*  )&RFIDTask_Handler); 
		if(xReturn==pdPASS)
		printf("RFID_task任务创建成功\r\n");	
								
		xReturn=xTaskCreate(( TaskFunction_t) SG90_task,
                            (const char *) "SG90_task",
                            (uint16_t)SG90_STK_SIZE,
                            (void *) NULL,
                            (UBaseType_t) SG90_TASK_PRIO,
                            (TaskHandle_t *) &SG90Task_Handler);
		if(xReturn==pdPASS)
		printf("SG90_task任务创建成功\r\n");
		
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}

void LCD_task(void *pvParameters)
{
    while (1)
    {
        if (Acquisition_Number()) // 密码输入正确
        {
            Chinese_Show_one(80,120,24,16,0);
            Chinese_Show_one(100,120,26,16,0);
            Chinese_Show_one(120,120,28,16,0);
            Chinese_Show_one(140,120,30,16,0);
            xTaskNotifyGive(SG90Task_Handler); // 发送任务通知
        }
        else
        {
            Chinese_Show_one(80,120,32,16,0);
            Chinese_Show_one(100,120,34,16,0);
            Chinese_Show_one(120,120,36,16,0);
            Chinese_Show_one(140,120,38,16,0);

            err++;
            if (err == 3)  // 输错 3 次
            {
                LCD_ShowString(65, 100, 260, 16, 16, (u8 *)"LOCK 10 seconds");
								vTaskSuspend(SG90Task_Handler);

                vTaskDelay(10000 / portTICK_PERIOD_MS); // **等待 10秒钟后解锁**
								LCD_ShowString(65, 100, 260, 16, 16, "                      ");
								LCD_ShowString(65, 120, 260, 16, 16, "                      ");
                err = 0; // **解锁后允许用户输入**
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // 避免 CPU 过载
    }
}


void AS608_task(void *pvParameters)
{
  while(1)
	{
		extern uint8_t Numvalue[];
		if((Numvalue[9] == 0) && (Numvalue[5] != 0))
        {
					  Chinese_Show_two(80,120,0,16,0);
            Chinese_Show_two(96,120,2,16,0);
            Chinese_Show_two(112,120,4,16,0);
            Chinese_Show_two(128,120,6,16,0);
					  Chinese_Show_two(144,120,8,16,0);
            Chinese_Show_two(160,120,10,16,0);
						xTaskNotifyGive(SG90Task_Handler); // 发送任务通知
					  Numvalue[9] = 1;
        }
				vTaskDelay(100 / portTICK_PERIOD_MS); // 避免 CPU 过载
		}		
}

void RFID_task(void * pvParameters)
{
	
   while(1)
	 {
	    if(shibieka())
			{
				 
						Chinese_Show_two(80,120,16,16,0);
            Chinese_Show_two(96,120,18,16,0);
            Chinese_Show_two(112,120,20,16,0);
					  Chinese_Show_two(128,120,8,16,0);
            Chinese_Show_two(144,120,10,16,0);
	       
				xTaskNotifyGive(SG90Task_Handler); // 发送任务通知
				printf("识别卡号成功\r\n");
				
			
			}
			else if(shibieka()==0)
			{
						Chinese_Show_two(80,120,16,16,0);
            Chinese_Show_two(96,120,18,16,0);
            Chinese_Show_two(112,120,20,16,0);
						Chinese_Show_two(128,120,12,16,0);
            Chinese_Show_two(144,120,14,16,0);
			  printf("识别卡号失败\r\n");
				err++;
					if(err==3)
					{
					  vTaskSuspend(SG90Task_Handler);
						printf("舵机任务挂起\r\n");
						LCD_ShowString(0,100,260,16,16,(u8 *)"Task has been suspended");
						vTaskDelay(2000 / portTICK_PERIOD_MS); // 每100ms检查一次
						LCD_ShowString(0,100,260,16,16,(u8 *)"                       ");
						
					}			
			}
			
	    vTaskDelay(100 / portTICK_PERIOD_MS); // 每100ms检查一次
	 }
}

void SG90_task(void * pvParameters)
{
    while (1)
    {
        // 等待通知，直到接收到来自LCD_task的通知
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // 密码正确，舵机旋转到180度
        Servo_SetAngle(180);
        vTaskDelay(2000 / portTICK_PERIOD_MS);  // 延时2秒

        // 舵机旋转回0度
        Servo_SetAngle(0);
        
        // 清空LCD显示
        LCD_ShowString(80, 120, 100, 16, 16, "                      ");
        
        // 延时一定时间以防止任务过度频繁运行
        vTaskDelay(100 / portTICK_PERIOD_MS); // 每100ms检查一次
    }
}
