
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

#define START_TASK_PRIO				1				 	//�������ȼ�
#define START_STK_SIZE				128				//�����ջ��С
TaskHandle_t StartTask_Handler;					//������	
void start_task(void *pvParameters);		//������


#define LCD_TASK_PRIO		2								//�������ȼ�			
#define LCD_STK_SIZE 		512  						//�����ջ��С	
TaskHandle_t LCDTask_Handler;						//������
void LCD_task(void *pvParameters);			//������

#define AS608_TASK_PRIO		2							//�������ȼ�
#define AS608_STK_SIZE 		512  					//�����ջ��С	
TaskHandle_t AS608Task_Handler;					//������
void AS608_task(void *pvParameters);		//������

#define RFID_TASK_PRIO		2							//�������ȼ�
#define RFID_STK_SIZE 		512						//�����ջ��С	
TaskHandle_t RFIDTask_Handler;					//������
void RFID_task(void *pvParameters);			//������

#define SG90_TASK_PRIO				3				 	//�������ȼ�	
#define SG90_STK_SIZE					128				//�����ջ��С
TaskHandle_t SG90Task_Handler;					//������	
void SG90_task(void *p_arg);		 				//������

const  u8* kbd_menu[15]={"mima"," : ","lock","1","2","3","4","5","6","7","8","9","DEL","0","Enter",};//������
 u8 key;
 u8 err=0;

 int main(void)
 {
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init();					//��ʼ����ʱ����
	uart_init(115200);     				//��ʼ������
	 
	AS608_Config(); 
	 
	KEY_Init();							//��ʼ������
	LCD_Init();							//��ʼ��LCD
	Servo_Init();
  tp_dev.init();			//��ʼ�������� 
	 
	RC522_Init();
	Servo_SetAngle(0);
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
		if(xReturn==pdPASS)
		printf("LCD_task���񴴽��ɹ�\r\n");										
	 
		xReturn=xTaskCreate((TaskFunction_t )AS608_task,             
                (const char*    )"AS608_task",           
                (uint16_t       )AS608_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )AS608_TASK_PRIO,        
                (TaskHandle_t*  )&AS608Task_Handler);
		if(xReturn==pdPASS)
		printf("AS608_task���񴴽��ɹ�\r\n");								

		xReturn=xTaskCreate((TaskFunction_t )RFID_task,             
                (const char*    )"RFID_task",           
                (uint16_t       )RFID_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )RFID_TASK_PRIO,        
                (TaskHandle_t*  )&RFIDTask_Handler); 
		if(xReturn==pdPASS)
		printf("RFID_task���񴴽��ɹ�\r\n");	
								
		xReturn=xTaskCreate(( TaskFunction_t) SG90_task,
                            (const char *) "SG90_task",
                            (uint16_t)SG90_STK_SIZE,
                            (void *) NULL,
                            (UBaseType_t) SG90_TASK_PRIO,
                            (TaskHandle_t *) &SG90Task_Handler);
		if(xReturn==pdPASS)
		printf("SG90_task���񴴽��ɹ�\r\n");
		
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
            xTaskNotifyGive(SG90Task_Handler); // ��������֪ͨ
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
								vTaskSuspend(SG90Task_Handler);

                vTaskDelay(10000 / portTICK_PERIOD_MS); // **�ȴ� 10���Ӻ����**
								LCD_ShowString(65, 100, 260, 16, 16, "                      ");
								LCD_ShowString(65, 120, 260, 16, 16, "                      ");
                err = 0; // **�����������û�����**
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // ���� CPU ����
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
						xTaskNotifyGive(SG90Task_Handler); // ��������֪ͨ
					  Numvalue[9] = 1;
        }
				vTaskDelay(100 / portTICK_PERIOD_MS); // ���� CPU ����
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
	       
				xTaskNotifyGive(SG90Task_Handler); // ��������֪ͨ
				printf("ʶ�𿨺ųɹ�\r\n");
				
			
			}
			else if(shibieka()==0)
			{
						Chinese_Show_two(80,120,16,16,0);
            Chinese_Show_two(96,120,18,16,0);
            Chinese_Show_two(112,120,20,16,0);
						Chinese_Show_two(128,120,12,16,0);
            Chinese_Show_two(144,120,14,16,0);
			  printf("ʶ�𿨺�ʧ��\r\n");
				err++;
					if(err==3)
					{
					  vTaskSuspend(SG90Task_Handler);
						printf("����������\r\n");
						LCD_ShowString(0,100,260,16,16,(u8 *)"Task has been suspended");
						vTaskDelay(2000 / portTICK_PERIOD_MS); // ÿ100ms���һ��
						LCD_ShowString(0,100,260,16,16,(u8 *)"                       ");
						
					}			
			}
			
	    vTaskDelay(100 / portTICK_PERIOD_MS); // ÿ100ms���һ��
	 }
}

void SG90_task(void * pvParameters)
{
    while (1)
    {
        // �ȴ�֪ͨ��ֱ�����յ�����LCD_task��֪ͨ
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // ������ȷ�������ת��180��
        Servo_SetAngle(180);
        vTaskDelay(2000 / portTICK_PERIOD_MS);  // ��ʱ2��

        // �����ת��0��
        Servo_SetAngle(0);
        
        // ���LCD��ʾ
        LCD_ShowString(80, 120, 100, 16, 16, "                      ");
        
        // ��ʱһ��ʱ���Է�ֹ�������Ƶ������
        vTaskDelay(100 / portTICK_PERIOD_MS); // ÿ100ms���һ��
    }
}
