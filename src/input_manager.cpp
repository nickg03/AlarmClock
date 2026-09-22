/* This script handles button inputs including the rotary encoder, switches, and photoresistor.
*/

#include <input_manager.h>
#include <display_manager.h>
#include <alarm.h>
#include <data_handling.h>

int CLK_current; // Initalizes variables that will hold the state of both encoder_clk and encoder_dt
int CLK_previous;

Alarm alarmSlots[3]; 
int counter = 0;
int DT_current;
int DT_previous;
int Photoresistor_value;
bool override;
bool lastButtonState;
bool isEditingAlarm;
bool alarmTrigger = false;
bool isPreviewActive = false;    // Turns menu sound wave generation ON/OFF
int previewToneIndex = 0;        // Holds the temporary tone index (0, 1, or 2)

unsigned long previewTurnOffTime = 0; 

static unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void initInputManager() {
    // Loop through the structures and extract the exact pin numbers
    for (int i = 0; i < NUM_INPUTS; i++) {
        pinMode(INPUTS[i].pin, INPUT_PULLUP);
    }
    CLK_previous = digitalRead(INPUTS[ENCODER_CLK].pin);

}

bool inputPressed(InputIndex btn) { // For buttons/switches. rotary encoder cw and ccw are handles separately
    if (digitalRead(INPUTS[btn].pin) == LOW) {
        return true;
    } else {
        return false;
    }
}

void encoderDirection() {
  // Read clk pin
    CLK_current = digitalRead(INPUTS[ENCODER_CLK].pin); 

    if (CLK_current == LOW && CLK_previous == HIGH) {
       override = true;
    // Determine direction of the turn
       bool isClockwise = (digitalRead(INPUTS[ENCODER_DT].pin) != CLK_current);
    // Serial.print(isClockwise);
       bool updateDisplay = false; //dont update since nothing has happened

       if (currentUIState != STATE_DEFAULT && currentUIState != STATE_SCROLL_MENU) {
          if (isClockwise) {
              if (currentUIState == STATE_EDIT_YEAR)  editBuffer.tm_year++;
              if (currentUIState == STATE_EDIT_MONTH) { editBuffer.tm_mon++; if(editBuffer.tm_mon > 11) editBuffer.tm_mon = 0; }
              if (currentUIState == STATE_EDIT_DAY)   { editBuffer.tm_mday++; if(editBuffer.tm_mday > 31) editBuffer.tm_mday = 1; }
              if (currentUIState == STATE_EDIT_HOUR)  { editBuffer.tm_hour++; if(editBuffer.tm_hour > 23) editBuffer.tm_hour = 0; }
              if (currentUIState == STATE_EDIT_MINUTES)   { editBuffer.tm_min++;  if(editBuffer.tm_min > 59)  editBuffer.tm_min = 0; }
              if (currentUIState == STATE_EDIT_SECONDS)   { editBuffer.tm_sec++;   if(editBuffer.tm_sec > 59)   editBuffer.tm_sec = 0; }
              if (currentUIState == STATE_EDIT_SNOOZEDELAY) { editAlarmBuffer.snoozeDelay++;   if(editAlarmBuffer.snoozeDelay > 15)   editAlarmBuffer.snoozeDelay = 5; }
              if (currentUIState == STATE_EDIT_SNOOZESBEFORESILENCE) { editAlarmBuffer.snoozesBeforeSilence++;   if(editAlarmBuffer.snoozesBeforeSilence > 10)   editAlarmBuffer.snoozesBeforeSilence = 0; }
              if (currentUIState == STATE_EDIT_TIMEUNTILSILENCE) 
              {
                  if (editAlarmBuffer.timeUntilSilence == 0) {      
                      editAlarmBuffer.timeUntilSilence = 15;
                  } 
                  else if (editAlarmBuffer.timeUntilSilence == 15) { 
                      editAlarmBuffer.timeUntilSilence = 30;
                  } 
                  else if (editAlarmBuffer.timeUntilSilence == 30) { 
                      editAlarmBuffer.timeUntilSilence = 60;
                  } 
                  else if (editAlarmBuffer.timeUntilSilence == 60) { 
                      editAlarmBuffer.timeUntilSilence = 0;
                  }
              }

              if (currentUIState == STATE_EDIT_SELECTALARM) { 
                  editAlarmBuffer.chosenAlarm++;
                  if(editAlarmBuffer.chosenAlarm > 2)  { 
                      editAlarmBuffer.chosenAlarm = 0; 

                  }
                  previewToneIndex = editAlarmBuffer.chosenAlarm; // Match the scrolled choice
                  previewTurnOffTime = millis() + 2000;           // Hard-coded 2-second window (glitchy)
                  isPreviewActive = true;    
              }
         }   

          else {
              if (currentUIState == STATE_EDIT_YEAR)  editBuffer.tm_year--;
              if (currentUIState == STATE_EDIT_MONTH) { editBuffer.tm_mon--; if (editBuffer.tm_mon < 0) editBuffer.tm_mon = 11; }
              if (currentUIState == STATE_EDIT_DAY)   { editBuffer.tm_mday--;   if(editBuffer.tm_mday < 1)   editBuffer.tm_mday = 31; }
              if (currentUIState == STATE_EDIT_HOUR)  { editBuffer.tm_hour--;  if(editBuffer.tm_hour < 0)   editBuffer.tm_hour = 23; }
              if (currentUIState == STATE_EDIT_MINUTES)   { editBuffer.tm_min--;   if(editBuffer.tm_min < 0)   editBuffer.tm_min = 59; }
              if (currentUIState == STATE_EDIT_SECONDS)   { editBuffer.tm_sec--;   if(editBuffer.tm_sec < 0)   editBuffer.tm_sec = 59; }
              if (currentUIState == STATE_EDIT_SNOOZEDELAY) { editAlarmBuffer.snoozeDelay--;   if(editAlarmBuffer.snoozeDelay < 5)   editAlarmBuffer.snoozeDelay = 15; }
              if (currentUIState == STATE_EDIT_SNOOZESBEFORESILENCE) { editAlarmBuffer.snoozesBeforeSilence--;   if(editAlarmBuffer.snoozesBeforeSilence < 0)   editAlarmBuffer.snoozesBeforeSilence = 10; }
              if (currentUIState == STATE_EDIT_TIMEUNTILSILENCE) 
              {
                  if (editAlarmBuffer.timeUntilSilence == 0) {     
                      editAlarmBuffer.timeUntilSilence = 60;
                  } 
                  else if (editAlarmBuffer.timeUntilSilence == 60) { 
                      editAlarmBuffer.timeUntilSilence = 30;
                  } 
                  else if (editAlarmBuffer.timeUntilSilence == 30) { 
                      editAlarmBuffer.timeUntilSilence = 15;
                  } 
                  else if (editAlarmBuffer.timeUntilSilence == 15) { 
                      editAlarmBuffer.timeUntilSilence = 0;
                  }
              }

             if (currentUIState == STATE_EDIT_SELECTALARM) {
                 if (editAlarmBuffer.chosenAlarm == 0) {
                    editAlarmBuffer.chosenAlarm = 2;
                } else {
                    editAlarmBuffer.chosenAlarm--;
 
                  }
                     previewToneIndex = editAlarmBuffer.chosenAlarm; // Match the scrolled choice
                     previewTurnOffTime = millis() + 2000;               // Hard-coded 2-second window
                     isPreviewActive = true;   
                  // playAlarmSound(editAlarmBuffer.chosenAlarm, 20000, alarmTrigger);
              } 
        }
        updateDisplayFlag = true;
    }

    switch (currentUIState) {
      
      case STATE_DEFAULT:
        // Default screen, turning the knob dims or brightens the display
        if (isClockwise) {
          Serial.println("illuminate");
          displayDim(false, true); // Go backward toward full illumination (2 -> 1 -> 0)
        } else {
          Serial.println("dim");
          displayDim(true, true);  // Go further into dimming (0 -> 1 -> 2)
        }
        break;

      case STATE_SCROLL_MENU:
        // In a menu list
        if (isClockwise) {
          currentMenuScrollPosition++;
        } else {
          currentMenuScrollPosition--;
        }

        if (currentMenuScrollPosition > 2) currentMenuScrollPosition = 2;
        if (currentMenuScrollPosition < 0) currentMenuScrollPosition = 0;

        updateDisplayFlag = true;
        break;

      case STATE_EDIT_ALARMDATEBOOL:
        if (isClockwise) {
          alarmDateBool = false;
        } else {
          alarmDateBool = true;
          Serial.println("no");
        }
        break;
    }

  }
  CLK_previous = CLK_current;
}

void automaticDimming() {
    Photoresistor_value = analogRead(3); // Reads photoresistor input
    if (Photoresistor_value < 1000) { // Automatic dimming, this can be cleaned up
        displayDim(true, false);
        override = false; // Override waits until the user covers the photoresistor again to reactivate its capabilities
    }
    else if (Photoresistor_value >= 1000 && (override == false)) {
        displayDim(false, false);
    } 
    //updateDisplayFlag = true;
}

void encoderSwitch() {
    bool currentButtonState = (digitalRead(INPUTS[ENCODER_SWITCH].pin) == LOW);
    if (currentButtonState != lastButtonState) {
        if (millis() - lastDebounceTime > debounceDelay) {
            lastDebounceTime = millis(); // Reset debounce timer
    
            if (currentButtonState == true) {

                switch (currentUIState) {
      
                case STATE_DEFAULT:
                  isEditingAlarm = false; 
                  editBuffer = systemTime;
                  currentUIState = STATE_EDIT_YEAR;
                  break;

                case STATE_SCROLL_MENU:
                    activeSlotIndex = currentMenuScrollPosition;
                    editBuffer = alarmSlots[activeSlotIndex].alarmTime; 
                    currentUIState = STATE_EDIT_HOUR;  
                    break;

                case STATE_EDIT_HOUR:
                case STATE_EDIT_MINUTES:
                case STATE_EDIT_SECONDS:
                case STATE_EDIT_ALARMDATEBOOL:
                case STATE_EDIT_YEAR:
                case STATE_EDIT_MONTH:
                case STATE_EDIT_DAY:
                case STATE_EDIT_SELECTALARM:
                case STATE_EDIT_SNOOZEDELAY:
                case STATE_EDIT_SNOOZESBEFORESILENCE:
                case STATE_EDIT_TIMEUNTILSILENCE:
                  if (isEditingAlarm) {
                      alarmData();   
                  }  
                  else {
                    //Serial.println("in place above change date time");
                    changeDateTime();
                  }
                  break;
               }
            updateDisplayFlag = true;
              }
          }
      }     
    lastButtonState = currentButtonState;
}