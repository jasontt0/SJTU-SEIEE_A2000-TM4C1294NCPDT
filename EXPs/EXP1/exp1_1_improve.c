//**************************************************************************************
//
// Copyright: 2020-2021, �Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// File name: exp0_debug.c
// Description: LED4(D4-PF0)��Լ��6000����Ϊ���ڻ�����˸��
//              ������PUSH1(USR_SW1-PJ0)����LED4(D4-PF0)��Լ��100����Ϊ���ڿ�����˸��
//              �ɿ�PUSH1(USR_SW1-PJ0)����LED4(D4-PF0)�ָ���500����Ϊ���ڻ�����˸��
// Author:	�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// Version: 1.0.0.20201228
// Date��2020-12-28
// History��
//
//**************************************************************************************

#define DEBUG_CONSOLE

//*****************************************************************************
//
// ͷ�ļ�
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // ��ַ�궨��
#include "inc/hw_types.h"         // �������ͺ궨�壬�Ĵ������ʺ���
#include "driverlib/debug.h"      // ������
#include "driverlib/gpio.h"       // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"    // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"	  // ϵͳ���ƶ���
#include "driverlib/systick.h"    // SysTick Driver ԭ��
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver ԭ��

#include "tm1638.h"               // �����TM1638оƬ�йصĺ���

#include "driverlib/uart.h"       // UART��غ궨��
#include "utils/uartstdio.h"      // UART0��Ϊ����̨��غ���ԭ������  
//**************************************************************************************
//
// �궨��
//
//**************************************************************************************
//#define  MilliSecond      4000    // �γ�1msʱ������ѭ������
//#define  FASTFLASHTIME    100	  // ����ʱ��100ms��
//#define  SLOWFLASHTIME    6000     // ����ʱ��1s��

#define SYSTICK_FREQUENCY		50		// SysTickƵ��Ϊ50Hz����ѭ����ʱ����20ms

#define V_T100ms	5                   // 100ms������ʱ�����ֵ��5��20ms
#define V_T6s	300                  // 6s������ʱ�����ֵ��25��20ms

//**************************************************************************************
//
// ����ԭ������
//
//**************************************************************************************
//void  DelayMilliSec(uint32_t ui32DelaySecond);		// �ӳ�һ��ʱ������λΪ����

void GPIOInit(void);         // GPIO��ʼ��
void SysTickInit(void);     // ����SysTick�ж�
void DevicesInit(void);     // MCU������ʼ����ע���������������

void  PF0Flash(uint8_t ui8KeyValue);      // ���ݴ���İ���ֵ������PF0����������
void  InitConsole(void);      // UART0��ʼ��

void switch_100ms(void);//100ms��������
void switch_6s(void);//6s��������

uint32_t ui32SysClock;
uint8_t ui8KeyValue;

uint32_t clock_6s;
uint32_t clock_100ms;

bool flag_100ms=0;
bool flag_6s=0;

uint8_t ifis_on=1;
//**************************************************************************************
//
// ������
//
//**************************************************************************************
int main(void)
{

//**************************************************************************************
//�ڲ�ʱ��ԴPIOSC(16MHz)����ϵͳʱ��ԴOSCֱ��ʹ�ã���ƵΪ16/12/8 MHz
//	g_ui32SysClock = SysCtlClockFreqSet(SYSCTL_OSC_INT|SYSCTL_USE_OSC,16000000);


//�ⲿʱ��ԴMOSC(25MHz)����ϵͳʱ��ԴOSCֱ��ʹ�ã���ƵΪ25/12/1 MHz
//	g_ui32SysClock = SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_OSC , 1000000);


//�ⲿʱ��ԴMOSC(25MHz)����ϵͳʱ��ԴPLL��Ƶ��480MHz����ƵΪ25/20/8 MHz
  ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 20000000);


//�ڲ�ʱ��ԴPIOSC(16MHz)����ϵͳʱ��ԴPLL��Ƶ��480MHz����ƵΪ20/8/1 MHz
//	g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_OSC_INT|SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),1000000);
//**************************************************************************************


  DevicesInit();     // MCU������ʼ��

#ifdef DEBUG_CONSOLE
  InitConsole();          // UART0��ʼ��
  UARTprintf("Hello Everyone\n");
  UARTprintf("System Clock = %d Hz\n", ui32SysClock);
#endif

GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0); // ���� LED4(D4-PF0)

  while(1)                // ����ѭ��
    {


#ifdef DEBUG_CONSOLE
      UARTprintf("PJ1= %d\n", ui8KeyValue);
#endif

      PF0Flash(ui8KeyValue);          // ���ݴ���İ�������������PF0����������
    }
}


/*
//**************************************************************************************
//
// ����ԭ�ͣ�void DelayMilliSec(uint32_t ui32DelaySecond)
// �������ܣ��ӳ�һ��ʱ������λΪ����
// ����������ui32DelaySecond���ӳٺ�����
//
//**************************************************************************************
void DelayMilliSec(uint32_t ui32DelaySecond)
{
    uint32_t ui32Loop;

    ui32DelaySecond = ui32DelaySecond * MilliSecond;
    for(ui32Loop = 0; ui32Loop < ui32DelaySecond; ui32Loop++){ };
}


*/

//**************************************************************************************
//
// ����ԭ�ͣ�void GPIOInit(void)
// �������ܣ�GPIO��ʼ����ʹ��PortF������PF0Ϊ�����ʹ��PortJ������PJ0Ϊ����
// ������������
//
//**************************************************************************************
void GPIOInit(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);		   // ʹ�ܶ˿� F
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));	   // �ȴ��˿� F׼�����

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);		   // ʹ�ܶ˿� J
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)) {}; // �ȴ��˿� J׼�����

  // ���ö˿� F�ĵ�0λ��PF0��Ϊ�������
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);

//    // ���ö˿� F�ĵ�4λ��PF4��Ϊ�������
//    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4);

//    // ���ö˿� F�ĵ�0��4λ��PF0��PF4��Ϊ�������
//    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4);

  // ���ö˿� J�ĵ�0λ��PJ0��Ϊ��������
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);

  // �˿� J�ĵ�0λ��Ϊ�������룬�������óɡ�����������
  GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0, GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD_WPU);
}


//*****************************************************************************
//
// ����ԭ�ͣ�SysTickInit(void)
// �������ܣ�����SysTick�ж�
// ������������
// ��������ֵ����
//
//*****************************************************************************
void SysTickInit(void)
{
  SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); // ������������,��ʱ����20ms=1/50s
  SysTickEnable();  			// SysTickʹ��
  SysTickIntEnable();			// SysTick�ж�����
}

//*****************************************************************************
//
// ����ԭ�ͣ�void DevicesInit(void)
// �������ܣ�CU������ʼ��������ϵͳʱ�����á�GPIO��ʼ����SysTick�ж�����
// ������������
// ��������ֵ����
//
//*****************************************************************************
void DevicesInit(void)
{
  // ʹ���ⲿ25MHz��ʱ��Դ������PLL��Ȼ���ƵΪ20MHz
  ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |
                                     SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480),
                                    20000000);

  GPIOInit();             // GPIO��ʼ��
  SysTickInit();          // ����SysTick�ж�
  IntMasterEnable();		// ���ж�����
}



//*****************************************************************************
//
// ����ԭ�ͣ�void SysTick_Handler(void)
// �������ܣ�SysTick�жϷ������
// ������������
// ��������ֵ����
//
//*****************************************************************************
void SysTick_Handler(void)       // ��ʱ����Ϊ20ms,��ÿ20ms�Զ�����һ�η�����
{
  ui8KeyValue = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0); // ��ȡ PJ0 ��ֵ  0-���� 1-�ɿ�

  ++clock_100ms;
  if(clock_100ms>=V_T100ms)
    {
      flag_100ms=1;
      clock_100ms=0;
    }

  ++clock_6s;
  if(clock_6s >=V_T6s )
    {
      flag_6s=1;
      clock_6s=0;
    }
}


//**************************************************************************************
//
// ����ԭ�ͣ�void PF0Flash(uint8_t ui8KeyValue)
// �������ܣ����ݴ���İ���ֵ������PF0������������0-������1-����
// ����������ui8KeyValue������ֵ
//
//**************************************************************************************
void PF0Flash(uint8_t ui8KeyValue)
{
  //uint32_t ui32DelayTime;

  if (ui8KeyValue	== 0)// PUSH1(USR_SW1-PJ0) ����
    {
      switch_100ms();
    }
  else switch_6s();                                            // PUSH1(USR_SW1-PJ0) �ɿ�


/*
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0); // ���� LED4(D4-PF0)
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4); // ���� LED3(D3-PF4)
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_PIN_0|GPIO_PIN_4); // ���� LED4(D4-PF0),LED3(D3-PF4)

//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_PIN_0); //


  DelayMilliSec(ui32DelayTime);                          // ��ʱui32DelayTime����

  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x00);        // �ر� LED4(D4-PF0)

//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x00);        // �ر� LED3(D3-PF4)
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, 0x00); // �ر� LED4(D4-PF0),LED3(D3-PF4)
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_PIN_4); //


  DelayMilliSec(ui32DelayTime);                          // ��ʱui32DelayTime����     !!!!������
*/
}


//*****************************************************************************
//
// This function sets up UART0 to be used for a console to display information
// as the example is running.
//
//*****************************************************************************
void InitConsole(void)
{
  //
  // Enable GPIO port A which is used for UART0 pins.
  // TODO: change this to whichever GPIO port you are using.
  //
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

  //
  // Configure the pin muxing for UART0 functions on port A0 and A1.
  // This step is not necessary if your part does not support pin muxing.
  // TODO: change this to select the port/pin you are using.
  //
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);

  //
  // Enable UART0 so that we can configure the clock.
  //
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

  //
  // Select the alternate (UART) function for these pins.
  // TODO: change this to select the port/pin you are using.
  //
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  //
  // Initialize the UART for console I/O.
  //
  UARTStdioConfig(0, 115200, ui32SysClock);

}


void switch_100ms(void)	//100ms��������
{
  if(flag_100ms && ifis_on==1)
    {
      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x00);        // �ر� LED4(D4-PF0)
      ifis_on=0;
      flag_100ms=0;
    }
  else if(flag_100ms && ifis_on==0)
    {
      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0); // ���� LED4(D4-PF0)
      ifis_on=1;
      flag_100ms=0;
    }
}

void switch_6s(void)	//6s��������
{
  if(flag_6s && ifis_on==1)
    {
      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x00);        // �ر� LED4(D4-PF0)
      ifis_on=0;
      flag_6s=0;
    }
  else if(flag_6s && ifis_on==0)
    {
      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0); // ���� LED4(D4-PF0)
      ifis_on=1;
      flag_6s=0;
    }
}