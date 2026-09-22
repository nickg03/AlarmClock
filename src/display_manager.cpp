/* This script is designed to handle the OLED display of the alarm clock, including all UI functionality.
 * This script receives information and updates the OLED display accordingly.
 */

#include <display_manager.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time_config.h>
#include <alarm.h>
#include <elapsedMillis.h>
#include <data_handling.h>

#define OLED_VCC_PIN 18

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

tm displayTime = systemTime;
bool updateDisplayFlag = false;

elapsedMillis sinceLastFlicker;

bool displayIsFlickering;
bool displayOn;
int maxFlickerTime;

bool dimmed = false;
bool screenOff = false;
int dimState = 0; // 0 is fully illuminated
int oldState;

char alarmMenu[3][16] = {
  "Alarm 1",
  "Alarm 2",
  "Alarm 3"
};

int alarmCount = 3;
int currentMenuScrollPosition = 0;

bool displayFound() { 
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Infinitely loop if the display is not found.
    }
    displayClear();
    return true;
}

void displayClear() {
    display.clearDisplay();
    display.display();
}

void displayString(String message, int textSize, int cursorX, int cursorY, bool clearDisplay) { // Function to print a string to the oled with some parameters
    if (clearDisplay) {
        display.clearDisplay();
    }
    display.setTextSize(textSize);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(cursorX, cursorY);
    display.println(message);
}


void updateDisplay() {
  if (!updateDisplayFlag) return; // Return if updateflag was not updated to true

  char timeBuffer[32]; // Stores characters

  displayTime = systemTime; 

  if (dstEnabled) {
    displayTime.tm_hour += 1;
    mktime(&displayTime); 
  }

  switch (currentUIState) {
    case STATE_SCROLL_MENU:
        drawAlarmMenu();
        break;
    case STATE_DEFAULT:
      if (isSnoozing) {
            displayClear();
            
            displayString("Snoozing... zZz", 1, 20, 10, false);
        
            unsigned long currentMillis = millis();
            unsigned long remainingSecs = (snoozeEndTime > currentMillis) ? (snoozeEndTime - currentMillis) / 1000 : 0;
            snprintf(timeBuffer, sizeof(timeBuffer), "%02lu:%02lu", remainingSecs / 60, remainingSecs % 60);
            displayString(timeBuffer, 2, 35, 28, false);
            
            displayString("Tap again to Snooze", 1, 10, 52, false);
            break;
        }

        if (isBuzzerActive) {
            flickerDisplay(); 
        }
      display.clearDisplay();

      if (standardFormat) { 
        int hour12 = displayTime.tm_hour % 12;
        if (hour12 == 0) hour12 = 12;
          const char* period = (displayTime.tm_hour >= 12) ? "PM" : "AM";
          snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d %s %s", hour12, displayTime.tm_min, displayTime.tm_sec, period, dayOfTheWeek(displayTime));
        } else {
          snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d %s", displayTime.tm_hour, displayTime.tm_min, displayTime.tm_sec, dayOfTheWeek(displayTime));
        }

      displayString(timeBuffer, 2, 16, 10, false);
      snprintf(timeBuffer, sizeof(timeBuffer), "%04d/%02d/%02d", displayTime.tm_year + 1900, displayTime.tm_mon + 1, displayTime.tm_mday);
      displayString(timeBuffer, 1, 32, 45, false);
      break;
    case STATE_EDIT_YEAR:
      displayString("SET YEAR:", 1, 0, 0, true); 
      snprintf(timeBuffer, sizeof(timeBuffer), "%04d", editBuffer.tm_year + 1900); 
      displayString(timeBuffer, 3, 0, 16, false); 
      display.display();
      break;

    case STATE_EDIT_MONTH:
      displayString("SET MONTH:", 1, 0, 0, true); 
      snprintf(timeBuffer, sizeof(timeBuffer), "%02d", editBuffer.tm_mon + 1);  // Add 1 since month does not start at 0
      displayString(timeBuffer, 3, 0, 16, false); 
      display.display();
      break;

    case STATE_EDIT_DAY:
      displayString("SET DAY:", 1, 0, 0, true); 
      snprintf(timeBuffer, sizeof(timeBuffer), "%02d", editBuffer.tm_mday); 
      displayString(timeBuffer, 3, 0, 16, false); 
      display.display();
      break;

    case STATE_EDIT_HOUR:
      displayString("SET HOUR:", 1, 0, 0, true); 
      snprintf(timeBuffer, sizeof(timeBuffer), "%02d", editBuffer.tm_hour); 
      displayString(timeBuffer, 3, 0, 16, false);
      display.display();
      break;

    case STATE_EDIT_MINUTES:
      displayString("SET MINUTE:", 1, 0, 0, true); 
      snprintf(timeBuffer, sizeof(timeBuffer), "%02d", editBuffer.tm_min); 
      displayString(timeBuffer, 3, 0, 16, false); 
      display.display();
      break;
    case STATE_EDIT_SECONDS:
      displayString("SET SECOND:", 1, 0, 0, true); 
      snprintf(timeBuffer, sizeof(timeBuffer), "%02d", editBuffer.tm_sec); 
      displayString(timeBuffer, 3, 0, 16, false); 
      display.display();
      break;
    case STATE_EDIT_SELECTALARM:
        displayString("SELECT ALARM:", 1, 0, 0, true);
        snprintf(timeBuffer, sizeof(timeBuffer), "%02d", editAlarmBuffer.chosenAlarm + 1);
        displayString(timeBuffer, 3, 0, 16, false);
        display.display();
        break;
   case STATE_EDIT_SNOOZEDELAY:
        displayString("TIME BTWN SNOOZES:", 1, 0, 0, true);
        snprintf(timeBuffer, sizeof(timeBuffer), "%02d MINS", editAlarmBuffer.snoozeDelay);
        displayString(timeBuffer, 3, 0, 16, false);
        display.display();
        break;

    case STATE_EDIT_SNOOZESBEFORESILENCE:
        displayString("SNOOZES TIL SILENCE:",1, 0, 0, true);
        
        if (editAlarmBuffer.snoozesBeforeSilence == 0) {
            snprintf(timeBuffer, sizeof(timeBuffer), "INDEFINITE");
        } else {
            snprintf(timeBuffer, sizeof(timeBuffer), "%02d TIMES", editAlarmBuffer.snoozesBeforeSilence);
        }
        
        displayString(timeBuffer, 2, 0, 16, false);
        display.display();
        break;

    case STATE_EDIT_TIMEUNTILSILENCE:
        displayString("DURATION TIL SILENCE:", 1, 0, 0, true);

         if (editAlarmBuffer.timeUntilSilence == 0) {
            snprintf(timeBuffer, sizeof(timeBuffer), "INDEFINITE");
        } else {
            snprintf(timeBuffer, sizeof(timeBuffer), "%02d MINS", editAlarmBuffer.timeUntilSilence);
        }
        displayString(timeBuffer, 2, 0, 16, false);
        display.display();
        break;

    case STATE_EDIT_ALARMDATEBOOL:
      displayString("ADD A DATE?:", 1, 0, 0, true);
    if (alarmDateBool) {
        displayString("> YES", 2, 0,  20, false);
        displayString("  NO",  2, 70, 20, false); 
    } else {
        displayString("  YES", 2, 0,  20, false);
        displayString("> NO",  2, 70, 20, false); 
    }
    display.display();
    break;
  }

  if (currentUIState != STATE_DEFAULT && currentUIState != STATE_SCROLL_MENU) {
    displayString("[Select to Confirm]", 1, 0, 55, false);
  }
  display.display();
  updateDisplayFlag = false; 
}

void displayDim(bool dimFurther, bool isManual) { 
   oldState = dimState;

   if (isManual) {
    if (dimFurther) {
      dimState++; 
      if (dimState > 2) dimState = 2; // Off stays off.
    } else {
      dimState--;
      if (dimState < 0) dimState = 0; // Max stays max.
    }
  } 
  else {
    if (dimState != 2) { 
      if (dimFurther) {
        dimState = 1; // If photoresistor is reading the dim threshold then dim.
      } else {
        dimState = 0; // If photoresistor is reading the illuminate threshold then illuminate.
      }
    }
  }

  // Only changes if dimState and oldState are different, suggesting that the user has turned the knob.
  if (dimState != oldState) {
    switch (dimState) {
      
      case 0: // Dimmed to fully illuminate.
        display.ssd1306_command(SSD1306_DISPLAYON);      // Turn screen on.
        display.ssd1306_command(SSD1306_SETCONTRAST); 
        display.ssd1306_command(255);                    // Set display to max brightness.
        break;

      case 1: // Moving from either off or fully illuminated into dim.
        display.ssd1306_command(SSD1306_DISPLAYON);      // Turn screen on.
        display.ssd1306_command(SSD1306_SETCONTRAST); 
        display.ssd1306_command(1);                      // Dim the display.
        break;

      case 2: // Dimmed to off.
        display.ssd1306_command(SSD1306_DISPLAYOFF); // Display turns off for a "sleep mode".
        break;
    }
  }
    
}

void flickerDisplay() {
   if (!isBuzzerActive) {
        if (!displayOn) {
            display.ssd1306_command(SSD1306_DISPLAYON);
            displayOn = true;
        }
        return; 
    }

    if (sinceLastFlicker >= 500) { // Supposed to flicker every 500 ms. Does not do that. Could not figure it out.
        sinceLastFlicker = 0; 

        if (displayOn) {
            display.ssd1306_command(SSD1306_DISPLAYOFF);
            displayOn = false;
        } else {
            display.ssd1306_command(SSD1306_DISPLAYON);
            displayOn = true;
        }
    }
}

  void drawAlarmMenu() {

    displayString("--- SELECT ALARM ---", 1, 0, 0, true);

    for (int i = 0; i < 3; i++) {
        char slotText[24];
        int yPosition = 16 + (i * 12); // Stack rows neatly down the screen (Y: 16, 28, 40).

        // Draw a selection arrow '>' only next to the active scroll position.
        if (i == currentMenuScrollPosition) {
            displayString(">", 1, 0, yPosition, false);
        } else {
            displayString(" ", 1, 0, yPosition, false);
        }

        // Format the alarm clock slot according to whether its empty or configured and daily or not daily.
        if (alarmSlots[i].isEnabled && !alarmSlots[i].isDaily) {
           // If active, print the saved time configuration 
            snprintf(slotText, sizeof(slotText), "Alm %d: %02d:%02d", 
            i + 1, 
            alarmSlots[i].alarmTime.tm_hour, 
            alarmSlots[i].alarmTime.tm_min);
        } else  if (alarmSlots[i].isEnabled && alarmSlots[i].isDaily) {
            snprintf(slotText, sizeof(slotText), "Al %d: %02d:%02d %02d/%02d/%02d",
            i + 1,
            alarmSlots[i].alarmTime.tm_hour,
            alarmSlots[i].alarmTime.tm_min,
            alarmSlots[i].alarmTime.tm_mon + 1,  
            alarmSlots[i].alarmTime.tm_mday,     
            alarmSlots[i].alarmTime.tm_year % 100); 
        } else {
            // If empty, show a placeholder slot.
            snprintf(slotText, sizeof(slotText), "Alm %d: --:-- [EMPTY]", i + 1);
        }

        displayString(slotText, 1, 8, yPosition, false);
    }

}

