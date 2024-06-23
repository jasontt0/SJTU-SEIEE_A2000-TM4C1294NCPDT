//******************************************************************************************
//
// Copyright: 2020-2021, �Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// File name: LM75BD_Hardware_IIC.c
// Description: ��ʾ�����ÿ��1000ms��ȡ�¶ȴ�����LM75BD���¶�ֵ
//    1.���4���������ʾ��һ���¶ȴ�����LM75DB��ֵ(ADDR:0x48)����1λΪ����λ��������ʾ������ʾ-
//    2.�Ҳ�4���������ʾ�ڶ����¶ȴ�����LM75DB��ֵ(ADDR:0x4F)����1λΪ����λ��������ʾ������ʾ-
// Author:	�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// Version: 1.0.0.20210513
// Date��2021-05-13
// History��
//
//******************************************************************************************

//******************************************************************************************
//
// ͷ�ļ�
//
//******************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h" // ��ַ�궨��
#include "inc/hw_types.h"
#include "inc/hw_timer.h" // ??????????
#include "inc/hw_ints.h"
#include "driverlib/debug.h"	 // ������
#include "driverlib/gpio.h"		 // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"	 // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"	 // ϵͳ���ƶ���
#include "driverlib/systick.h"	 // SysTick Driver ԭ��
#include "driverlib/interrupt.h" // NVIC Interrupt Controller Driver ԭ��
#include "driverlib/fpu.h"
#include "driverlib/timer.h" // ?Timer???????
#include "driverlib/pwm.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"

#include "JXL.c"
#include "tm1638.h" // �����TM1638оƬ�йصĺ���
#include "LM75BD.h" // �����LM75BDоƬ�йصĺ���
#include "PWM.h"
#include "ADC.h"
//******************************************************************************************
//
// �궨��
//
//******************************************************************************************
#define SYSTICK_FREQUENCY 50 // SysTickƵ��Ϊ10Hz����ѭ����ʱ����100ms
#define MilliSecond 4000
#define V_T1s 10 // 1s������ʱ�����ֵ��50��20ms
#define V_T100ms 1
//******************************************************************************************
//
// ����ԭ������
//
//******************************************************************************************
void GPIOInit(void);	// GPIO��ʼ��
void SysTickInit(void); // ����SysTick�ж�
void DevicesInit(void); // MCU������ʼ����ע���������������
void UARTInit(void);
void UARTStringPut(uint32_t ui32Base, const char *cMessage);
void DelayMilliSec(uint32_t ui32DelaySecond);
void disp_number(uint32_t page, uint32_t column, uint8_t number, uint8_t inverse);
void refresh_temp_in(void);
void refresh_volt_in(void);
void refresh_temp_lcd(void);
void refresh_volt_lcd(void);
void trans_temp(void);

void playmusic(void);

//******************************************************************************************
//
// ��������
//
//******************************************************************************************

uint8_t clock40ms = 0;
uint8_t clock40ms_flag = 0;

// ������ʱ������
uint8_t clock1s = 0;

// ������ʱ�������־
uint8_t clock1s_flag = 0;

// �����ü�����
uint32_t test_counter = 0;

// 8λ�������ʾ�����ֻ���ĸ����
// ע����������λ�������������Ϊ4��5��6��7��0��1��2��3
uint8_t digit[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};

// 8λС���� 1��  0��
// ע����������λС����������������Ϊ4��5��6��7��0��1��2��3
uint8_t pnt = 0x44;

// 8��LEDָʾ��״̬��0��1��
// ע������ָʾ�ƴ������������Ϊ7��6��5��4��3��2��1��0
//     ��ӦԪ��LED8��LED7��LED6��LED5��LED4��LED3��LED2��LED1
uint8_t led[] = {1, 1, 1, 1, 1, 1, 1, 0};

uint8_t temp1[] = {0, 0, 0, 0};
uint8_t temp2[] = {0, 0, 0, 0};
uint8_t volt[] = {0, 0, 0, 0};
uint32_t freq_square = 700;
uint8_t freq_lcd[] = {1, 0, 1, 7};
uint8_t freq_lcd_random[] = {1, 0, 1, 7};

// ��ǰ����ֵ
uint8_t key_code = 0;
uint8_t key_cnt = 0;

uint8_t pre_PJ0 = 1;
uint8_t cur_PJ0 = 1;
uint8_t key = 0;

uint8_t actmode = 0;
uint8_t freqmode = 1;

uint32_t range_flag = 0;

// AIN2(PE1)  ADC???[0-4095]
uint32_t ui32ADC0Value;

// AIN2???(???0.01V) [0.00-3.30]
uint32_t ui32ADC0Voltage;

// ϵͳʱ��Ƶ��
uint32_t ui32SysClock;
uint32_t g_ui32SysClock;

// �����¶�ֵ 1LSB=0.1��
int16_t i16Temperature1;
int16_t i16Temperature2;

uint8_t ui8DigitRefresh = 0;

uint16_t ui16Temp1, ui16Temp2;

uint32_t frequence[4][8] = {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 262, 294, 330, 349, 392, 440, 494}, {0, 523, 587, 659, 698, 784, 880, 988}, {0, 1046, 1175, 1318, 1397, 1568, 1760, 1976}};

uint8_t V_Tquarter = 15;
uint8_t dura_counter = 1;
uint8_t num_of_symbol = 1;

bool music_flag = 0;

uint8_t mode_1;
//******************************************************************************************
//
// ������
//
//******************************************************************************************
/*
 * ������
 * �ú�����ʼ��MCU��������ȡ����ʾ�¶Ⱥ͵�ѹ����
 */
int main(void)
{
	// ��ʼ��MCU����
	DevicesInit();

	// ��ʱ�Եȴ�TM1638�ϵ����
	SysCtlDelay(60 * (ui32SysClock / 3000));

	// ��ʼ��TM1638
	TM1638_Init();

	// ��ʼ��LCD
	initial_lcd();

	// ��UART�����ַ���
	UARTStringPut(UART6_BASE, (const char *)"AT+FRE=100.0\r\n");

	// ��ȡ�����¶ȴ��������¶�ֵ
	i16Temperature1 = GetTemputerature(LM75BD_ADR2);
	i16Temperature2 = GetTemputerature(LM75BD_ADR1);

	// �����¶�ֵ��ת��Ϊ�޷������������洢ʮ���Ƹ�λ����ֵ
	if (i16Temperature1 < 0)
	{
		ui16Temp1 = -i16Temperature1;
		temp1[0] = 1; // ������־
	}
	else
	{
		ui16Temp1 = i16Temperature1;
		temp1[0] = 0; // ����
	}

	if (i16Temperature2 < 0)
	{
		ui16Temp2 = -i16Temperature2;
		temp2[0] = 1; // ������־
	}
	else
	{
		ui16Temp2 = i16Temperature2;
		temp2[0] = 0; // ����
	}

	// �����¶ȵ�ʮλ�͸�λ��ֵ
	temp1[1] = ui16Temp1 / 100;
	temp1[2] = ui16Temp1 / 10 % 10;
	temp1[3] = ui16Temp1 % 10;

	temp2[1] = ui16Temp2 / 100;
	temp2[2] = ui16Temp2 / 10 % 10;
	temp2[3] = ui16Temp2 % 10;

	// ����ADCֵ����ת��Ϊ��ѹֵ
	ui32ADC0Value = ADC_Sample();
	ui32ADC0Voltage = ui32ADC0Value * 3300 / 4095;

	// �����ѹ�ĸ�λ��ֵ
	volt[0] = (ui32ADC0Voltage / 1000) % 10;
	volt[1] = (ui32ADC0Voltage / 100) % 10;
	volt[2] = (ui32ADC0Voltage / 10) % 10;
	volt[3] = ui32ADC0Voltage % 10;

	// ���LCD��Ļ������ʾ��ѹ���¶�����
	clear_screen();
	display_GB2312_string(1, 1, "��ѹ:", 0);
	display_GB2312_string(3, 1, "�¶�:", 0);
	disp_number(1, 41, volt[0], 0);
	display_GB2312_string(1, 49, ".", 0);
	disp_number(1, 57, volt[1], 0);
	disp_number(1, 65, volt[2], 0);
	disp_number(1, 73, volt[3], 0);
	display_GB2312_string(1, 81, "V", 0);

	// �����¶���������LCD����ʾ����
	if (temp1[0] == 1)
	{
		display_GB2312_string(3, 41, "-", 0);
	}
	else
	{
		display_GB2312_string(3, 41, " ", 0);
	}
	disp_number(3, 49, temp1[1], 0);
	disp_number(3, 57, temp1[2], 0);
	display_GB2312_string(3, 65, ".", 0);
	disp_number(3, 73, temp1[3], 0);
	display_GB2312_string(3, 89, "��", 0);

	if (temp2[0] == 1)
	{
		display_GB2312_string(5, 41, "-", 0);
	}
	else
	{
		display_GB2312_string(5, 41, " ", 0);
	}
	disp_number(5, 49, temp2[1], 0);
	disp_number(5, 57, temp2[2], 0);
	display_GB2312_string(5, 65, ".", 0);
	disp_number(5, 73, temp2[3], 0);
	display_GB2312_string(5, 89, "��", 0);

	while (1)
	{

		if (clock1s_flag == 1) // ˢ���¶���ʾ
		{
			clock1s_flag = 0;
			refresh_temp_in();
			// PWMStart(523);
			if (actmode == 0)
			{
				refresh_temp_lcd();
			}
		}

		if (clock40ms_flag == 1) // ˢ�µ�ѹ��ʾ
		{
			clock40ms_flag = 0;
			refresh_volt_in();
			if (actmode == 0)
			{
				refresh_volt_lcd();
			}
		}

		if (mode_1 == 3)
		{
			trans_temp();
		}

		/////////////////////////////////////////////////////////////////////////////////////

		if (key == 0) // �ް�������
		{
			key = 0;
		}
		// ���¼�1
		else if (key == 1)
		{
			key = 0;
			if (actmode == 0)
			{
				actmode = 1;
				clear_screen();
				display_GB2312_string(1, 1, "��Ƶ:", 0);
				if (freq_lcd[0] == 0)
				{
					freqmode = 2;
					display_GB2312_string(1, 41, " ", 0);
				}
				else
				{
					freqmode = 1;
					disp_number(1, 41, freq_lcd[0], 1);
				}

				if (freq_lcd[0] == 0)
				{
					disp_number(1, 49, freq_lcd[1], 1);
				}
				else
				{
					disp_number(1, 49, freq_lcd[1], 0);
				}
				disp_number(1, 57, freq_lcd[2], 0);
				display_GB2312_string(1, 65, ".", 0);
				disp_number(1, 73, freq_lcd[3], 0);
				display_GB2312_string(1, 81, "MHz", 0);
				display_GB2312_string(7, 97, "ȷ��", 0);
			}
		}
		// ���¼�2
		else if (key == 2)
		{
			key = 0;
			if (actmode == 0)
			{
				actmode = 1;
				clear_screen();
				display_GB2312_string(1, 1, "��Ƶ:", 0);
				if (freq_lcd[0] == 0)
				{
					freqmode = 2;
					display_GB2312_string(1, 41, " ", 0);
				}
				else
				{
					freqmode = 1;
					disp_number(1, 41, freq_lcd[0], 1);
				}

				if (freq_lcd[0] == 0)
				{
					disp_number(1, 49, freq_lcd[1], 1);
				}
				else
				{
					disp_number(1, 49, freq_lcd[1], 0);
				}
				disp_number(1, 57, freq_lcd[2], 0);
				display_GB2312_string(1, 65, ".", 0);
				disp_number(1, 73, freq_lcd[3], 0);
				display_GB2312_string(1, 81, "MHz", 0);
				display_GB2312_string(7, 97, "ȷ��", 0);
			}
			else if (actmode == 1)
			{
				if (freqmode == 1)
				{
					if (freq_lcd_random[0] != 9)
					{
						++freq_lcd_random[0];
						disp_number(1, 41, freq_lcd_random[0], 1);
					}
					else
					{
						freq_lcd_random[0] = 0;
						disp_number(1, 41, freq_lcd_random[0], 1);
					}
				}
				else if (freqmode == 2)
				{
					if (freq_lcd_random[1] == 9)
					{
						freq_lcd_random[1] = 0;
						disp_number(1, 49, freq_lcd_random[1], 1);
						if (freq_lcd_random[0] == 9)
						{
							freq_lcd_random[0] = 0;
							disp_number(1, 41, freq_lcd_random[0], 0);
						}
						else
						{
							++freq_lcd_random[0];
							disp_number(1, 41, freq_lcd_random[0], 0);
						}
					}
					else
					{
						++freq_lcd_random[1];
						disp_number(1, 49, freq_lcd_random[1], 1);
					}
				}
				else if (freqmode == 3)
				{
					if (freq_lcd_random[2] == 9)
					{
						freq_lcd_random[2] = 0;
						disp_number(1, 57, freq_lcd_random[2], 1);
						if (freq_lcd_random[1] == 9)
						{
							freq_lcd_random[1] = 0;
							disp_number(1, 49, freq_lcd_random[1], 0);
							if (freq_lcd_random[0] == 9)
							{
								freq_lcd_random[0] = 0;
								disp_number(1, 41, freq_lcd_random[0], 0);
							}
							else
							{
								++freq_lcd_random[0];
								disp_number(1, 41, freq_lcd_random[0], 0);
							}
						}
						else
						{
							++freq_lcd_random[1];
							disp_number(1, 49, freq_lcd_random[1], 0);
						}
					}
					else
					{
						++freq_lcd_random[2];
						disp_number(1, 57, freq_lcd_random[2], 1);
					}
				}
				else if (freqmode == 5)
				{
					if (freq_lcd_random[3] == 9)
					{
						freq_lcd_random[3] = 0;
						disp_number(1, 73, freq_lcd_random[3], 1);
						if (freq_lcd_random[2] == 9)
						{
							freq_lcd_random[2] = 0;
							disp_number(1, 57, freq_lcd_random[2], 0);
							if (freq_lcd_random[1] == 9)
							{
								freq_lcd_random[1] = 0;
								disp_number(1, 49, freq_lcd_random[1], 0);
								if (freq_lcd_random[0] == 9)
								{
									freq_lcd_random[0] = 0;
									disp_number(1, 41, freq_lcd_random[0], 0);
								}
								else
								{
									++freq_lcd_random[0];
									disp_number(1, 41, freq_lcd_random[0], 0);
								}
							}
							else
							{
								++freq_lcd_random[1];
								disp_number(1, 49, freq_lcd_random[1], 0);
							}
						}
						else
						{
							++freq_lcd_random[2];
							disp_number(1, 57, freq_lcd_random[2], 0);
						}
					}
					else
					{
						++freq_lcd_random[3];
						disp_number(1, 73, freq_lcd_random[3], 1);
					}
				}
				else if (freqmode == 7)
				{
				}
			}
		}
		// ���¼�3
		else if (key == 3)
		{
			key = 0;
			if (actmode == 0)
			{
				actmode = 1;
				clear_screen();
				display_GB2312_string(1, 1, "��Ƶ:", 0);
				if (freq_lcd[0] == 0)
				{
					freqmode = 2;
					display_GB2312_string(1, 41, " ", 0);
				}
				else
				{
					freqmode = 1;
					disp_number(1, 41, freq_lcd[0], 1);
				}

				if (freq_lcd[0] == 0)
				{
					disp_number(1, 49, freq_lcd[1], 1);
				}
				else
				{
					disp_number(1, 49, freq_lcd[1], 0);
				}
				disp_number(1, 57, freq_lcd[2], 0);
				display_GB2312_string(1, 65, ".", 0);
				disp_number(1, 73, freq_lcd[3], 0);
				display_GB2312_string(1, 81, "MHz", 0);
				display_GB2312_string(7, 97, "ȷ��", 0);
			}
		}
		// ���¼�4
		else if (key == 4)
		{
			key = 0;
			if (actmode == 0)
			{
				actmode = 1;
				clear_screen();
				display_GB2312_string(1, 1, "��Ƶ:", 0);
				if (freq_lcd[0] == 0)
				{
					freqmode = 2;
					display_GB2312_string(1, 41, " ", 0);
				}
				else
				{
					freqmode = 1;
					disp_number(1, 41, freq_lcd[0], 1);
				}

				if (freq_lcd[0] == 0)
				{
					disp_number(1, 49, freq_lcd[1], 1);
				}
				else
				{
					disp_number(1, 49, freq_lcd[1], 0);
				}
				disp_number(1, 57, freq_lcd[2], 0);
				display_GB2312_string(1, 65, ".", 0);
				disp_number(1, 73, freq_lcd[3], 0);
				display_GB2312_string(1, 81, "MHz", 0);
				display_GB2312_string(7, 97, "ȷ��", 0);
			}
			else if (actmode == 1)
			{
				if (freqmode == 1)
				{
					freqmode = 7;
					disp_number(1, 41, freq_lcd_random[0], 0);
					display_GB2312_string(7, 97, "ȷ��", 1);
				}
				else if (freqmode == 2)
				{
					if (freq_lcd_random[0] == 0)
					{
						freqmode = 7;
						display_GB2312_string(7, 97, "ȷ��", 1);
						disp_number(1, 49, freq_lcd_random[1], 0);
					}
					else
					{
						freqmode = 1;
						disp_number(1, 49, freq_lcd_random[1], 0);
						disp_number(1, 41, freq_lcd_random[0], 1);
					}
				}
				else if (freqmode == 3)
				{
					freqmode = 2;
					disp_number(1, 49, freq_lcd_random[1], 1);
					disp_number(1, 57, freq_lcd_random[2], 0);
				}
				else if (freqmode == 5)
				{
					freqmode = 3;
					disp_number(1, 57, freq_lcd_random[2], 1);
					disp_number(1, 73, freq_lcd_random[3], 0);
				}
				else if (freqmode == 7)
				{
					freqmode = 5;
					display_GB2312_string(7, 97, "ȷ��", 0);
					disp_number(1, 73, freq_lcd_random[3], 1);
				}
			}
		}
		// ���¼�5
		else if (key == 5)
		{
			key = 0;
			if (actmode == 0)
			{
				actmode = 1;
				clear_screen();
				display_GB2312_string(1, 1, "��Ƶ:", 0);
				if (freq_lcd[0] == 0)
				{
					freqmode = 2;
					display_GB2312_string(1, 41, " ", 0);
				}
				else
				{
					freqmode = 1;
					disp_number(1, 41, freq_lcd[0], 1);
				}

				if (freq_lcd[0] == 0)
				{
					disp_number(1, 49, freq_lcd[1], 1);
				}
				else
				{
					disp_number(1, 49, freq_lcd[1], 0);
				}
				disp_number(1, 57, freq_lcd[2], 0);
				display_GB2312_string(1, 65, ".", 0);
				disp_number(1, 73, freq_lcd[3], 0);
				display_GB2312_string(1, 81, "MHz", 0);
				display_GB2312_string(7, 97, "ȷ��", 0);
			}
			else if (actmode == 1)
			{
				if (freqmode == 7)
				{
					range_flag = freq_lcd_random[0] * 1000 + freq_lcd_random[1] * 100 + freq_lcd_random[2] * 10 + freq_lcd_random[3];
					if (range_flag >= 880 && range_flag <= 1080)
					{
						freq_lcd[0] = freq_lcd_random[0];
						freq_lcd[1] = freq_lcd_random[1];
						freq_lcd[2] = freq_lcd_random[2];
						freq_lcd[3] = freq_lcd_random[3];

						UARTCharPut(UART6_BASE, 'A');
						UARTCharPut(UART6_BASE, 'T');
						UARTCharPut(UART6_BASE, '+');
						UARTCharPut(UART6_BASE, 'F');
						UARTCharPut(UART6_BASE, 'R');
						UARTCharPut(UART6_BASE, 'E');
						UARTCharPut(UART6_BASE, '=');
						if (freq_lcd[0] == 0)
						{
							UARTCharPut(UART6_BASE, '0' + freq_lcd[1]);
							UARTCharPut(UART6_BASE, '0' + freq_lcd[2]);
							UARTCharPut(UART6_BASE, '0' + freq_lcd[3]);
							UARTCharPut(UART6_BASE, '\r');
							UARTCharPut(UART6_BASE, '\n');
						}
						else
						{
							UARTCharPut(UART6_BASE, '0' + freq_lcd[0]);
							UARTCharPut(UART6_BASE, '0' + freq_lcd[1]);
							UARTCharPut(UART6_BASE, '0' + freq_lcd[2]);
							UARTCharPut(UART6_BASE, '0' + freq_lcd[3]);
							UARTCharPut(UART6_BASE, '\r');
							UARTCharPut(UART6_BASE, '\n');
						}

						actmode = 0;
						clear_screen();
						display_GB2312_string(1, 1, "��ѹ:", 0);
						display_GB2312_string(3, 1, "�¶�:", 0);
						disp_number(1, 41, volt[0], 0);
						display_GB2312_string(1, 49, ".", 0);
						disp_number(1, 57, volt[1], 0);
						disp_number(1, 65, volt[2], 0);
						disp_number(1, 73, volt[3], 0);
						display_GB2312_string(1, 81, "V", 0);

						if (temp1[0] == 1)
						{
							display_GB2312_string(3, 41, "-", 0);
						}
						else
						{
							display_GB2312_string(3, 41, " ", 0);
						}
						disp_number(3, 49, temp1[1], 0);
						disp_number(3, 57, temp1[2], 0);
						display_GB2312_string(3, 65, ".", 0);
						disp_number(3, 73, temp1[3], 0);
						display_GB2312_string(3, 89, "��", 0);

						if (temp2[0] == 1)
						{
							display_GB2312_string(5, 41, "-", 0);
						}
						else
						{
							display_GB2312_string(5, 41, " ", 0);
						}
						disp_number(5, 49, temp2[1], 0);
						disp_number(5, 57, temp2[2], 0);
						display_GB2312_string(5, 65, ".", 0);
						disp_number(5, 73, temp2[3], 0);
						display_GB2312_string(5, 89, "��", 0);
					}
					else
					{
						freq_lcd_random[0] = freq_lcd[0];
						freq_lcd_random[1] = freq_lcd[1];
						freq_lcd_random[2] = freq_lcd[2];
						freq_lcd_random[3] = freq_lcd[3];

						clear_screen();
						display_GB2312_string(1, 1, "��Ƶ����Χ��", 0);
						LCD_delay(81000);
						clear_screen();
						actmode = 1;
						display_GB2312_string(1, 1, "��Ƶ:", 0);
						if (freq_lcd[0] == 0)
						{
							freqmode = 2;
							display_GB2312_string(1, 41, " ", 0);
						}
						else
						{
							freqmode = 1;
							disp_number(1, 41, freq_lcd[0], 1);
						}

						if (freq_lcd[0] == 0)
						{
							disp_number(1, 49, freq_lcd[1], 1);
						}
						else
						{
							disp_number(1, 49, freq_lcd[1], 0);
						}
						disp_number(1, 57, freq_lcd[2], 0);
						display_GB2312_string(1, 65, ".", 0);
						disp_number(1, 73, freq_lcd[3], 0);
						display_GB2312_string(1, 81, "MHz", 0);
						display_GB2312_string(7, 97, "ȷ��", 0);
					}
				}
			}
		}
		// ���¼�6
		else if (key == 6)
		{
			key = 0;
			if (actmode == 0)
			{
				actmode = 1;
				clear_screen();
				display_GB2312_string(1, 1, "��Ƶ:", 0);
				if (freq_lcd[0] == 0)
				{
					freqmode = 2;
					display_GB2312_string(1, 41, " ", 0);
				}
				else
				{
					freqmode = 1;
					disp_number(1, 41, freq_lcd[0], 1);
				}

				if (freq_lcd[0] == 0)
				{
					disp_number(1, 49, freq_lcd[1], 1);
				}
				else
				{
					disp_number(1, 49, freq_lcd[1], 0);
				}
				disp_number(1, 57, freq_lcd[2], 0);
				display_GB2312_string(1, 65, ".", 0);
				disp_number(1, 73, freq_lcd[3], 0);
				display_GB2312_string(1, 81, "MHz", 0);
				display_GB2312_string(7, 97, "ȷ��", 0);
			}
			else if (actmode == 1)
			{
				if (freqmode == 1)
				{
					freqmode = 2;
					disp_number(1, 41, freq_lcd_random[0], 0);
					disp_number(1, 49, freq_lcd_random[1], 1);
				}
				else if (freqmode == 2)
				{
					freqmode = 3;
					disp_number(1, 49, freq_lcd_random[1], 0);
					disp_number(1, 57, freq_lcd_random[2], 1);
				}
				else if (freqmode == 3)
				{
					freqmode = 5;
					disp_number(1, 57, freq_lcd_random[2], 0);
					disp_number(1, 73, freq_lcd_random[3], 1);
				}
				else if (freqmode == 5)
				{
					freqmode = 7;
					disp_number(1, 73, freq_lcd_random[3], 0);
					display_GB2312_string(7, 97, "ȷ��", 1);
				}
				else if (freqmode == 7)
				{
					if (freq_lcd_random[0] == 0)
					{
						freqmode = 2;
						display_GB2312_string(7, 97, "ȷ��", 0);
						disp_number(1, 49, freq_lcd_random[1], 1);
					}
					else
					{
						freqmode = 1;
						display_GB2312_string(7, 97, "ȷ��", 0);
						disp_number(1, 41, freq_lcd_random[0], 1);
					}
				}
			}
		}
		// ���¼�7
		else if (key == 7)
		{
			key = 0;
			if (actmode == 0)
			{
				actmode = 1;
				clear_screen();
				display_GB2312_string(1, 1, "��Ƶ:", 0);
				if (freq_lcd[0] == 0)
				{
					freqmode = 2;
					display_GB2312_string(1, 41, " ", 0);
				}
				else
				{
					freqmode = 1;
					disp_number(1, 41, freq_lcd[0], 1);
				}

				if (freq_lcd[0] == 0)
				{
					disp_number(1, 49, freq_lcd[1], 1);
				}
				else
				{
					disp_number(1, 49, freq_lcd[1], 0);
				}
				disp_number(1, 57, freq_lcd[2], 0);
				display_GB2312_string(1, 65, ".", 0);
				disp_number(1, 73, freq_lcd[3], 0);
				display_GB2312_string(1, 81, "MHz", 0);
				display_GB2312_string(7, 97, "ȷ��", 0);
			}
		}
		// ���¼�8
		else if (key == 8)
		{
			key = 0;
			if (actmode == 0)
			{
				actmode = 1;
				clear_screen();
				display_GB2312_string(1, 1, "��Ƶ:", 0);
				if (freq_lcd[0] == 0)
				{
					freqmode = 2;
					display_GB2312_string(1, 41, " ", 0);
				}
				else
				{
					freqmode = 1;
					disp_number(1, 41, freq_lcd[0], 1);
				}

				if (freq_lcd[0] == 0)
				{
					disp_number(1, 49, freq_lcd[1], 1);
				}
				else
				{
					disp_number(1, 49, freq_lcd[1], 0);
				}
				disp_number(1, 57, freq_lcd[2], 0);
				display_GB2312_string(1, 65, ".", 0);
				disp_number(1, 73, freq_lcd[3], 0);
				display_GB2312_string(1, 81, "MHz", 0);
				display_GB2312_string(7, 97, "ȷ��", 0);
			}
			else if (actmode == 1)
			{
				if (freqmode == 1)
				{
					if (freq_lcd_random[0] == 0)
					{
						freq_lcd_random[0] = 9;
						disp_number(1, 41, freq_lcd_random[0], 1);
					}
					else
					{
						--freq_lcd_random[0];
						disp_number(1, 41, freq_lcd_random[0], 1);
					}
				}
				else if (freqmode == 2)
				{
					if (freq_lcd_random[1] == 0)
					{
						freq_lcd_random[1] = 9;
						disp_number(1, 49, freq_lcd_random[1], 1);
						if (freq_lcd_random[0] == 0)
						{
							freq_lcd_random[0] = 9;
							disp_number(1, 41, freq_lcd_random[0], 0);
						}
						else
						{
							--freq_lcd_random[0];
							disp_number(1, 41, freq_lcd_random[0], 0);
						}
					}
					else
					{
						--freq_lcd_random[1];
						disp_number(1, 49, freq_lcd_random[1], 1);
					}
				}
				else if (freqmode == 3)
				{
					if (freq_lcd_random[2] == 0)
					{
						freq_lcd_random[2] = 9;
						disp_number(1, 57, freq_lcd_random[2], 1);
						if (freq_lcd_random[1] == 0)
						{
							freq_lcd_random[1] = 9;
							disp_number(1, 49, freq_lcd_random[1], 0);
							if (freq_lcd_random[0] == 0)
							{
								freq_lcd_random[0] = 9;
								disp_number(1, 41, freq_lcd_random[0], 0);
							}
							else
							{
								--freq_lcd_random[0];
								disp_number(1, 41, freq_lcd_random[0], 0);
							}
						}
						else
						{
							--freq_lcd_random[1];
							disp_number(1, 49, freq_lcd_random[1], 0);
						}
					}
					else
					{
						--freq_lcd_random[2];
						disp_number(1, 57, freq_lcd_random[2], 1);
					}
				}
				else if (freqmode == 5)
				{
					if (freq_lcd_random[3] == 0)
					{
						freq_lcd_random[3] = 9;
						disp_number(1, 73, freq_lcd_random[3], 1);
						if (freq_lcd_random[2] == 0)
						{
							freq_lcd_random[2] = 9;
							disp_number(1, 57, freq_lcd_random[2], 0);
							if (freq_lcd_random[1] == 0)
							{
								freq_lcd_random[1] = 9;
								disp_number(1, 49, freq_lcd_random[1], 0);
								if (freq_lcd_random[0] == 0)
								{
									freq_lcd_random[0] = 9;
									disp_number(1, 41, freq_lcd_random[0], 0);
								}
								else
								{
									--freq_lcd_random[0];
									disp_number(1, 41, freq_lcd_random[0], 0);
								}
							}
							else
							{
								--freq_lcd_random[1];
								disp_number(1, 49, freq_lcd_random[1], 0);
							}
						}
						else
						{
							--freq_lcd_random[2];
							disp_number(1, 57, freq_lcd_random[2], 0);
						}
					}
					else
					{
						--freq_lcd_random[3];
						disp_number(1, 73, freq_lcd_random[3], 1);
					}
				}
				else if (freqmode == 7)
				{
				}
			}
		}
		// ���¼�9
		else if (key == 9)
		{
			key = 0;
			if (actmode == 0)
			{
				actmode = 1;
				clear_screen();
				display_GB2312_string(1, 1, "��Ƶ:", 0);
				if (freq_lcd[0] == 0)
				{
					freqmode = 2;
					display_GB2312_string(1, 41, " ", 0);
				}
				else
				{
					freqmode = 1;
					disp_number(1, 41, freq_lcd[0], 1);
				}

				if (freq_lcd[0] == 0)
				{
					disp_number(1, 49, freq_lcd[1], 1);
				}
				else
				{
					disp_number(1, 49, freq_lcd[1], 0);
				}
				disp_number(1, 57, freq_lcd[2], 0);
				display_GB2312_string(1, 65, ".", 0);
				disp_number(1, 73, freq_lcd[3], 0);
				display_GB2312_string(1, 81, "MHz", 0);
				display_GB2312_string(7, 97, "ȷ��", 0);
			}
		}
	}
}

// �¶�ת��ΪƵ�ʱ���**********************************************************************
void trans_temp(void)
{
	freq_square = 92.5 * (temp2[1] * 10 + temp2[2] + 0.1 * temp2[3]) + 300;

	PWMStart_1(freq_square);
}

// ˢ�µ�ѹ��ʾ*************************************************************
void refresh_volt_lcd(void)
{
	disp_number(1, 41, volt[0], 0);
	disp_number(1, 57, volt[1], 0);
	disp_number(1, 65, volt[2], 0);
	disp_number(1, 73, volt[3], 0);
}

// ˢ���¶���ʾ*************************************************************
void refresh_temp_lcd(void)
{
	if (temp1[0] == 1)
	{
		display_GB2312_string(3, 41, "-", 0);
	}
	else
	{
		display_GB2312_string(3, 41, " ", 0);
	}
	disp_number(3, 49, temp1[1], 0);
	disp_number(3, 57, temp1[2], 0);
	disp_number(3, 73, temp1[3], 0);

	if (temp2[0] == 1)
	{
		display_GB2312_string(5, 41, "-", 0);
	}
	else
	{
		display_GB2312_string(5, 41, " ", 0);
	}
	disp_number(5, 49, temp2[1], 0);
	disp_number(5, 57, temp2[2], 0);
	disp_number(5, 73, temp2[3], 0);
}

//**************************************************************
void DelayMilliSec(uint32_t ui32DelaySecond)
{
	uint32_t ui32Loop;

	ui32DelaySecond = ui32DelaySecond * MilliSecond;
	for (ui32Loop = 0; ui32Loop < ui32DelaySecond; ui32Loop++)
	{
	};
}

// ��ʾ����**********************************************************************
void disp_number(uint32_t page, uint32_t column, uint8_t number, uint8_t inverse)
{
	if (number == 0)
	{
		display_GB2312_string(page, column, "0", inverse);
	}
	if (number == 1)
	{
		display_GB2312_string(page, column, "1", inverse);
	}
	if (number == 2)
	{
		display_GB2312_string(page, column, "2", inverse);
	}
	if (number == 3)
	{
		display_GB2312_string(page, column, "3", inverse);
	}
	if (number == 4)
	{
		display_GB2312_string(page, column, "4", inverse);
	}
	if (number == 5)
	{
		display_GB2312_string(page, column, "5", inverse);
	}
	if (number == 6)
	{
		display_GB2312_string(page, column, "6", inverse);
	}
	if (number == 7)
	{
		display_GB2312_string(page, column, "7", inverse);
	}
	if (number == 8)
	{
		display_GB2312_string(page, column, "8", inverse);
	}
	if (number == 9)
	{
		display_GB2312_string(page, column, "9", inverse);
	}
}

// ˢ���¶ȴ�������ȡ�����¶�************************************************************
void refresh_temp_in(void)
{
	i16Temperature1 = GetTemputerature(LM75BD_ADR2);
	i16Temperature2 = GetTemputerature(LM75BD_ADR1);
	if (i16Temperature1 < 0)
	{
		ui16Temp1 = -i16Temperature1;
		temp1[0] = 1;
	}
	else
	{
		ui16Temp1 = i16Temperature1;
		temp1[0] = 0;
	}

	if (i16Temperature2 < 0)
	{
		ui16Temp2 = -i16Temperature2;
		temp2[0] = 1;
	}
	else
	{
		ui16Temp2 = i16Temperature2;
		temp2[0] = 0;
	}

	temp1[1] = ui16Temp1 / 100;		// ����ʮλ��
	temp1[2] = ui16Temp1 / 10 % 10; // �����λ��
	temp1[3] = ui16Temp1 % 10;

	temp2[1] = ui16Temp2 / 100;		// ����ʮλ��
	temp2[2] = ui16Temp2 / 10 % 10; // �����λ��
	temp2[3] = ui16Temp2 % 10;
}

// ˢ�¶�ȡ���ĵ�ѹ*****************************************************
void refresh_volt_in(void)
{
	ui32ADC0Value = (ADC_Sample() + ADC_Sample() + ADC_Sample() + ADC_Sample() + ADC_Sample() + ADC_Sample() + ADC_Sample() + ADC_Sample() + ADC_Sample() + ADC_Sample() + ADC_Sample() + ADC_Sample() + ADC_Sample() + ADC_Sample() + ADC_Sample()) / 15;
	ui32ADC0Voltage = ui32ADC0Value * 3300 / 4095;

	volt[0] = (ui32ADC0Voltage / 1000) % 10; // ????????
	volt[1] = (ui32ADC0Voltage / 100) % 10;	 // ?????????
	volt[2] = (ui32ADC0Voltage / 10) % 10;	 // ?????????
	volt[3] = ui32ADC0Voltage % 10;
}

//******************************************************************************************
//
// ����ԭ�ͣ�void GPIOInit(void)
// �������ܣ�GPIO��ʼ����
//     ʹ��PortK������PK4,PK5Ϊ�����ʹ��PortM������PM0Ϊ�����
//     PK4����TM1638��STB��PK5����TM1638��DIO��PM0����TM1638��CLK��
//     ʹ��PortL������LPL0,PL1Ϊ�����PL0��PL1�ֱ�����DAC6571��SDA��SCL��
// ������������
// ��������ֵ����
//
//******************************************************************************************
void GPIOInit(void)
{
	// ����TM1638оƬ�ܽ�
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK); // ʹ�ܶ˿� K
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK))
	{
	}; // �ȴ��˿� K ׼�����

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM); // ʹ�ܶ˿� M
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM))
	{
	}; // �ȴ��˿� M ׼�����

	// ���ö˿� K �ĵ�4,5λ��PK4,PK5��Ϊ�������		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4 | GPIO_PIN_5);
	// ���ö˿� M �ĵ�0λ��PM0��Ϊ�������   PM0-CLK
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); // ʹ�ܶ˿� J
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ))
	{
	}; // �ȴ��˿� J ׼�����

	// ���ö˿� J �ĵ�0,1λ��PJ0,PJ1��Ϊ�������		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);
}

void UARTStringPut(uint32_t ui32Base, const char *cMessage)
{
	while (*cMessage != '\0')
	{
		UARTCharPut(ui32Base, *(cMessage++));
	}
	DelayMilliSec(1);
}

void UARTInit(void)
{
	// ????
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0); // ??UART0??
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // ???? A
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
		; // ???? A????

	GPIOPinConfigure(GPIO_PA0_U0RX); // ??PA0?UART0 RX??
	GPIOPinConfigure(GPIO_PA1_U0TX); // ??PA1?UART0 TX??

	// ???? A??0,1?(PA0,PA1)?UART??
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// ?????????
	UARTConfigSetExpClk(UART0_BASE,
						ui32SysClock,
						38400,					 // ???:115200
						(UART_CONFIG_WLEN_8 |	 // ???:8
						 UART_CONFIG_STOP_ONE |	 // ???:1
						 UART_CONFIG_PAR_NONE)); // ???:?

	// ????
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6); // ??UART0??
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP); // ???? A
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP))
		; // ???? A????

	GPIOPinConfigure(GPIO_PP0_U6RX); // ??PA0?UART0 RX??
	GPIOPinConfigure(GPIO_PP1_U6TX); // ??PA1?UART0 TX??

	// ???? A??0,1?(PA0,PA1)?UART??
	GPIOPinTypeUART(GPIO_PORTP_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// ?????????
	UARTConfigSetExpClk(UART6_BASE,
						ui32SysClock,
						38400,					 // ???:115200
						(UART_CONFIG_WLEN_8 |	 // ???:8
						 UART_CONFIG_STOP_ONE |	 // ???:1
						 UART_CONFIG_PAR_NONE)); // ???:?
}

//******************************************************************************************
//
// ����ԭ�ͣ�SysTickInit(void)
// �������ܣ�����SysTick�ж�
// ������������
// ��������ֵ����
//
//******************************************************************************************
void SysTickInit(void)
{
	SysTickPeriodSet(ui32SysClock / SYSTICK_FREQUENCY); // ������������,��ʱ����20ms
	SysTickEnable();									// SysTickʹ��
	SysTickIntEnable();									// SysTick�ж�����
}

//******************************************************************************************
//
// ����ԭ�ͣ�void DevicesInit(void)
// �������ܣ�MCU������ʼ��������ϵͳʱ�����á�GPIO��ʼ����SysTick�ж�����
// ������������
// ��������ֵ����
//
//******************************************************************************************
void DevicesInit(void)
{
	// ʹ���ⲿ25MHz��ʱ��Դ������PLL��Ȼ���ƵΪ16MHz
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
									   SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
									  16000000);
	g_ui32SysClock = ui32SysClock;

	FPULazyStackingEnable();
	FPUEnable();

	GPIOInit();	   // GPIO��ʼ��
	I2C0Init();	   // I2C0��ʼ��
	SysTickInit(); // ����SysTick�ж�
	UARTInit();
	IntMasterEnable(); // ���ж�����
	ADCInit();
	PWMInit();
}

//******************************************************************************************
//
// ����ԭ�ͣ�void SysTick_Handler(void)
// �������ܣ�SysTick�жϷ������
// ������������
// ��������ֵ����
//
//******************************************************************************************
void SysTick_Handler(void) // ��ʱ����Ϊ20ms
{

	// 0.1��������ʱ������
	if (++clock1s >= V_T1s)
	{
		clock1s_flag = 1; // ��0.1�뵽ʱ�������־��1
		clock1s = 0;
	}

	if (++clock40ms >= V_T100ms)
	{
		clock40ms_flag = 1; // ?40ms??,?????1
		clock40ms = 0;
	}

	// ˢ��ȫ������ܺ�LEDָʾ��
	if (ui8DigitRefresh == 0)
		TM1638_RefreshDIGIandLED(digit, pnt, led);

	// ��鵱ǰ�������룬0�����޼�������1-9��ʾ�ж�Ӧ����
	// ������ʾ��һλ�������
	key_code = TM1638_Readkeyboard();
	if (key_code == 1)
		music_flag = !music_flag;

	if (key_code != 0)
	{
		switch (key_code)
		{
		case 3:
		{
			mode_1 = 3;
			break;
		}
		case 7:
		{
			mode_1 = 7;
			break;
		}
		default:
			break;
		}
	}

	pre_PJ0 = cur_PJ0;
	cur_PJ0 = key_code;
	if (pre_PJ0 == 0 && cur_PJ0 != 0)
	{
		key = cur_PJ0;
	}
	else
	{
		key = 0;
	}

	if (music_flag && mode_1 != 3)
	{
		playmusic();
	}
	else
	{
		PWMStop();
		PWMStop_1();
	}
}

void playmusic(void)
{
	if (dura_counter > 166) // 830 / 5
	{
		num_of_symbol = 33;
	}
	else if (dura_counter > 160) // 800 / 5
	{
		num_of_symbol = 32;
	}
	else if (dura_counter > 157) // 785 / 5
	{
		num_of_symbol = 31;
	}
	else if (dura_counter > 154) // 770 / 5
	{
		num_of_symbol = 30;
	}
	else if (dura_counter > 151) // 755 / 5
	{
		num_of_symbol = 29;
	}
	else if (dura_counter > 142) // 710 / 5
	{
		num_of_symbol = 28;
	}
	else if (dura_counter > 130) // 650 / 5
	{
		num_of_symbol = 27;
	}
	else if (dura_counter > 127) // 635 / 5
	{
		num_of_symbol = 26;
	}
	else if (dura_counter > 121) // 605 / 5
	{
		num_of_symbol = 25;
	}
	else if (dura_counter > 118) // 118 * 5
	{
		num_of_symbol = 24;
	}
	else if (dura_counter > 115) // 590 / 5
	{
		num_of_symbol = 23;
	}
	else if (dura_counter > 112) // 575 / 5
	{
		num_of_symbol = 22;
	}
	else if (dura_counter > 109) // 560 / 5
	{
		num_of_symbol = 21;
	}
	else if (dura_counter > 106) // 545 / 5
	{
		num_of_symbol = 20;
	}
	else if (dura_counter > 103) // 530 / 5
	{
		num_of_symbol = 19;
	}
	else if (dura_counter > 94) // 470 / 5
	{
		num_of_symbol = 18;
	}
	else if (dura_counter > 91) // 455 / 5
	{
		num_of_symbol = 17;
	}
	else if (dura_counter > 90) // 450 / 5
	{
		num_of_symbol = 16;
	}
	else if (dura_counter > 72) // 360 / 5
	{
		num_of_symbol = 15;
	}
	else if (dura_counter > 69) // 345 / 5
	{
		num_of_symbol = 14;
	}
	else if (dura_counter > 66) // 330 / 5
	{
		num_of_symbol = 13;
	}
	else if (dura_counter > 63) // 315 / 5
	{
		num_of_symbol = 12;
	}
	else if (dura_counter > 60) // 300 / 5
	{
		num_of_symbol = 11;
	}
	else if (dura_counter > 57) // 285 / 5
	{
		num_of_symbol = 10;
	}
	else if (dura_counter > 50) // 270 / 5
	{
		num_of_symbol = 9;
	}
	else if (dura_counter > 48) // 240 / 5
	{
		num_of_symbol = 8;
	}
	else if (dura_counter > 24) // 120 / 5
	{
		num_of_symbol = 7;
	}
	else if (dura_counter > 21) // 105 / 5
	{
		num_of_symbol = 6;
	}
	else if (dura_counter > 18) // 90 / 5
	{
		num_of_symbol = 5;
	}
	else if (dura_counter > 15) // 75 / 5
	{
		num_of_symbol = 4;
	}
	else if (dura_counter > 12) // 60 / 5
	{
		num_of_symbol = 3;
	}
	else if (dura_counter > 9) // 45 / 5
	{
		num_of_symbol = 2;
	}

	switch (num_of_symbol)
	{
	case 1:
	{
		PWMStart(frequence[1][6]);
		PWMStart_1(frequence[1][6]);
		break;
	}

	case 2:
	{
		PWMStart(frequence[2][3]);
		PWMStart_1(frequence[2][3]);
		break;
	}

	case 3:
	{
		PWMStart(frequence[2][2]);
		PWMStart_1(frequence[2][2]);
		break;
	}

	case 4:
	{
		PWMStart(frequence[2][3]);
		PWMStart_1(frequence[2][3]);
		break;
	}

	case 5:
	{
		PWMStart(frequence[2][1]);
		PWMStart_1(frequence[2][1]);
		break;
	}
	case 6:
	{
		PWMStart(frequence[1][7]);
		PWMStart_1(frequence[1][7]);
		break;
	}
	case 7:
	{
		PWMStart(frequence[1][6]);
		PWMStart_1(frequence[1][6]);
		break;
	}
	case 8:
	{
		PWMStart(frequence[2][3]);
		PWMStart_1(frequence[2][3]);
		break;
	}
	case 9:
	{
		PWMStart(frequence[2][6]);
		PWMStart_1(frequence[2][6]);
		break;
	}
	case 10:
	{
		PWMStart(frequence[2][5]);
		PWMStart_1(frequence[2][5]);
		break;
	}
	case 11:
	{
		PWMStart(frequence[2][3]);
		PWMStart_1(frequence[2][3]);
		break;
	}
	case 12:
	{
		PWMStart(frequence[2][2]);
		PWMStart_1(frequence[2][2]);
		break;
	}
	case 13:
	{
		PWMStart(frequence[2][1]);
		PWMStart_1(frequence[2][2]);
		break;
	}
	case 14:
	{
		PWMStart(frequence[2][2]);
		PWMStart_1(frequence[2][2]);
		break;
	}
	case 15:
	{
		PWMStart(frequence[2][3]);
		PWMStart_1(frequence[2][3]);
		break;
	}
	case 16:
	{
		PWMStart(frequence[2][3]);
		PWMStart_1(frequence[2][3]);
		break;
	}
	case 17:
	{
		PWMStart(frequence[2][5]);
		PWMStart_1(frequence[2][5]);
		break;
	}
	case 18:
	{
		PWMStart(frequence[2][6]);
		PWMStart_1(frequence[2][6]);
		break;
	}
	case 19:
	{
		PWMStart(frequence[2][7]);
		PWMStart_1(frequence[2][7]);
		break;
	}
	case 20:
	{
		PWMStart(frequence[2][6]);
		PWMStart_1(frequence[2][6]);
		break;
	}
	case 21:
	{
		PWMStart(frequence[2][5]);
		PWMStart_1(frequence[2][5]);
		break;
	}
	case 22:
	{
		PWMStart(frequence[2][3]);
		PWMStart_1(frequence[2][3]);
		break;
	}
	case 23:
	{
		PWMStart(frequence[2][2]);
		PWMStart_1(frequence[2][2]);
		break;
	}
	case 24:
	{
		PWMStart(frequence[2][1]);
		PWMStart_1(frequence[2][1]);
		break;
	}
	case 25:
	{
		PWMStart(frequence[2][1]);
		PWMStart_1(frequence[2][1]);
		break;
	}
	case 26:
	{
		PWMStart(frequence[2][2]);
		PWMStart_1(frequence[2][2]);
		break;
	}
	case 27:
	{
		PWMStart(frequence[2][3]);
		PWMStart_1(frequence[2][3]);
		break;
	}
	case 28:
	{
		PWMStart(frequence[2][2]);
		PWMStart_1(frequence[2][2]);
		break;
	}
	case 29:
	{
		PWMStart(frequence[2][3]);
		PWMStart_1(frequence[2][3]);
		break;
	}
	case 30:
	{
		PWMStart(frequence[1][7]);
		PWMStart_1(frequence[1][7]);
		break;
	}
	case 31:
	{
		PWMStart(frequence[1][6]);
		PWMStart_1(frequence[1][6]);
		break;
	}
	case 32:
	{
		PWMStart(frequence[1][5]);
		PWMStart_1(frequence[1][5]);
		break;
	}
	case 33:
	{
		PWMStart(frequence[1][6]);
		PWMStart_1(frequence[1][6]);
		break;
	}
	}

	if (dura_counter == 190)
	{
		dura_counter = 0;
		num_of_symbol = 1;
	}
	else
	{
		dura_counter++;
	}
}