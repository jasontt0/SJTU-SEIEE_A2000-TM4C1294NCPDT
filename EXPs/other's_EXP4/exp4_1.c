//*****************************************************************************
//
// Copyright: 2020-2021, 上海交通大学电子工程系实验教学中心
// File name: exp2_0.c
// Description: 
//    1.开机或复位后，底板上右边4位数码管自动显示计时数值，最低位对应单位是0.1秒；
//    2.开机或复位后，底板上8个LED灯以跑马灯形式由左向右循环变换，约0.5秒变换1次；
//    3.当没有按键按下时，从左边数第二位数码管显示“0”；
//      当人工按下某键，数码管显示该键的编号；
//      此刻四位计时数码管暂停变化，停止计时，直到放开按键后自动继续计时。
// Author:	上海交通大学电子工程系实验教学中心
// Version: 1.0.0.20201228 
// Date：2020-12-28
// History：
//
//*****************************************************************************

//*****************************************************************************
//
// 头文件
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // 基址宏定义
#include "inc/hw_types.h"         // 数据类型宏定义，寄存器访问函数
#include "driverlib/debug.h"      // 调试用
#include "driverlib/gpio.h"       // 通用IO口宏定义
#include "driverlib/pin_map.h"    // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"	  // 系统控制定义
#include "driverlib/systick.h"    // SysTick Driver 原型
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver 原型

#include "tm1638.h"               // 与控制TM1638芯片有关的函数
#include "JLX12864.h"

//*****************************************************************************
//
// 宏定义
//
//*****************************************************************************
#define SYSTICK_FREQUENCY		50		// SysTick频率为50Hz，即循环定时周期20ms

#define V_T100ms	5              // 0.1s软件定时器溢出值，5个20ms
#define V_T500ms	25            // 0.5s软件定时器溢出值，25个20ms
#define V_T2s 100                 // 2s软件定时器溢出值，100个20ms

//*****************************************************************************
//
// 函数原型声明
//
//*****************************************************************************
void GPIOInit(void);        // GPIO初始化
void SysTickInit(void);     // 设置SysTick中断 
void DevicesInit(void);     // MCU器件初始化，注：会调用上述函数
//*****************************************************************************
//
// 变量定义
//
//*****************************************************************************

// 软件定时器计数
uint8_t clock100ms = 0;
uint8_t clock500ms = 0;
uint8_t clock2s = 0;

//LCD屏幕状态机
uint8_t ui_state = 0;

// 软件定时器溢出标志
uint8_t clock100ms_flag = 0;
uint8_t clock500ms_flag = 0;
uint8_t clock2s_flag = 0;

// 测试用计数器
uint32_t test_counter = 0;

// 8位数码管显示的数字或字母符号，先定义板上从左到右第1个(下标4)和第3个数码位(下标6)中显示下划线
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t digit[8]={' ',' ',' ',' ','_',' ','_',' '};

// 8位小数点 1亮  0灭。先使板上从右到左第2个数码位小数点亮，注意该16进制数从低位到高位下标为0~7
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t pnt = 0x04;

// 8个LED指示灯状态，0灭，1亮
// 注：板上指示灯从左到右序号排列为7、6、5、4、3、2、1、0
//     对应元件LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
uint8_t led[] = {1, 1, 1, 1, 1, 1, 1, 0};

// 当前按键值
uint8_t key_code = 0;

// 系统时钟频率 
uint32_t ui32SysClock;

//*****************************************************************************
//
// 主程序
//
//*****************************************************************************
 int main(void)
{
	uint8_t temp,i;

	DevicesInit();            //  MCU器件初始化
	
	while (clock100ms < 3);   // 延时>60ms,等待TM1638上电完成
	TM1638_Init();	          // 初始化TM1638
	
	initial_lcd();//LCD初始化
	
	while (1)
	{				
		if (clock100ms_flag == 1)      // 检查0.1秒定时是否到
		{
			clock100ms_flag		= 0;
			// 每0.1秒累加计时值在数码管上以十进制显示，有键按下(key_code!=0)时暂停计时(即暂停更新test_counter和digit[0~3])
			if (key_code == 0)
			{
				if (++test_counter >= 10000) test_counter = 0;//注意test_counter加到10000时对应的总计时为1000.0s，此时test_counter清零重新计时
				digit[0] = test_counter / 1000; 	    // 计算百位数
				digit[1] = test_counter / 100 % 10; 	// 计算十位数
				digit[2] = test_counter / 10 % 10; 	  // 计算个位数
				digit[3] = test_counter % 10;         // 计算十分位数
			}
		}

		if (clock500ms_flag == 1)   // 检查0.5秒定时是否到
		{
			clock500ms_flag = 0;
			// 8个指示灯以走马灯方式，每0.5秒向右（循环）移动一格(对应唯一不亮的那个灯向右(循环)移动一格)
			temp = led[0];
			for (i = 0; i < 7; i++) led[i] = led[i + 1];
			led[7] = temp;
		}
		
		if (clock2s_flag)
		{
			 clock2s_flag = 0;
			 switch( ui_state )
			 {
				 case 0:
             clear_screen(); //clear all dots
             display_128x64(bmp1);
				     break;
       //LCD_delay(86000);
				 case 1:
					   clear_screen();
             display_GB2312_string(1,1,(unsigned char*)"12864,带中文字库",1); //在第 1 页，第 1 列，显示一串 16x16 点阵汉字或 8x16 的 ASCII 字
             display_GB2312_string(3,1,(unsigned char*)"16X16 简体汉字库,",0); //显示一串 16x16 点阵汉字或 8x16 的 ASCII 字.以下雷同
             display_GB2312_string(5,1,(unsigned char*)"或 8X16 点阵 ASCII,",1);
             display_GB2312_string(7,1,(unsigned char*)"或 5x8 点阵 ASCII 码",0);
				     break;
       //LCD_delay(86000);
				 case 2:
             clear_screen();
             display_GB2312_string(1,1,(unsigned char*)"晶联讯成立于二零",1);
		         display_GB2312_string(3,1,(unsigned char*)"零四年十一月七日",0);
             display_GB2312_string(5,1,(unsigned char*)"主要生产液晶模块",1);
             display_GB2312_string(7,1,(unsigned char*)"品质至上真诚服务",0);
				     break;
       //LCD_delay(86000);
				 case 3:
             display_GB2312_string(1,1,(unsigned char*)"GB2312 简体字库及",1);
             display_GB2312_string(3,1,(unsigned char*)"有图型功能，可自",0);
             display_GB2312_string(5,1,(unsigned char*)"编大字或图像或生",1);
             display_GB2312_string(7,1,(unsigned char*)"僻字，例如： ",0);
             display_graphic_16x16(7,97,jiong1); //在第 7 页，第 81 列显示单个自编生僻汉字“囧”
             display_graphic_16x16(7,113,lei1); //显示单个自编生僻汉字"畾“
				     break;
       //LCD_delay(86000);
				 case 4:
             clear_screen();
             display_GB2312_string(1,1,(unsigned char*)"<!@#$%^&*()_-+]/",0); //在第 1 页，第 1 列，显示一串 16x16 点阵汉字或 8*16 的 ASCII 字
             display_string_5x8(3,1,(unsigned char*)"<!@#$%^&*()_-+]/;.,?[",0);//在第 3 页，第 1 列，显示一串 5x8 点阵的 ASCII 字
             display_string_5x8(4,1,(unsigned char*)"JLX electronics Co., ",0);//显示一串 5x8 点阵的 ASCII 字
             display_string_5x8(5,1,(unsigned char*)"Ltd. established at ",0);//显示一串 5x8 点阵的 ASCII 字
             display_string_5x8(6,1,(unsigned char*)"year 2004.Focus LCM. ",0);//显示一串 5x8 点阵的 ASCII 字
             display_string_5x8(7,1,(unsigned char*)"TEL:0755-29784961 ",0);//显示一串 5x8 点阵的 ASCII 字
             display_string_5x8(8,1,(unsigned char*)"FAX:0755-29784964 ",0);//显示一串 5x8 点阵的 ASCII 字
				     break; 
       //LCD_delay(86000);
				 case 5:
             display_GB2312_string(1,1,(unsigned char*)"啊阿埃挨哎唉哀皑",1); //在第 1 页，第 1 列，显示一串 16x16 点阵汉字或 8x16 的 ASCII 字
             display_GB2312_string(3,1,(unsigned char*)"癌蔼矮艾碍爱隘鞍",0); //显示一串 16x16 点阵汉字或 8x16 的 ASCII 字.以下雷同
             display_GB2312_string(5,1,(unsigned char*)"氨安俺按暗岸胺案",1);
             display_GB2312_string(7,1,(unsigned char*)"肮昂盎凹敖熬翱袄",0);
				     break;
       //LCD_delay(86000);
				 case 6:
             display_GB2312_string(1,1,(unsigned char*)"鬟鬣麽麾縻麂麇麈",1);
             display_GB2312_string(3,1,(unsigned char*)"麋麒鏖麝麟黛黜黝",0);
             display_GB2312_string(5,1,(unsigned char*)"黠黟黢黩黧黥黪黯",1);
             display_GB2312_string(7,1,(unsigned char*)"鼢鼬鼯鼹鼷鼽鼾齄",0);
				     break;
       //LCD_delay(86000);
				 default: ui_state = 0; break;
			 }
			 
			 ui_state = (ui_state + 1)%7;
		 }
	}

}

//*****************************************************************************
//
// 函数原型：void GPIOInit(void)
// 函数功能：GPIO初始化。使能PortK，设置PK4,PK5为输出；使能PortM，设置PM0为输出。
//          （PK4连接TM1638的STB，PK5连接TM1638的DIO，PM0连接TM1638的CLK）
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void GPIOInit(void)
{
	//配置TM1638芯片管脚
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);						// 使能端口 K	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)){};		// 等待端口 K准备完毕		
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);						// 使能端口 M	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)){};		// 等待端口 M准备完毕		
	
  // 设置端口 K的第4,5位（PK4,PK5）为输出引脚		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
	// 设置端口 M的第0位（PM0）为输出引脚   PM0-CLK
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);	
}

//*****************************************************************************
// 
// 函数原型：SysTickInit(void)
// 函数功能：设置SysTick中断
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void SysTickInit(void)
{
	SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); // 设置心跳节拍,定时周期20ms
	SysTickEnable();  			// SysTick使能
	SysTickIntEnable();			// SysTick中断允许
}

//*****************************************************************************
// 
// 函数原型：void DevicesInit(void)
// 函数功能：CU器件初始化，包括系统时钟设置、GPIO初始化和SysTick中断设置
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void DevicesInit(void)
{
	// 使用外部25MHz主时钟源，经过PLL，然后分频为20MHz
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | 
	                                   SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 
	                                   20000000);

	GPIOInit();             // GPIO初始化
    SysTickInit();          // 设置SysTick中断
    IntMasterEnable();			// 总中断允许
}

//*****************************************************************************
// 
// 函数原型：void SysTick_Handler(void)
// 函数功能：SysTick中断服务程序，注意其每隔20ms自动触发一次
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void SysTick_Handler(void)       // 定时周期为20ms
{
	// 0.1秒钟软定时器计数
	if (++clock100ms >= V_T100ms)
	{
		clock100ms_flag = 1; // 当0.1秒到时，溢出标志置1
		clock100ms = 0;
	}
	
 	// 0.5秒钟软定时器计数
	if (++clock500ms >= V_T500ms)
	{
		clock500ms_flag = 1; // 当0.5秒到时，溢出标志置1
		clock500ms = 0;
	}
	
	//2秒钟软件计时器计数
	if (++clock2s >= V_T2s)
	{
		clock2s_flag = 1; // 当2s到时，溢出标志置1
		clock2s = 0;
	}
	
	// 刷新显示板上的全部数码管、小数点和LED指示灯
	TM1638_RefreshDIGIandLED(digit, pnt, led);

	// 检查当前键盘输入，0代表无键操作，1-9表示有对应按键
	// 键号显示在从左到右第2位数码管(下标5)上
	key_code = TM1638_Readkeyboard();

	digit[5] = key_code;
}
