STM32 FreeRTOS 智能门禁项目
本项目基于STM32F103ZET6开发板，使用FreeRTOS实现智能门禁系统。
包含密码验证功能，正确时LED1闪烁，错误3次后 LED0 常亮，并停止LED1任务。
功能介绍
1.密码验证
正确 → LED1闪烁
输错3次 → LED0常亮，LED1停止
2.使用FreeRTOS任务通知：
1)任务间通信高效，不阻塞系统.
2)ulTaskNotifyTake()让任务等待通知，不占CPU.
3)xTaskNotifyGive()触发LED控制.
3.低功耗
1)LED0任务执行后自动挂起，减少CPU占用
2)LED1任务可被暂停/恢复
4.📂 STM32-FreeRTOS-智能门禁
├── 📄 main.c           // 主程序入口
├── 📄 lcd.c            // LCD 显示相关代码
├── 📄 led.c            // LED 控制代码
├── 📄 key.c            // 按键检测
├── 📄 touch.c          // 触摸屏驱动
├── 📄 FreeRTOSConfig.h // FreeRTOS 配置文件
└── 📄 README.md        // 说明文档
5. 硬件要求
STM32F103ZET6
TFT-LCD 屏幕
LED0 (错误 3 次后常亮)
LED1 (正确时闪烁)
按键输入
