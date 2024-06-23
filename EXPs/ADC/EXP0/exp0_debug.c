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

//**************************************************************************************
//
// ͷ�ļ�
//
//**************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // ��ַ�궨��
#include "inc/hw_types.h"         // �������ͺ궨�壬�Ĵ������ʺ���
#include "driverlib/debug.h"      // ������
#include "driverlib/gpio.h"       // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"    // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"     // ϵͳ���ƺ궨��


#include "driverlib/uart.h"       // UART��غ궨��
#include "utils/uartstdio.h"      // UART0��Ϊ����̨��غ���ԭ������
                                  // uartstdio.h��uartstdio.c������utilsĿ¼��    
//**************************************************************************************
//
// �궨��
//
//**************************************************************************************
#define  MilliSecond      4000    // �γ�1msʱ������ѭ������ 
#define  FASTFLASHTIME    50	  // ����ʱ��50ms��
#define  SLOWFLASHTIME    500     // ����ʱ��500ms��

//**************************************************************************************
//
// ����ԭ������
//
//**************************************************************************************
void  DelayMilliSec(uint32_t ui32DelaySecond);		// �ӳ�һ��ʱ������λΪ����
void  GPIOInit(void);                               // GPIO��ʼ��
void  PF0Flash(uint8_t ui8KeyValue);      // ���ݴ���İ���ֵ������PF0����������
void  InitConsole(void);      // UART0��ʼ��


uint32_t g_ui32SysClock;
//**************************************************************************************
//
// ������
//
//**************************************************************************************
int main(void)
{
    uint8_t ui8KeyValue;
    
    // ʹ��16MHz�ڲ�ʱ��Դ��ϣ�����õ�ϵͳʱ��Ƶ��Ϊ16M
    // g_ui32SysClock���ʵ�����õ�ϵͳʱ��Ƶ��  
//    g_ui32SysClock = SysCtlClockFreqSet(SYSCTL_OSC_INT|SYSCTL_USE_OSC,16000000); 
    

	// ʹ���ⲿ25MHz��ʱ��Դ������PLL��Ȼ���ƵΪ20MHz
	g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | 
	                                   SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 
	                                   25000000);
    
    
    GPIOInit();             // GPIO��ʼ��

    #ifdef DEBUG_CONSOLE
    InitConsole();          // UART0��ʼ��   
    UARTprintf("Hello Everyone\n");
    UARTprintf("System Clock = %d Hz\n", g_ui32SysClock);
    #endif
 
    while(1)                // ����ѭ��
    {
                                        // ��ȡ PJ0 ��ֵ  0-���� 1-�ɿ�
        ui8KeyValue = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0); 

        #ifdef DEBUG_CONSOLE
        UARTprintf("PJ0= %d\n", ui8KeyValue);
        #endif        
        
        PF0Flash(ui8KeyValue);          // ���ݴ���İ�������������PF0����������
    }
}

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
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)){};  // �ȴ��˿� J׼�����
	
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

//**************************************************************************************
// 
// ����ԭ�ͣ�void PF0Flash(uint8_t ui8KeyValue)
// �������ܣ����ݴ���İ���ֵ������PF0������������0-������1-����
// ����������ui8KeyValue������ֵ
//
//**************************************************************************************
void PF0Flash(uint8_t ui8KeyValue)
{
    uint32_t ui32DelayTime;
	
    if (ui8KeyValue	== 0)                                  // PUSH1(USR_SW1-PJ0) ����
        ui32DelayTime = FASTFLASHTIME;
    else                                                   // PUSH1(USR_SW1-PJ0) �ɿ�
        ui32DelayTime = SLOWFLASHTIME;
		

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0); // ���� LED4(D4-PF0)
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4); // ���� LED3(D3-PF4)
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_PIN_0|GPIO_PIN_4); // ���� LED4(D4-PF0),LED3(D3-PF4)

//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_PIN_0); // 
    

    DelayMilliSec(ui32DelayTime);                          // ��ʱui32DelayTime���� 
		
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x00);        // �ر� LED4(D4-PF0)

//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x00);        // �ر� LED3(D3-PF4)
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, 0x00); // �ر� LED4(D4-PF0),LED3(D3-PF4)
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_PIN_4); // 

    
    DelayMilliSec(ui32DelayTime);                          // ��ʱui32DelayTime���� 
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
    UARTStdioConfig(0, 115200, g_ui32SysClock);
    
}