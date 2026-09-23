/* This script is designed to handle the time and date data that is manually entered by the user, 
 * including hours, minutes, and AM or PM, day, month, year, and day of the week.
 * This program also includes functions such as 12-24 hour formatting, DST configuration, and day of the week calculation.
 */

#include <RTClib.h>
#include <time_config.h>
#include <display_manager.h>
#include <alarm.h>
#include <input_manager.h>
#include <data_handling.h>

bool dstEnabled = false;
bool standardFormat = false;

String dayOfTheWeek(tm& displayTime) { // Algorithm that returns a day of the week given a date and time.
    switch (displayTime.tm_wday) {
        case 0: return "Sun";
        case 1: return "Mon";
        case 2: return "Tue";
        case 3: return "Wed";
        case 4: return "Thu";
        case 5: return "Fri";
        case 6: return "Sat";
        default: return "?";
    } 
}

void writeUserTime(tm userTime) {
    // This function formats user entered data in a DateTime structure
    rtc.adjust(DateTime(
      userTime.tm_year + 1900,
      userTime.tm_mon + 1,
      userTime.tm_mday,
      userTime.tm_hour,
      userTime.tm_min,
      userTime.tm_sec
    ));
}

void rtcFound() {
   if (! rtc.begin()) {
    Serial.println("RTC not found");
    for (;;); // Infinitely loop if RTC is not found
   }
}

void formatTime() { 
   bool currentButtonState = (digitalRead(INPUTS[BTN_FORMAT].pin) == LOW);
  static bool lastButtonState = false;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;

  if (currentButtonState == true && lastButtonState == false) {
    if (millis() - lastDebounceTime > debounceDelay) {
      lastDebounceTime = millis();

      if (currentUIState == STATE_DEFAULT) {
      
        standardFormat = !standardFormat;
      
        updateDisplayFlag = true; 

        saveSystemSettings();
      }
    }
  }
  lastButtonState = currentButtonState;
}


void clockUpdate() {
    if (currentUIState == STATE_DEFAULT) {
    
    DateTime now = rtc.now(); 

    static int lastSecond = -1;
    
    if (now.second() != lastSecond) {
      lastSecond = now.second();

      // Alarm clock's system time changes to reflect RTC data.
      systemTime.tm_sec  = now.second();
      systemTime.tm_min  = now.minute();
      systemTime.tm_hour = now.hour();
      systemTime.tm_mday = now.day();
      systemTime.tm_mon  = now.month() - 1;   
      systemTime.tm_year = now.year() - 1900; 

      updateDisplayFlag = true; // Update display to reflect change in time.
    }
  }
}

void dstConfigure() {
  bool currentButtonState = (digitalRead(INPUTS[BTN_DST].pin) == LOW);
  static bool lastButtonState = false;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;

  if (currentButtonState == true && lastButtonState == false) {
    if (millis() - lastDebounceTime > debounceDelay) {
      lastDebounceTime = millis();

      if (currentUIState == STATE_DEFAULT) { // Only allow the DST to be configured if the user is on the main display screen
      
        dstEnabled = !dstEnabled;
      
        updateDisplayFlag = true; 
        saveSystemSettings();
      }
    }
  }
  lastButtonState = currentButtonState;
}