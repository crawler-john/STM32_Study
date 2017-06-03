/*****************************************************************************/
/* File      : ds1302z_rtc.c                                                 */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-02-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stm32f10x.h>
#include "string.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define DS1302CLK GPIO_Pin_0   //与时钟线相连的芯片的管脚
#define DS1302DAT GPIO_Pin_8   //与数据线相连的芯片的管脚
#define DS1302RST GPIO_Pin_2   //与复位端相连的芯片的管脚

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

void ds1302_writebyte(unsigned char dat)
{
	unsigned char i = 0;
	//设置为推免输出
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin =  DS1302DAT;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOC,DS1302CLK);           //初始时钟线置为0
	for(i=0;i<8;i++)    //开始传输8个字节的数据
	{
		
		if(dat&0x01)	//取最低位，注意 DS1302的数据和地址都是从最低位开始传输的
		{
			GPIO_SetBits(GPIOC,DS1302DAT);
		}else
		{
			GPIO_ResetBits(GPIOC,DS1302DAT);
		}
		GPIO_SetBits(GPIOC,DS1302CLK);       //时钟线拉高，制造上升沿，SDA的数据被传输
		GPIO_ResetBits(GPIOC,DS1302CLK);       //时钟线拉低，为下一个上升沿做准备
		dat>>=1;        //数据右移一位，准备传输下一位数据
	}
}

unsigned char ds1302_readbyte(void)
{
	unsigned char i = 0,dat = 0;
	//设置为上拉输入
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin =  DS1302DAT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;    
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	for(i=0;i<8;i++)
	{
		dat>>=1;        //要返回的数据左移一位
		if(GPIO_ReadInputDataBit(GPIOC,DS1302DAT) == 1)     //当数据线为高时，证明该位数据为 1
			dat|=0x80;  //要传输数据的当前值置为 1,若不是,则为 0
		GPIO_SetBits(GPIOC,DS1302CLK);       //拉高时钟线

		GPIO_ResetBits(GPIOC,DS1302CLK);       //制造下降沿
	}
	return dat;         //返回读取出的数据
}

unsigned char ds1302_read(unsigned char cmd)
{
	unsigned char data = 0;

	GPIO_ResetBits(GPIOC,DS1302RST);
	GPIO_ResetBits(GPIOC,DS1302CLK);	
	GPIO_SetBits(GPIOC,DS1302RST);	
	ds1302_writebyte(cmd);
	data = ds1302_readbyte();
	GPIO_SetBits(GPIOC,DS1302CLK);
	GPIO_ResetBits(GPIOC,DS1302RST);	

	return data;
}

void ds1302_write(unsigned char cmd, unsigned char data)
{
	GPIO_ResetBits(GPIOC,DS1302RST);
	GPIO_ResetBits(GPIOC,DS1302CLK);
	GPIO_SetBits(GPIOC,DS1302RST);		
	
	ds1302_writebyte(cmd);
	ds1302_writebyte(data);
	GPIO_SetBits(GPIOC,DS1302CLK);
	GPIO_ResetBits(GPIOC,DS1302RST);	
}



int get_time(char* currenttime)
{
	unsigned char year, month, day, hour, minute, second;
	char datetime[20] = {'\0'};
	char temp[5] = {'\0'};

	year = ds1302_read(0x8D);
	month = ds1302_read(0x89);
	day = ds1302_read(0x87);
	hour = ds1302_read(0x85);
	minute = ds1302_read(0x83);
	second = ds1302_read(0x81);

	strcat(datetime, "20");

	sprintf(temp, "%d", ((year >> 4) & 0x0f));
	strcat(datetime, temp);
	memset(temp, '\0', 5);

	sprintf(temp, "%d", (year & 0x0f));
	strcat(datetime, temp);
	memset(temp, '\0', 5);

	sprintf(temp, "%d", ((month >> 4) & 0x0f));
	strcat(datetime, temp);
	memset(temp, '\0', 5);

	sprintf(temp, "%d", (month & 0x0f));
	strcat(datetime, temp);
	memset(temp, '\0', 5);

	sprintf(temp, "%d", ((day >> 4) & 0x0f));
	strcat(datetime, temp);
	memset(temp, '\0', 5);

	sprintf(temp, "%d", (day & 0x0f));
	strcat(datetime, temp);
	memset(temp, '\0', 5);

	sprintf(temp, "%d", ((hour >> 4) & 0x07));
	strcat(datetime, temp);
	memset(temp, '\0', 5);

	sprintf(temp, "%d", (hour & 0x0f));
	strcat(datetime, temp);
	memset(temp, '\0', 5);

	sprintf(temp, "%d", ((minute >> 4) & 0x0f));
	strcat(datetime, temp);
	memset(temp, '\0', 5);

	sprintf(temp, "%d", (minute & 0x0f));
	strcat(datetime, temp);
	memset(temp, '\0', 5);

	sprintf(temp, "%d", ((second >> 4) & 0x0f));
	strcat(datetime, temp);
	memset(temp, '\0', 5);

	sprintf(temp, "%d", (second & 0x0f));
	strcat(datetime, temp);
	memset(temp, '\0', 5);

	memcpy(currenttime,datetime,strlen(datetime));
	
	return 0;
}

int set_time(const char* time)
{
	unsigned year, month, day, hour, minute, second;
	unsigned char datetime[20] = {'\0'};

	memcpy(datetime,time,14);

	year = ((datetime[2] - 0x30) << 4) | (datetime[3] - 0x30);
	month = ((datetime[4] - 0x30) << 4) | (datetime[5] - 0x30);
	day = ((datetime[6] - 0x30) << 4) | (datetime[7] - 0x30);
	hour = ((datetime[8] - 0x30) << 4) | (datetime[9] - 0x30);
	minute = ((datetime[10] - 0x30) << 4) | (datetime[11] - 0x30);
	second = ((datetime[12] - 0x30) << 4) | (datetime[13] - 0x30);

	ds1302_write(0x8C, year);
	ds1302_write(0x88, month);
	ds1302_write(0x86, day);
	ds1302_write(0x84, hour);
	ds1302_write(0x82, minute);
	ds1302_write(0x80, second);

  return 0;
}

/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : Configures the RTC.
* Input          : None
* Output         : None
* Return         : 0 reday,-1 error.
*******************************************************************************/
int RTC_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* PC 2,8,为输出*/
	GPIO_InitStructure.GPIO_Pin =  DS1302CLK | DS1302RST;   
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC,DS1302CLK);
	GPIO_SetBits(GPIOC,DS1302RST);
	
	ds1302_write(0x8e, 0x00);	//关闭写保护
  return 0;
}

