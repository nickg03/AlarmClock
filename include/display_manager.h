#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>

#define I2C_SDA 8
#define I2C_SCL 9

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64

enum UIState { // Enum that describes every possible state in which the alarm clock can be
    STATE_DEFAULT,
    STATE_SCROLL_MENU,
    STATE_EDIT_YEAR,
    STATE_EDIT_MONTH,
    STATE_EDIT_DAY, 
    STATE_EDIT_HOUR,
    STATE_EDIT_MINUTES,
    STATE_EDIT_SECONDS,
    STATE_EDIT_AMPM,
    STATE_EDIT_SNOOZEDELAY, // Time between snoozes.
    STATE_EDIT_SNOOZESBEFORESILENCE, // Amount of times alarm can be snoozed before it silences.
    STATE_EDIT_TIMEUNTILSILENCE, // How long an alarm will sound before it silences.
    STATE_EDIT_ALARMDATEBOOL,
    STATE_EDIT_SELECTALARM,
};

extern int currentMenuScrollPosition;
extern bool displayIsFlickering;

extern UIState currentUIState;
extern bool updateDisplayFlag;

bool displayFound();
void displayString(String message, int textSize, int cursorX, int cursorY, bool clearDisplay); 
void displayClear();
void displayDim(bool dim, bool isManual);
void flickerDisplay();
void updateDisplay();
void drawAlarmMenu();

extern tm displayTime;

//add function to display a selectable list with arguments that include # of items and the list of items 

#endif // DISPLAY_MANAGER_H