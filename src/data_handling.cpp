#include <Arduino.h>
#include <Preferences.h>
#include "data_handling.h"

static Preferences prefs;

void saveSystemSettings() { // Save all of the user configured data to the non-volatile storage.
    prefs.begin("clockSettings", false);
    prefs.putBool("timeFormat", standardFormat);
    prefs.putBytes("alarms", &alarmSlots, sizeof(alarmSlots));
    prefs.end();
    Serial.println("alarm configurations saved");
}

void loadSystemSettings() { // Load all of the saved user configurations.
    prefs.begin("clockSettings", true);
    standardFormat = prefs.getBool("timeFormat", true);

    if (prefs.isKey("alarms")) {
        prefs.getBytes("alarms", &alarmSlots, sizeof(alarmSlots));
        Serial.println("loaded alarms");
    } else {
        Serial.println("first boot");
        for (int i = 0; i < 3; i++) {
            alarmSlots[i].isEnabled = false;
            alarmSlots[i].isDaily = false;
            alarmSlots[i].alarmTime.tm_hour = 0;
            alarmSlots[i].alarmTime.tm_min = 0;
            alarmSlots[i].alarmTime.tm_sec = 0;
            alarmSlots[i].alarmTime.tm_year = 0;
            alarmSlots[i].alarmTime.tm_mon = 0;
            alarmSlots[i].alarmTime.tm_mday = 0;
            alarmSlots[i].chosenAlarm= 0;
            alarmSlots[i].snoozeDelay = 0; 
            alarmSlots[i].snoozesBeforeSilence = 3;
            alarmSlots[i].timeUntilSilence = 5;
        }
    }
    prefs.end();
}
