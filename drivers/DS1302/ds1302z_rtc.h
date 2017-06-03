/*****************************************************************************/
/* File      : ds1302z_rtc.h                                                 */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-02-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef __DS1302Z_RTC_H__
#define __DS1302Z_RTC_H__

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
int RTC_Configuration(void);

int get_time(char* currenttime);
int set_time(const char* time);

#endif
