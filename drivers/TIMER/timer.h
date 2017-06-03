#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"


extern signed char COMM_Timeout_Event;

//串口接收超时中断
void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_Int_Deinit(void); 

//10分钟定时器 
void TIM2_Int_Init(u16 arr,u16 psc);	//	TIM2_Int_Init(14999,7199);    3S产生一个中断
void TIM2_Int_Deinit(void); 
void TIM2_Refresh(void);


#endif
