/* This script is designed to handle user configurations and data for the alarm clock, as well as snoozing, silencing, and buzzing.
 *
 */

 #include <Arduino.h>
 #include <display_manager.h>
 #include <input_manager.h>
 #include <alarm.h>
 #include <time_config.h>
 #include <data_handling.h>

 int activeSlotIndex = 0;
 bool alarmDateBool = false;
 bool isBuzzerActive = false;
 unsigned long buzzerTurnOffTime = 0;
 bool isIndefiniteAlarm = false;
 int activeToneIndex = 0;

const int pwmChannel = 0;    
const int pwmResolution = 8;  

int activeMaxSnoozes = 0;      
int activeSnoozeMinutes = 0;   
int currentSnoozeCount = 0;  

const unsigned long DURATION_VALUES[] = { // "Time until silence" values in milliseconds.
    900000,   // 15 minutes
    1800000,  // 30 minutes
    3600000   // 60 minutes
};

int configSnoozeMinutes = 5; 

int configMaxSnoozes = 3; 

unsigned long snoozeEndTime = 0;
bool isSnoozing = false;

Alarm editAlarmBuffer;

const char* alarmTones[] = { "tone 1", "tone 2", "tone 3" };

int selectedToneIndex = 0; 

void snoozeAndSilence() {
    unsigned long currentMillis = millis();

    bool isButtonPressed = (digitalRead(INPUTS[BTN_SNOOZE_SILENCE].pin) == LOW);
    static bool lastButtonState = false;
    static unsigned long buttonPressedStartTime = 0;
    static bool holdActionExecuted = false;

    bool localSnoozeTriggered = false;
    bool localStopTriggered = false;

    if (isPreviewActive && (currentMillis >= previewTurnOffTime)) { // Set "isPreviewActive" flag to false when the tone demonstration runs its allotted time.
        isPreviewActive = false; 
        Serial.println("silence");
    }   
    
    if (isButtonPressed && !lastButtonState) {  // The button is not being held 
        buttonPressedStartTime = currentMillis;
        holdActionExecuted = false; 
    }
    
    if (isButtonPressed && lastButtonState) { // Check to see if the button is held for at least 2 seconds and then silence the alarm.
        if (!holdActionExecuted && (currentMillis - buttonPressedStartTime >= 2000)) {
            localStopTriggered = true; 
            holdActionExecuted = true; 
        }
    }

  
    if (!isButtonPressed && lastButtonState) { // Regular button press
        unsigned long holdDuration = currentMillis - buttonPressedStartTime;
        if (!holdActionExecuted && (holdDuration >= 50)) { // 50ms noise debounce floor
            localSnoozeTriggered = true;
        }
    }

    lastButtonState = isButtonPressed; 
  
    if ((isBuzzerActive || isSnoozing) && localStopTriggered) { // Completely silence the buzzer if the alarm is sounding/snozzing and was held for at least 2 seconds.
        isBuzzerActive = false;
        isIndefiniteAlarm = false;
        isSnoozing = false;
        currentSnoozeCount = 0; 
         currentUIState = STATE_DEFAULT; 
        return;
    }

  
    if (isBuzzerActive && localSnoozeTriggered) { // If the alarm is sounding and the "snooze" button is pressed
    
        if (activeMaxSnoozes != 0 && currentSnoozeCount >= activeMaxSnoozes) { // Completely silence the alarm if the amount of snoozes is at least the max amount allowed.
            isBuzzerActive = false;
            isIndefiniteAlarm = false;
            currentUIState = STATE_DEFAULT;
    
            return;
        }

        currentSnoozeCount++; 

        isBuzzerActive = false;
        isIndefiniteAlarm = false; 
        
        
        snoozeEndTime = currentMillis + ((unsigned long)activeSnoozeMinutes * 60 * 1000); 
        isSnoozing = true;

        Serial.println(currentSnoozeCount);
        return;
    }

    if (isSnoozing && (currentMillis >= snoozeEndTime)) {
        isSnoozing = false;
        isBuzzerActive = true; 

        if (isIndefiniteAlarm) {
        
            buzzerTurnOffTime = 0;
        } else {
    
            buzzerTurnOffTime = currentMillis + DURATION_VALUES[activeSlotIndex];
        }
        return;
    }


    if (isIndefiniteAlarm) return; 

    if (isBuzzerActive && (currentMillis >= buzzerTurnOffTime)) {
        isBuzzerActive = false; // Automatically shut off because user-configured time ran out
        currentUIState = STATE_DEFAULT; 
    
    }
}

void soundAlarm() {
    if (isBuzzerActive || isSnoozing) return;

    static unsigned long lastCheckTime = 0;
    if (millis() - lastCheckTime < 1000) return;
    lastCheckTime = millis();

    if (currentUIState == STATE_SCROLL_MENU) return;

    for (int i = 0; i < 3; i++) {
        if (alarmSlots[i].isEnabled && !alarmSlots[i].isDaily) { // If it's strictly a "time" alarm, only make sure time values agree.
            if (displayTime.tm_hour == alarmSlots[i].alarmTime.tm_hour &&
                displayTime.tm_min  == alarmSlots[i].alarmTime.tm_min  &&
                displayTime.tm_sec  == alarmSlots[i].alarmTime.tm_sec) {
                currentUIState = STATE_DEFAULT;

                activeToneIndex = alarmSlots[i].chosenAlarm; 
                isBuzzerActive = true;
            
                currentSnoozeCount = 0; 

              
                activeMaxSnoozes = alarmSlots[i].snoozesBeforeSilence;
                activeSnoozeMinutes = alarmSlots[i].snoozeDelay;

                int slotDurationChoice = alarmSlots[i].timeUntilSilence;
                if (slotDurationChoice == 3) {
                    isIndefiniteAlarm = true; 
                    buzzerTurnOffTime = 0; 
                } else {
                    isIndefiniteAlarm = false;
                    buzzerTurnOffTime = millis() + DURATION_VALUES[slotDurationChoice]; 
                }
                break;
            }
        }
        else if (alarmSlots[i].isEnabled && alarmSlots[i].isDaily) { // If it's a daily alarm, make sure the date values agree too.
            if (displayTime.tm_hour == alarmSlots[i].alarmTime.tm_hour &&
                displayTime.tm_min  == alarmSlots[i].alarmTime.tm_min  &&
                displayTime.tm_sec  == alarmSlots[i].alarmTime.tm_sec  &&
                displayTime.tm_year == alarmSlots[i].alarmTime.tm_year &&                                                     
                displayTime.tm_mon  == alarmSlots[i].alarmTime.tm_mon  &&                                                        
                displayTime.tm_mday == alarmSlots[i].alarmTime.tm_mday) {

                currentUIState = STATE_DEFAULT;
        
                activeToneIndex = alarmSlots[i].chosenAlarm; 
                isBuzzerActive = true;
            
                currentSnoozeCount = 0; 

              
                activeMaxSnoozes = alarmSlots[i].snoozesBeforeSilence;
                activeSnoozeMinutes = alarmSlots[i].snoozeDelay;

                int slotDurationChoice = alarmSlots[i].timeUntilSilence;
                 Serial.println("time till silence");
                Serial.println(alarmSlots[i].timeUntilSilence);
                if (slotDurationChoice == 0) {
                    isIndefiniteAlarm = true; 
                    buzzerTurnOffTime = 0; 
                } else {
                    isIndefiniteAlarm = false;
                    buzzerTurnOffTime = millis() + DURATION_VALUES[slotDurationChoice]; 
                }
                break;
            }
        }

    }
}

void playAlarmSound() {

    static int lastToneIndex = -1; // Reset lastToneIndex using '-1' so it doesnt stay at 0 if set to 0.
    static bool wasPlaying = false;

      if (!isBuzzerActive && !isPreviewActive) { // If neither the buzzer or preview is active at the moment, but the alarm was sounding, mute the buzzer and reset lastToneIndex.
        if (wasPlaying) {
            ledcWriteTone(0, 0);
            ledcWrite(0, 0);
            wasPlaying = false;
            lastToneIndex = -1;
        }
        return;
    }

     if (!isBuzzerActive && isPreviewActive) { // If the buzzer is not sounding and the user is previewing the tones, the active tones are the preview tones.
        activeToneIndex = previewToneIndex;
    }

    if (!wasPlaying || activeToneIndex != lastToneIndex) { // Ensures sure that two alarms cannot overlap
        switch (activeToneIndex) {
            case 0:
                ledcWriteTone(0, 1000); // Set PWM Channel 0 to 1k Hz.
                ledcWrite(0, 127); // Set duty cycle to 50%.    
                break;
            case 1:
                ledcWriteTone(0, 600); // Set PWM Channel 0 to 600 Hz.
                ledcWrite(0, 127);
                break;
            case 2:
                ledcWriteTone(0, 300); // Set PWM Channel 0 to 300 Hz.
                ledcWrite(0, 127);
                break;
            default:
                ledcWriteTone(0, 0); // // Set PWM Channel 0 to 0 Hz.
                ledcWrite(0, 0);
                wasPlaying = false;
                break;
        }
            wasPlaying = true;
            lastToneIndex = activeToneIndex; 
    }
}

void setAlarm() {
  bool currentButtonState = (digitalRead(INPUTS[BTN_SET_ALARM].pin) == LOW);
  static bool lastButtonState = false;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;

  if (currentButtonState != lastButtonState) {
    if (millis() - lastDebounceTime > debounceDelay) {
      lastDebounceTime = millis(); // Reset debounce timer

      if (currentButtonState == true) {
      
         if (currentUIState == STATE_SCROLL_MENU) { // If the "Set Alarm" button is pressed again, the alarm clock returns to the display
          currentUIState = STATE_DEFAULT;
         } else {
          currentUIState = STATE_SCROLL_MENU;
          isEditingAlarm = true; 
           Serial.print("in statescroll menu");
          // Draws the menu, allows the user to scroll the menu via the rotary encoder, and assings the alarm's configurations to the editBuffer.
          activeSlotIndex = currentMenuScrollPosition;
          editBuffer = alarmSlots[activeSlotIndex].alarmTime; 
          drawAlarmMenu;
         }
        updateDisplayFlag = true;
      }
    }
  }  
    
  lastButtonState = currentButtonState;
}

 void changeDateTime() { // Contains a switch statement that the user goes through sequentially when changing the date and time.

    Serial.println("In menu: Change date and time.");

    switch (currentUIState) {

    case STATE_EDIT_YEAR:
      
      currentUIState = STATE_EDIT_MONTH;
      break;
        
    case STATE_EDIT_MONTH:
   
      currentUIState = STATE_EDIT_DAY;
      break;
        
    case STATE_EDIT_DAY:
     
      currentUIState = STATE_EDIT_HOUR;
      break;

    case STATE_EDIT_HOUR:
     
      currentUIState = STATE_EDIT_MINUTES;
      break;

    case STATE_EDIT_MINUTES:

      currentUIState = STATE_EDIT_SECONDS;
      break;
    case STATE_EDIT_SECONDS:

       systemTime = editBuffer;

      mktime(&systemTime);

      writeUserTime(systemTime);

     currentUIState = STATE_DEFAULT; // Automatically return to the main display screen.
     break;
    }
     updateDisplayFlag = true;
  }

void alarmData() { // Contains a switch statement that the user goes through sequentially when configuring an alarm; includes a path for if the user wants to include a date component.

  Serial.println("In menu: Configure an alarm.");

  switch (currentUIState) {
    case STATE_SCROLL_MENU:
        Serial.println("in statescreen menu in alarm data"); 
        currentUIState = STATE_EDIT_HOUR;
        break;
        
    case STATE_EDIT_HOUR:
        currentUIState = STATE_EDIT_MINUTES;
        break;
        
    case STATE_EDIT_MINUTES:
        currentUIState = STATE_EDIT_SECONDS;
        break;
    case STATE_EDIT_SECONDS:
        currentUIState = STATE_EDIT_ALARMDATEBOOL;
        break;
        
    case STATE_EDIT_ALARMDATEBOOL:
        Serial.println("Add a date component to the alarm?");
        if (!alarmDateBool) {
            currentUIState = STATE_EDIT_SELECTALARM; 
            editAlarmBuffer.isDaily = false;
        } else {
            currentUIState = STATE_EDIT_YEAR;
            editAlarmBuffer.isDaily = true;
        }
        break;
        
    case STATE_EDIT_YEAR:
        currentUIState = STATE_EDIT_MONTH;
        break;
        
    case STATE_EDIT_MONTH:
        currentUIState = STATE_EDIT_DAY;
        break;
        
    case STATE_EDIT_DAY:
        currentUIState = STATE_EDIT_SELECTALARM;
        break;
    case STATE_EDIT_SELECTALARM:
        editAlarmBuffer.snoozeDelay = 5; // Snooze delay must start at 5.
        currentUIState = STATE_EDIT_SNOOZEDELAY;
        break;
        
    case STATE_EDIT_SNOOZEDELAY:
        currentUIState = STATE_EDIT_SNOOZESBEFORESILENCE;
        break;
        
    case STATE_EDIT_SNOOZESBEFORESILENCE:
        currentUIState = STATE_EDIT_TIMEUNTILSILENCE; 
        break;
        
    case STATE_EDIT_TIMEUNTILSILENCE:
        
    // Save the user configured alarm settings to the designated alarm slot.
    alarmSlots[activeSlotIndex] = editAlarmBuffer;
    alarmSlots[activeSlotIndex].alarmTime = editBuffer;
    
    // Enable the newly configured alarm and save it to the system's non-volatile storage.
    alarmSlots[activeSlotIndex].isEnabled = true;
    saveSystemSettings();

    // Clear the buffer data for the next alarm.
    editAlarmBuffer = Alarm(); 

    // Return to the display when done configuring the alarm.
    isEditingAlarm = false;
    currentUIState = STATE_DEFAULT;
        break;

    }
     updateDisplayFlag = true;
  }