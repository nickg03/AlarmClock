#include <Arduino.h>
#include <Wire.h>
#include <input_manager.h>
#include <display_manager.h>
#include <time_config.h>
#include <RTClib.h>
#include <alarm.h>
#include <data_handling.h>

UIState currentUIState = STATE_DEFAULT;

RTC_DS3231 rtc;

tm editBuffer;
tm systemTime;

#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9

void setup() {
    Serial.begin(115200);

    loadSystemSettings(); // Load the saved system configurations set by the user.

    initInputManager(); // Initiate all assigned user inputs.

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN); // Initialize I2C communication for assigned pins.

    ledcSetup(0, 2000, 8); // "Open" LEDC Channel 0 for configuration, 2k Hz @ 8-bit depth.

    ledcAttachPin(buzzer, 0); // Map the GPIO pin assigned to the buzzer to LEDC Channel 0.
    
    ledcWrite(0, 0); // Set duty cycle to 0% so it stays completely silent on startup.

    rtcFound(); // Verify that the RTC exists.

    displayFound(); // Verify that the display exists.
    
    displayClear(); // Clear the display.

    clockUpdate(); // Initial clock update that writes the values upon startup.

    updateDisplay(); // Initial updateDisplay call that updates the display upon startup.

}   

void loop() {

    automaticDimming(); // Handles automatic dimming using photoresistor data.

    encoderDirection(); // Handles the rotary encoder dial input according to the UI state.

    encoderSwitch(); // Handles the rotary encoder switch input according to the UI state; also allows user to edit display date/time.

    dstConfigure(); // Toggles the display time between DST and no DST.

    formatTime(); // Toggles the display time between 12 and 24 hour format mode.

    soundAlarm(); // Iterates through user-set alarms and checks whether any of the configured times agree with the display time; if yes, then the alarm is sounded.

    playAlarmSound(); // Contains a switch statement of tones that the user is able to choose from; handles everything related to the Piezo buzzer.

    snoozeAndSilence(); // Handles the snooze and silence logic for the alarms. 

    setAlarm(); // Allows the user to configure 3 different alarms, including snooze configurations for each.

   if (inputPressed(BTN_RST)) { // Allows the user to restart the ESP32.
        displayString("Restarting", 1, 0, 0, true);
        ESP.restart();
    } else {
        // Do nothing.
    }

    clockUpdate(); // Function that updates the clock display via the RTC. Must always run.

    updateDisplay(); // Function that continuously updates the display using a flag variable.

}