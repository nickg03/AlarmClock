#ifndef TIME_CONFIG_H
#define TIME_CONFIG_H

#include <ctime>
#include <RTClib.h>

extern RTC_DS3231 rtc;

String dayOfTheWeek(tm& displayTime);
void writeUserTime(tm userTime);
void dstConfigure();
void formatTime(); 
void rtcFound();
void clockUpdate();

#endif // TIME_CONFIG_H