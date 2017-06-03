#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "Flash_24L512.h"
#include "SEGGER_RTT.h"
#include "led.h"
#include "CMT2300.h"
#include "timer.h"
#include "string.h"





int main(void)
{	
	delay_init();	    	 					//��ʱ������ʼ��	  
	NVIC_Configuration(); 				//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	I2C_Init();										//FLASH оƬ��ʼ��
	LED_init();										//LED�Ƴ�ʼ��
	KEY_Init();										//�ָ��������ð�����ʼ��
	EXTIX_Init();									//�ָ���������IO�жϳ�ʼ��
	CMT2300_init();
	uart_init(57600);							//���ڳ�ʼ��
	TIM2_Int_Init(14999,7199);    //��������ʱ�¼���ʱ����ʼ��
	SEGGER_RTT_printf(0, "init OK \n");

	

	while(1)
	{

		delay_us(2);
	}

	/*
	while(1)
	{
		Write_24L512_nByte(0x000000,10,eepromSenddata);
		SEGGER_RTT_printf(0, "eepromSenddata:   %s\n",eepromSenddata);

		Read_24L512_nByte(0x000001,10,eepromRecvdata);

		eepromRecvdata[10] = '\0';
		SEGGER_RTT_printf(0, "eepromRecvdata:   %s\n",eepromRecvdata);
		delay_ms(1000);
		
	}	
	*/
	
	/*
	while(1)
	{		
	SEGGER_RTT_printf(0, "111111\n");
		SendMessage(eepromSenddata,31);
		RF_leng =1;
		delay_ms(2000);
		//RF_leng = GetMessage(eepromRecvdata);
		//if(RF_leng)
		//{
		//	RF_leng = 0;
		//	RFM300H_SW = 0;
		//}
	}
	*/
	/*
	while(1)
	{	
		LED_on();	
		delay_ms(1000);
		LED_off();	
		delay_ms(1000);			
	}
	*/
	
	/*	
	while(1)
	{	
		ret = WIFI_SendData((char *)eepromSenddata, 10);
		SEGGER_RTT_printf(0, "ret = %d\n",ret);
		delay_ms(1000);		
	}
	*/	
	
 }

