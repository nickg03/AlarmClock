#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Arduino.h>

enum InputIndex {
    BTN_RST = 0,
    BTN_FORMAT,
    BTN_DST,
    BTN_SNOOZE_SILENCE,
    BTN_SET_ALARM,
    ENCODER_SWITCH,
    ENCODER_CLK,
    ENCODER_DT,
    NUM_INPUTS  // Equal to ...
};

 const int buzzer = 4;


// Structure that maps an input to a pin number
struct InputMap {
    InputIndex type;
    uint8_t pin;
};

// An array of buttons mapped to pins
inline constexpr InputMap INPUTS[NUM_INPUTS] = {
    { BTN_RST,            13 },
    { BTN_FORMAT,         15 },
    { BTN_DST,             7 },
    { BTN_SNOOZE_SILENCE,  5 },
    { BTN_SET_ALARM,       6 },
    { ENCODER_SWITCH,     16 },
    { ENCODER_CLK,        18 },
    { ENCODER_DT,         17 }
};

void initInputManager();
bool inputPressed(InputIndex btn);
void encoderDirection();
void encoderSwitch();
void automaticDimming();


#endif // INPUT_MANAGER_H