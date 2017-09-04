/*****************************************************************************/
/* File      : usart5.c                                                        */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-02 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "usart5.h"	
#include "SEGGER_RTT.h"
#include "timer.h"
#include "string.h"
#include "stm32f10x.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define WIFI_RCC                    RCC_APB2Periph_GPIOC
#define WIFI_GPIO                   GPIOC
#define WIFI_PIN                    (GPIO_Pin_6)

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
unsigned short packetlen_A(unsigned char *packet)
{
	unsigned short len = 0;
	len = ((packet[0]-'0')*1000 +(packet[1]-'0')*100 + (packet[2]-'0')*10 + (packet[3]-'0'));
	return len;
}

unsigned short packetlen_C(unsigned char *packet)
{
	unsigned short len = 0;
	int i = 0;
	for(i = 0;i < 5;i++)
	{
		if(packet[i] == 'A') packet[i] = '0';
	}
	len = ((packet[0]-'0')*10000 +(packet[1]-'0')*1000 + (packet[2]-'0')*100 + (packet[3]-'0')*10 + (packet[4]-'0'));
	return len;
}

//WIFI���ͺ��� 
int WIFI_SendData(char *data, int num)
{      
	int index = 0;
	char ch = 0;
	for(index = 0;index < num;index++)
	{
		ch = data[index];
		while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==RESET); 
    USART_SendData(UART5,(uint8_t)ch);
	}
	return index;
}

//��ʼ��IO ����5
//bound:������
void uart5_init(u32 bound){
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	//ʹ�ܴ���ʱ��UART5
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);	//ʹ��GPIOAʱ��

 	USART_DeInit(UART5);  //��λ����5
	 //UART5_TX   PC.12
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //PC.12
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOC, &GPIO_InitStructure); //��ʼ��PC12
   
    //UART5_RX	  PD.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOD, &GPIO_InitStructure);  //��ʼ��PD2
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(UART5, &USART_InitStructure); //��ʼ������

   //UART5 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
   
    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//�����ж�

    USART_Cmd(UART5, ENABLE);                    //ʹ�ܴ��� 


    RCC_APB2PeriphClockCmd(WIFI_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = WIFI_PIN;
    GPIO_Init(WIFI_GPIO, &GPIO_InitStructure);
	GPIO_SetBits(WIFI_GPIO, WIFI_PIN);

}

//WIFI  socket A
unsigned char WIFI_RecvData[USART_REC_LEN] = {'\0'};
unsigned char WIFI_Recv_SocketA_Event = 0;

unsigned char USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
unsigned short Cur = 0;		//��ǰ��ֵλ��
unsigned short PackLen = 0;
eRecvSM eStateMachine = EN_RECV_ST_GET_SCOKET_HEAD;	//���ݲɼ�״̬��
unsigned short pos = 0;				//���ݽ���λ��
unsigned short mvsize = 0;

void UART5_IRQHandler(void)                	//����1�жϷ������
{
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		USART_RX_BUF[Cur] = USART_ReceiveData(UART5);//(UART5->DR);	//��ȡ���յ�������
		//SEGGER_RTT_printf(0, "[%d] : %x %c\n",Cur,USART_RX_BUF[Cur],USART_RX_BUF[Cur]);
		Cur +=1;
		if(Cur >=USART_REC_LEN)
		{
			//SEGGER_RTT_printf(0, "%d \n",Cur);
			Cur = 0;
		}
	}
} 

void WIFI_GetEvent(int *messageLen,unsigned char *ID)
{
	  pos = 0;
		
		//����A����ͷ��
		if(eStateMachine == EN_RECV_ST_GET_A)
		{
			while(pos < Cur)
      {
				mvsize = Cur - pos;		//��ǰ�ڼ����ֽ�
				if(1 == mvsize)   //'a'
				{
						if(USART_RX_BUF[0] != 'a')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_A;
							break;
						}else
						{
							//SEGGER_RTT_printf(0, "a\n");
							delay_ms(2);
							eStateMachine = EN_RECV_ST_GET_ID;
							break;
						}
				}
				pos++;
			}
		}		
	
		//����ID
		if(eStateMachine == EN_RECV_ST_GET_ID)
		{
      while(pos < Cur)
      {
				//TIM3_Int_Deinit();

				if(2 == pos)
				{
					ID[0] = USART_RX_BUF[1];
				}
				
				if(3 == pos)
				{
					ID[1] = USART_RX_BUF[2];
				}	
				
				if(4 == pos)
				{
					ID[2] = USART_RX_BUF[3];
				}
				
				if(5 == pos) 
				{
					ID[3] = USART_RX_BUF[4];
				}
				
				if(6 == pos)
				{
					ID[4] = USART_RX_BUF[5];
				}
				
				if(7 == pos)
				{
					ID[5] = USART_RX_BUF[6];
				}
				
				if(8 == pos)   
				{
					ID[6] = USART_RX_BUF[7];
				}
				
				if(9 == pos)   //���հ汾�����
				{
					//SEGGER_RTT_printf(0, "ID\n");
					ID[7] = USART_RX_BUF[8];
					eStateMachine = EN_RECV_ST_GET_HEAD;
					//SEGGER_RTT_printf(0, "ID111 %x %x %x %x %x %x %x %x\n",ID[0],ID[1],ID[2],ID[3],ID[4],ID[5],ID[6],ID[7]);

					break;
				}
								
				pos++;
			}
		}			
	
		//receive start character
		if(eStateMachine == EN_RECV_ST_GET_HEAD)    //���ձ���ͷ��
		{
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_HEAD\n");
			// check for the start character(SYNC_CHARACTER)
      // also check it's not arriving the end of valid data
      while(pos < Cur)
      {
				//TIM3_Int_Deinit();
				mvsize = Cur - pos;		//��ǰ�ڼ����ֽ�
				if(10 == mvsize)   //'A'
				{
						if(USART_RX_BUF[9] != 'A')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_A;
							break;
						}
				}
				
				if(11 == mvsize)   //'P'
				{
						if(USART_RX_BUF[10] != 'P')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_A;
							break;
						}
				}	
				
				if(12 == mvsize)   //'S'
				{
						if(USART_RX_BUF[11] != 'S')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_A;
							break;
						}
				}
				
				if(14 == mvsize)   //���հ汾�����
				{
					//SEGGER_RTT_printf(0, "APS11\n");
					eStateMachine = EN_RECV_ST_GET_LEN;

					break;
				}
								
				pos++;
			}
		}
		
		//receive data length
		if(eStateMachine == EN_RECV_ST_GET_LEN)
		{
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_LEN\n");
			while(pos < Cur)
      {
				//�ж��Ƿ���a����  ����������жϺ���8���ֽ�
				mvsize = Cur - pos;		//��ǰ�ڼ����ֽ�
				if(18 == mvsize)   //�������ݳ��Ƚ���
				{
					PackLen = (packetlen(&USART_RX_BUF[14])+9);
					//SEGGER_RTT_printf(0, "LENGTH11111 : %d\n",PackLen);
					//���㳤��
					eStateMachine = EN_RECV_ST_GET_DATA;
					delay_ms(10);
					TIM3_Int_Init(299,7199);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms �򿪶�ʱ��

					break;
				}
				pos++;
			}
		}
		
		//Continue to receive data
		if(eStateMachine == EN_RECV_ST_GET_DATA)
		{
			pos = 0;
			while(pos < Cur)
      {

				if((PackLen - 3) == pos)   //�������ݳ��Ƚ���
				{
					eStateMachine = EN_RECV_ST_GET_END;
					//delay_ms(10);
					//TIM3_Int_Init(299,7199);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms �򿪶�ʱ��
					break;
				}
				pos++;
			}
		}		
		
		//receive END
		if(eStateMachine == EN_RECV_ST_GET_END)
		{
			pos = 0;
			//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_END\n");
			while(pos < Cur)
      {
				mvsize = Cur - pos;		//��ǰ�ڼ����ֽ�
				if((PackLen - 2) == mvsize)   //'A'
				{
						if(USART_RX_BUF[PackLen - 3] != 'E')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_A;
							break;
						}
				}
				
				if((PackLen - 1) == mvsize)   //'P'
				{
						if(USART_RX_BUF[PackLen - 2] != 'N')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_A;
							break;
						}
				}	
				
				if((PackLen) == mvsize)   //'S'
				{
						if(USART_RX_BUF[PackLen - 1] != 'D')
						{
							Cur = 0;
							pos = 0;
							eStateMachine = EN_RECV_ST_GET_A;
							break;
						}
						//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_END OVER\n");
						
						//SEGGER_RTT_printf(0, "ID %x %x %x %x %x %x %x %x\n",ID[0],ID[1],ID[2],ID[3],ID[4],ID[5],ID[6],ID[7]);
						
						//���Ľ������
						//������ϵ���Ӧ����
						//���ɼ��ɹ������ݸ��Ƶ��ɹ�����
						memset(WIFI_RecvData,0x00,USART_REC_LEN);
						memcpy(WIFI_RecvData,&USART_RX_BUF[9],(PackLen-9));
						//�������ݣ�ȥ���������
						*messageLen = PackLen-9;
						
						WIFI_RecvData[*messageLen] = '\0';
						WIFI_Recv_Event = 1;
						//SEGGER_RTT_printf(0, "WIFI_RecvData :%s\n",WIFI_RecvData);
						eStateMachine = EN_RECV_ST_GET_A;
						Cur = 0;
						pos = 0;		
						TIM3_Int_Deinit();
						break;
				}
								
				pos++;
			}
		}
		
}

void clear_WIFI(void)
{
	//TIM3_Int_Deinit();
	eStateMachine = EN_RECV_ST_GET_A;
	Cur = 0;
}

//����ATģʽ
int AT(void)
{
	clear_WIFI();
	//����ģ��д��"+++"Ȼ����д��"a" д��+++����"a" д��"a"����+ok
	WIFI_SendData("+++", 3);
	//��ȡ��a
	delay_ms(350);
	if(Cur <1)
	{
		return -1;
	}else
	{
		if(memcmp(USART_RX_BUF,"a",1))
		{
			return -1;
		}
	}
	
	//���ŷ���a
	clear_WIFI();
	WIFI_SendData("a", 1);
	delay_ms(350);
	if(Cur < 3)
	{
		return -1;
	}else
	{
		if(memcmp(USART_RX_BUF,"+ok",3))
		{
			return -1;
		}

	}
	SEGGER_RTT_printf(0, "AT :a+ok\n");
	clear_WIFI();
	return 0;
}


//�л���ԭ���Ĺ���ģʽ    OK
int AT_ENTM(void)
{

	clear_WIFI();
	//����"AT+ENTM\n",����+ok
	WIFI_SendData("AT+ENTM\n", 8);
	delay_ms(300);
	if(Cur < 10)
	{
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[9],"+ok",3))
		{
			return -1;
		}

	}
	SEGGER_RTT_printf(0, "AT+ENTM :+ok\n");
	clear_WIFI();
	return 0;
	
}

int AT_Z(void)
{

	clear_WIFI();
	//����"AT+Z\n",����+ok
	WIFI_SendData("AT+Z\n", 5);
	delay_ms(300);
	if(Cur < 6)
	{
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[6],"+ok",3))
		{
			return -1;
		}

	}
	SEGGER_RTT_printf(0, "AT+Z :+ok\n");
	clear_WIFI();
	return 0;
	
}

//����WIFI SSID

int AT_WAP(char *ECUID12)
{
	char AT[100] = { '\0' };
	clear_WIFI();
	//����"AT+WAKEY\n",����+ok
	sprintf(AT,"AT+WAP=11BGN,ECU_R_%s,Auto\n",ECUID12);
	SEGGER_RTT_printf(0, "%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	delay_ms(1000);
	
	if(Cur < 10)
	{
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[strlen(AT)+1],"+ok",3))
		{
			return -1;
		}
	}
	SEGGER_RTT_printf(0, "AT+WAP :+ok\n");
	clear_WIFI();
	return 0;
}

//����WIFI����
int AT_WAKEY(char *NewPasswd)
{
	char AT[100] = { '\0' };
	clear_WIFI();
	//����"AT+WAKEY\n",����+ok
	sprintf(AT,"AT+WAKEY=WPA2PSK,AES,%s\n",NewPasswd);
	SEGGER_RTT_printf(0, "%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	delay_ms(1000);
	
	if(Cur < 10)
	{
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[strlen(AT)+1],"+ok",3))
		{
			return -1;
		}
	}
	SEGGER_RTT_printf(0, "AT+WAKEY :+ok\n");
	clear_WIFI();
	return 0;
}

//����WIFI����
int AT_WAKEY_Clear(void)
{
	char AT[100] = { '\0' };
	clear_WIFI();
	//����"AT+WAKEY\n",����+ok
	sprintf(AT,"AT+WAKEY=OPEN,NONE\n");
	SEGGER_RTT_printf(0, "%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	
	delay_ms(1000);
	
	if(Cur < 10)
	{
		return -1;
	}else
	{
		if(memcmp(&USART_RX_BUF[strlen(AT)+1],"+ok",3))
		{
			return -1;
		}
	}
	SEGGER_RTT_printf(0, "AT+WAKEY Clear :+ok\n");
	clear_WIFI();
	return 0;
}



int WIFI_ChangePasswd(char *NewPasswd)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1) return -1;
	
	delay_ms(200);
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_WAKEY(NewPasswd);
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delay_ms(200);
			ret =AT_ENTM();;
			if(ret == 0) break;
		}
	
		return -1;
	}		
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_Z();
		if(ret == 0) return 0;
	}
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_ENTM();;
		if(ret == 0) break;
	}
	if(ret == -1) return -1;
	
	WIFI_Reset();	
	return 0;
}

int WIFI_Reset(void)
{
	GPIO_ResetBits(WIFI_GPIO, WIFI_PIN);
	
	delay_ms(1000);
	GPIO_SetBits(WIFI_GPIO, WIFI_PIN);
	return 0;
}

int WIFI_SoftReset(void)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delay_ms(200);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}	
	
	delay_ms(200);	
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_Z();
		if(ret == 0) return 0;
	}
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_ENTM();;
		if(ret == 0) break;
	}
	if(ret == -1) 
	{
		WIFI_Reset();	
		return -1;
	}
	
	return 0;
}

int WIFI_ClearPasswd(void)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delay_ms(200);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}	
	
	delay_ms(200);
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret = AT_WAKEY_Clear();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delay_ms(200);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}		
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_Z();
		if(ret == 0) return 0;
	}
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_ENTM();;
		if(ret == 0) break;
	}
	if(ret == -1) return -1;
	
	WIFI_Reset();	
	return 0;

}


int WIFI_Test(void)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delay_ms(200);
			ret =AT_ENTM();
			if(ret == 0)return 0;
		}
	
		return -1;
	}	
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_ENTM();
		if(ret == 0) return 0;
	}
	return -1;
}


int WIFI_Factory(char *ECUID12)
{
	int ret = 0,index;
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT();
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delay_ms(200);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}	
	
	delay_ms(200);
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret = AT_WAP(ECUID12);
		ret = AT_WAKEY("88888888");
		if(ret == 0) break;
	}
	if(ret == -1)
	{
		for(index = 0;index<3;index++)
		{
			delay_ms(200);
			ret =AT_ENTM();
			if(ret == 0) break;
		}
	
		return -1;
	}		
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_Z();
		if(ret == 0) return 0;
	}
	
	for(index = 0;index<3;index++)
	{
		delay_ms(200);
		ret =AT_ENTM();;
		if(ret == 0) break;
	}
	if(ret == -1) return -1;
	
	WIFI_Reset();	
	return 0;

}
