#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "beep.h"
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

const  u8* kbd_menu[15]={"mima"," : ","lock","1","2","3","4","5","6","7","8","9","DEL","0","Enter",};//������
 u8 key;
 u8 err=0;

 int main(void)
 {
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init();					//��ʼ����ʱ����
	uart_init(115200);     				//��ʼ������
	LED_Init();		        			//��ʼ��LED�˿�
	BEEP_Init();         	//��ʼ���������˿�
	KEY_Init();							//��ʼ������
	LCD_Init();							//��ʼ��LCD
  tp_dev.init();			//��ʼ�������� 
	 
	if(!(tp_dev.touchtype&0x80))//����ǵ�����
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
				TP_Adjust();  	 //��ĻУ׼ 
				TP_Save_Adjdata();//����У׼����
				break;				
			}
		}
   }
	
	Virtual_keyboard(0,170,(u8**)kbd_menu);//�����������
	 
	
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

    xReturn=xTaskCreate((TaskFunction_t )LCD_task,             
                (const char*    )"LCD_task",           
                (uint16_t       )LCD_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LCD_TASK_PRIO,        
                (TaskHandle_t*  )&LCDTask_Handler); 
	 
	 xTaskCreate(( TaskFunction_t) led0_task,
                            (const char *) "led0_task",
                            (uint16_t)LED0_STK_SIZE,
                            (void *) NULL,
                            (UBaseType_t) LED0_TASK_PRIO,
                            (TaskHandle_t *) &LED0Task_Handler);
		
		 xTaskCreate(( TaskFunction_t) led1_task,
                            (const char *) "led1_task",
                            (uint16_t)LED1_STK_SIZE,
                            (void *) NULL,
                            (UBaseType_t) LED1_TASK_PRIO,
                            (TaskHandle_t *) &LED1Task_Handler);
		
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

void LCD_task(void *pvParameters)
{
    while (1)
    {
        if (Acquisition_Number()) // ����������ȷ
        {
            Chinese_Show_one(80,120,24,16,0);
            Chinese_Show_one(100,120,26,16,0);
            Chinese_Show_one(120,120,28,16,0);
            Chinese_Show_one(140,120,30,16,0);
            xTaskNotifyGive(LED1Task_Handler); // ��������֪ͨ
        }
        else
        {
            Chinese_Show_one(80,120,32,16,0);
            Chinese_Show_one(100,120,34,16,0);
            Chinese_Show_one(120,120,36,16,0);
            Chinese_Show_one(140,120,38,16,0);

            err++;
            if (err == 3)  // ��� 3 ��
            {
                LCD_ShowString(65, 100, 260, 16, 16, (u8 *)"LOCK 10 seconds");
                xTaskNotifyGive(LED0Task_Handler); // ��������֪ͨ�� LED0 ����

                vTaskDelay(10000 / portTICK_PERIOD_MS); // **�ȴ� 10���Ӻ����**
                err = 0; // **�����������û�����**
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // ���� CPU ����
    }
}



void led0_task(void *p_arg)
{
    while (1)
    {
			int i;
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // �ȴ�֪ͨ��������� 3 �Σ�
       
        for(i=0;i <6;i++) // �÷���������� 3 ��
        {
            BEEP = 0;  // �򿪷��������͵�ƽ������
            vTaskDelay(500 / portTICK_PERIOD_MS); // �� 500ms
            BEEP = 1;  // �رշ�����
            vTaskDelay(500 / portTICK_PERIOD_MS); // ͣ 500ms
        }

        BEEP = 0; // ȷ�����������չر�
        err = 0; // ��λ��������������û�������������

        // **��Ҫʹ�� vTaskSuspend(NULL);������������ȴ��µ�֪ͨ**
    }
}


void led1_task(void *p_arg)
{
    while (1)
    {
			int i=0;
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // �ȴ� LCD �����֪ͨ
        for(i=0;i <5;i++)// LED1 ��˸ 10 ��
        {
            LED0 = !LED0;
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
        LED0 = 1; // LED1 �ر�
    }
}




