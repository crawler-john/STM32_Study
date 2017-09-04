/*****************************************************************************/
/* File      : usart.h                                                       */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-02 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef __USART5_H
#define __USART5_H
#include "stdio.h"	

typedef enum
{ 
    EN_RECV_ST_GET_SCOKET_HEAD 	= 0,	//接收Socket数据头
    EN_RECV_ST_GET_SCOKET_ID    = 1,	//接收手机Socket ID
    EN_RECV_ST_GET_A_HEAD      	= 2,	//接收报文数据头
    EN_RECV_ST_GET_A_LEN        = 3,	//接收报文数据长度   其中数据部分的长度为接收到长度减去12个字节
    EN_RECV_ST_GET_A_DATA       = 4,	//接收报文数据部分数据
    EN_RECV_ST_GET_A_END        = 5,	//接收报文END结尾标志
	
	EN_RECV_ST_GET_B_LEN        = 6,
	EN_RECV_ST_GET_B_DATA       = 7,
	
    EN_RECV_ST_GET_C_HEAD      	= 8,	//接收报文数据头
    EN_RECV_ST_GET_C_LEN        = 9,	//接收报文数据长度   其中数据部分的长度为接收到长度减去12个字节
    EN_RECV_ST_GET_C_DATA       = 10,	//接收报文数据部分数据
    EN_RECV_ST_GET_C_END        = 11,	//接收报文END结尾标志
	
} eRecvSM;// receive state machin


#define USART_REC_LEN  				2048  	//定义最大接收字节数 200
	  
extern unsigned char WIFI_RecvData[USART_REC_LEN];
extern unsigned char WIFI_Recv_Event;

unsigned short packetlen_A(unsigned char *packet);
unsigned short packetlen_C(unsigned char *packet);

int WIFI_SendData(char *data, int num);
void WIFI_GetEvent(int *messageLen,unsigned char *ID);

void uart5_init(u32 bound);

int AT(void);
int AT_ENTM(void);
int AT_WAP(char *ECUID12);
int AT_WAKEY(char *NewPasswd);
int WIFI_ChangePasswd(char *NewPasswd);
int WIFI_Reset(void);
int AT_Z(void);
int WIFI_ClearPasswd(void);
int WIFI_SoftReset(void);

int WIFI_Test(void);
int WIFI_Factory(char *ECUID12);
#endif


