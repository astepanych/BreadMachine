#include "Rtc.h"

#include "stm32f4xx.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_pwr.h"
//#include "stm32f4xx_conf.h"
#include "system_stm32f4xx.h"

Rtc &Rtc::instance()
{
	static Rtc obj;
	return obj;
}


void Rtc::initRtc() {
	RTC_InitTypeDef rtc;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	RCC_BackupResetCmd(ENABLE);
	RCC_BackupResetCmd(DISABLE);
	
	//RCC_LSICmd(ENABLE);
	RCC_LSEConfig(RCC_LSE_ON);
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	RCC_RTCCLKCmd(ENABLE);
	
	while(!(RCC->BDCR&RCC_BDCR_LSERDY))
	      asm(" nop");
	
	RTC_StructInit(&rtc);
	rtc.RTC_HourFormat = RTC_HourFormat_24;
	//rtc.RTC_SynchPrediv = 0x7FFF;
	RTC_Init(&rtc);
}

void Rtc::getTime(uint8_t &h, uint8_t &m, uint8_t &s)
{
	RTC_TimeTypeDef time;
	RTC_GetTime(RTC_Format_BIN, &time);
	h = time.RTC_Hours;
	m = time.RTC_Minutes;
	s = time.RTC_Seconds;
	
}
void Rtc::getDate(uint16_t &y, uint8_t &m, uint8_t &d)
{
	RTC_DateTypeDef date;
	RTC_GetDate(RTC_Format_BIN, &date);
	y = date.RTC_Year;
	m = date.RTC_Month;
	d = date.RTC_Date;
}
void Rtc::setTime(uint8_t &h, uint8_t &m, uint8_t &s)
{
	RTC_TimeTypeDef time;
	
	time.RTC_Hours = h;
	time.RTC_Minutes = m;
	time.RTC_Seconds = s;
	RTC_SetTime(RTC_Format_BIN, &time);
}
void Rtc::setDate(uint8_t &y, uint8_t &m, uint8_t &d)
{
	RTC_DateTypeDef date;
	
	date.RTC_Year = y;
	date.RTC_Month = m;
	date.RTC_Date = d;
	RTC_SetDate(RTC_Format_BIN, &date);
}

uint8_t Rtc::getMinute()
{
	RTC_TimeTypeDef time;
	RTC_GetTime(RTC_Format_BIN, &time);
	return time.RTC_Minutes;
}
uint8_t Rtc::getHoure()
{
	RTC_TimeTypeDef time;
	RTC_GetTime(RTC_Format_BIN, &time);
	return time.RTC_Hours;
}