#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>

extern tm systemTime;
extern tm editBuffer;


extern int activeSlotIndex; 

extern bool isIndefiniteAlarm;

extern int snoozeDelay;
extern int snoozesBeforeSilence;
extern int timeUntilSilence;
extern bool isEditingAlarm;
extern bool alarmTrigger;
extern bool isBuzzerActive;
extern unsigned long buzzerTurnOffTime;
extern int activeToneIndex;
extern unsigned long buzzerTurnOffTime;
extern unsigned long snoozeEndTime;
extern bool isSnoozing;
extern bool isPreviewActive;  
extern int previewToneIndex;      
extern unsigned long previewTurnOffTime;

extern int activeMaxSnoozes;      
extern int activeSnoozeMinutes;   
extern int currentSnoozeCount;   

extern bool alarmDateBool; 

extern const char* alarmTones[3];

void buzzerSound();
void snoozeAndSilence();
void setAlarm();
void changeDateTime();
void playAlarmSound();
void soundAlarm();
void alarmData();
void snoozeAndAlarmDisplayOverlay();
//void updateBuzzerState();

#endif // ALARM_H