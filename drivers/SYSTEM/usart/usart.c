#include "sys.h"
#include "usart.h"	
#include "SEGGER_RTT.h"
#include "timer.h"
#include "string.h"

unsigned short packetlen(unsigned char *packet)
{
	unsigned short len = 0;
	len = ((packet[0]-'0')*1000 +(packet[1]-'0')*100 + (packet[2]-'0')*10 + (packet[3]-'0'));
	return len;
}

//WIFI发送函数 
int WIFI_SendData(char *data, int num)
{      
	int index = 0;
	char ch = 0;
	for(index = 0;index < num;index++)
	{
		ch = data[index];
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); 
    USART_SendData(USART2,(uint8_t)ch);
	}
	return index;
}

 
#if EN_USART2_RX   //如果使能了接收
//初始化IO 串口1 
//bound:波特率
void uart_init(u32 bound){
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能串口时钟USART2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能GPIOA时钟
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART2|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART2，GPIOA时钟
 	USART_DeInit(USART2);  //复位串口1
	 //USART2_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
   
    //USART2_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART2, &USART_InitStructure); //初始化串口
#if EN_USART2_RX		  //如果使能了接收  
   //USART2 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
   
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
#endif
    USART_Cmd(USART2, ENABLE);                    //使能串口 

}

//串口2中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	

unsigned char WIFI_RecvData[USART_REC_LEN] = {'\0'};
unsigned char WIFI_Recv_Event = 0;

unsigned char USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
unsigned short Cur = 0;		//当前采值位置
unsigned short PackLen = 0;
eRecvSM eStateMachine = EN_RECV_ST_GET_HEAD;	//数据采集状态机
unsigned short pos = 0;				//数据解析位置
unsigned short mvsize = 0;

void USART2_IRQHandler(void)                	//串口1中断服务程序
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		USART_RX_BUF[Cur] = USART_ReceiveData(USART2);//(USART2->DR);	//读取接收到的数据
		//SEGGER_RTT_printf(0, "%x %c\n",USART_RX_BUF[Cur],USART_RX_BUF[Cur]);
		
		Cur +=1;
    pos = 0;

		
		//receive start character
		if(eStateMachine == EN_RECV_ST_GET_HEAD)    //接收报文头部
		{
			////SEGGER_RTT_printf(0, "EN_RECV_ST_GET_HEAD\n");
			// check for the start character(SYNC_CHARACTER)
      // also check it's not arriving the end of valid data
      while(pos < Cur)
      {
				TIM3_Int_Deinit();
				mvsize = Cur - pos;		//当前第几个字节
				if(1 == mvsize)   //'A'
				{
						if(USART_RX_BUF[0] != 'A')
						{
							Cur = 0;
							pos = 0;
						}
				}
				
				if(2 == mvsize)   //'P'
				{
						if(USART_RX_BUF[1] != 'P')
						{
							Cur = 0;
							pos = 0;
						}
				}	
				
				if(3 == mvsize)   //'S'
				{
						if(USART_RX_BUF[2] != 'S')
						{
							Cur = 0;
							pos = 0;
						}
				}
				
				if(5 == mvsize)   //接收版本号完毕
				{
					//SEGGER_RTT_printf(0, "APS11\n");
					eStateMachine = EN_RECV_ST_GET_LEN;

					TIM3_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
					break;
				}
				
				TIM3_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
				
				pos++;
			}
		}
		
		//receive data length
		if(eStateMachine == EN_RECV_ST_GET_LEN)
		{
			////SEGGER_RTT_printf(0, "EN_RECV_ST_GET_LEN\n");
			while(pos < Cur)
      {
				TIM3_Int_Deinit();
				mvsize = Cur - pos;		//当前第几个字节
				if(9 == mvsize)   //接收数据长度结束
				{
					PackLen = packetlen(&USART_RX_BUF[5]);
					//SEGGER_RTT_printf(0, "LENGTH : %d\n",PackLen);
					//计算长度
					eStateMachine = EN_RECV_ST_GET_DATA;

					TIM3_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
					break;
				}
				TIM3_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
				pos++;
			}
		}
		
		//Continue to receive data
		if(eStateMachine == EN_RECV_ST_GET_DATA)
		{
			////SEGGER_RTT_printf(0, "EN_RECV_ST_GET_DATA\n");
			while(pos < Cur)
      {
				TIM3_Int_Deinit();
				mvsize = Cur - pos;		//当前第几个字节
				if((PackLen - 3) == mvsize)   //接收数据长度结束
				{
					eStateMachine = EN_RECV_ST_GET_END;

					TIM3_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
					break;
				}
				TIM3_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
				pos++;
			}
		}
		
		
		
		//receive END
		if(eStateMachine == EN_RECV_ST_GET_END)
		{
			////SEGGER_RTT_printf(0, "EN_RECV_ST_GET_END\n");
			while(pos < Cur)
      {
				TIM3_Int_Deinit();
				mvsize = Cur - pos;		//当前第几个字节
				if((PackLen - 2) == mvsize)   //'A'
				{
						if(USART_RX_BUF[PackLen - 3] != 'E')
						{
							Cur = 0;
							pos = 0;
						}
				}
				
				if((PackLen - 1) == mvsize)   //'P'
				{
						if(USART_RX_BUF[PackLen - 2] != 'N')
						{
							Cur = 0;
							pos = 0;
						}
				}	
				
				if((PackLen) == mvsize)   //'S'
				{
						if(USART_RX_BUF[PackLen - 1] != 'D')
						{
							Cur = 0;
							pos = 0;
						}
						//SEGGER_RTT_printf(0, "EN_RECV_ST_GET_END OVER\n");
						
						//报文接收完毕
						//进行完毕的相应操作
						//将采集成功的数据复制到成功数组
						memset(WIFI_RecvData,0x00,USART_REC_LEN);
						memcpy(WIFI_RecvData,USART_RX_BUF,PackLen);
						WIFI_RecvData[PackLen] = '\0';
						WIFI_Recv_Event = 1;
						////SEGGER_RTT_printf(0, "WIFI_RecvData :%s\n",WIFI_RecvData);
						eStateMachine = EN_RECV_ST_GET_HEAD;
						Cur = 0;
						pos = 0;
						
						TIM3_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
						break;
				}
				
				TIM3_Int_Init(149,7199);//10Khz的计数频率，计数到5000为500ms 打开定时器
				
				pos++;
			}
		}

	}
} 
#endif	

