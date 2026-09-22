#ifndef DATA_HANDLING_H
#define DATA_HANDLING_H

struct Alarm {
    tm alarmTime{};
    bool isEnabled{false};
    bool isDaily{false};
    char chosenAlarm{0};
    int snoozeDelay;
    int snoozesBeforeSilence;
    int timeUntilSilence;
};

extern bool standardFormat;
extern bool dstEnabled;
extern Alarm alarmSlots[3];
extern Alarm editAlarmBuffer;

void loadSystemSettings();
void saveSystemSettings();

#endif