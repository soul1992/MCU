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
 *������������Ҫ����֧�������΢��phdd2024��ȡ
 *����������֧�ָ��ֵ�Ƭ����ƶ��ƣ�1V1����ָ������ֻ������ƣ������ṩǶ��ʽ�����صļ���ָ���;�ҵָ��
 *����Ƶ�˵��CSDN����:https://blog.csdn.net/weixin_41011452?type=blog
 *�˳����Bվ��Ƶ���� :https://space.bilibili.com/306931401?spm_id_from=333.1007.0.0
 * ����: zs
 * ��������: 2025-02-08
 * 
 * ����:
 * ���ļ�������������룬���ڿ���STM32F103ZET6��Ƭ���հ���ֲ����
 *
 * �汾: 1.0
 * �޸ļ�¼:
 * �汾      �޸���      �޸�����       �޸�����
 * 1.0       [zs]   	 	2025-02-08     		����
 ***********************************************************************/

#define START_TASK_PRIO				1				 	 //�������ȼ�
#define START_STK_SIZE				128				 //�����ջ��С
TaskHandle_t StartTask_Handler;					 //������	
void start_task(void *pvParameters);		 //������


#define LCD_TASK_PRIO		2		//�������ȼ�			
#define LCD_STK_SIZE 		512  	//�����ջ��С	
TaskHandle_t LCDTask_Handler;	//������
void LCD_task(void *pvParameters);	//������

#define LED0_TASK_PRIO				3 				 //�������ȼ�	
#define LED0_STK_SIZE					128				 //�����ջ��С
TaskHandle_t LED0Task_Handler;					 //������	
void led0_task(void *p_arg);		 //������

#define LED1_TASK_PRIO				3 				 //�������ȼ�	
#define LED1_STK_SIZE					128				 //�����ջ��С
TaskHandle_t LED1Task_Handler;					 //������	
void led1_task(void *p_arg);		 //������

EventGroupHandle_t EventGroupHandler;	//�¼���־����

#define EVENTBIT_0	(1<<0)				//�¼�λ
#define EVENTBIT_1	(1<<1)

const  u8* kbd_menu[15]={"mima"," : ","lock","1","2","3","4","5","6","7","8","9","DEL","0","Enter",};//������
 u8 key;
 u8 err=0;

 int main(void)
 {
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init();					//��ʼ����ʱ����
	uart_init(115200);     				//��ʼ������
	LED_Init();		        			//��ʼ��LED�˿�
	KEY_Init();							//��ʼ������
	LCD_Init();							//��ʼ��LCD
  tp_dev.init();			//��ʼ�������� 
	 
	if(!(tp_dev.touchtype&0x80))//����ǵ�����
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
				TP_Adjust();  	 //��ĻУ׼ 
				TP_Save_Adjdata();//����У׼����
				break;				
			}
		}
   }
	
	AS608_load_keyboard(0,170,(u8**)kbd_menu);//�����������
	 
	
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
 
//��ʼ����������
void start_task(void *pvParameters)
{		
		BaseType_t xReturn;
    taskENTER_CRITICAL();           //�����ٽ���

		EventGroupHandler=xEventGroupCreate();
		if(NULL!=EventGroupHandler)
		printf("EventGroupHandler�¼������ɹ�\r\n");
    xReturn=xTaskCreate((TaskFunction_t )LCD_task,             
                (const char*    )"LCD_task",           
                (uint16_t       )LCD_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LCD_TASK_PRIO,        
                (TaskHandle_t*  )&LCDTask_Handler); 
   if(xReturn==pdPASS)
		 printf("LCD_TASK_PRIO���񴴽��ɹ�\r\n");	
	 
	 xTaskCreate(( TaskFunction_t) led0_task,
                            (const char *) "led0_task",
                            (uint16_t)LED0_STK_SIZE,
                            (void *) NULL,
                            (UBaseType_t) LED0_TASK_PRIO,
                            (TaskHandle_t *) &LED0Task_Handler);
		if(xReturn==pdPASS)
			printf("LED0_TASK_PRIO���񴴽��ɹ�\r\n");
		
		 xTaskCreate(( TaskFunction_t) led1_task,
                            (const char *) "led1_task",
                            (uint16_t)LED1_STK_SIZE,
                            (void *) NULL,
                            (UBaseType_t) LED1_TASK_PRIO,
                            (TaskHandle_t *) &LED1Task_Handler);
		if(xReturn==pdPASS)
			printf("LED1_TASK_PRIO���񴴽��ɹ�\r\n");
		
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

// LCD����
void LCD_task(void *pvParameters)
{
    while (1)
    {
        if (GET_NUM()) // ����������ȷ
        {
            printf("����������ȷ\r\n");
            LCD_ShowString(80, 150, 260, 16, 16, (u8 *)"password match");
            xTaskNotifyGive(LED1Task_Handler); // ��������֪ͨ�� LED1 ����
        }
        else
        {
            printf("�����������\r\n");
            LCD_ShowString(80, 150, 260, 16, 16, (u8 *)"password error");
            err++;
            if (err == 3)  // ��� 3 ��
            {
                LCD_ShowString(0, 100, 260, 16, 16, (u8 *)"Lock 3 hours");
                xTaskNotifyGive(LED0Task_Handler); // ��������֪ͨ�� LED0 ����
                vTaskSuspend(LED1Task_Handler);  // ֹͣ LED1 ����
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// LED1 ��˸������ȷ����ʱ��˸��
void led1_task(void *p_arg)
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // �ȴ� LCD �����֪ͨ
        while (1)
        {
            LED1 = !LED1;
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
}

// LED0 ����������� 3 �κ�����
void led0_task(void *p_arg)
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // �ȴ�֪ͨ���յ�֪ͨ���������
        LED0 = 0; // �� LED0 ����
        vTaskSuspend(NULL); // ����������״̬�������ظ�ִ��
    }
}
