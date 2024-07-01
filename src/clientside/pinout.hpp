#ifndef PINOUT_HPP
#define PINOUT_HPP

#include <stdint.h>

namespace pinout {

// Analog input for clientside's (divided) battery voltage
const uint8_t BATT_VOLTAGE = 1;

const uint8_t KEY_SWITCH_IN = 7;

const uint8_t MISSILE_SWITCH_1 = 20;
const uint8_t MISSILE_SWITCH_2 = 19;
const uint8_t MISSILE_SWITCH_3 = 18;
const uint8_t MISSILE_SWITCH_4 = 17;
const uint8_t MISSILE_SWITCH_5 = 16;
const uint8_t MISSILE_SWITCH_6 = 28;
const uint8_t MISSILE_SWITCH_7 = 30;
const uint8_t MISSILE_SWITCH_8 = 2;
const uint8_t MISSILE_SWITCH_INJECTOR = 5;
const uint8_t MISSILE_SWITCH_IGNITION_PRI = 3;
const uint8_t MISSILE_SWITCH_IGNITION_SEC = 4;
const uint8_t MISSILE_SWITCH_IGNITION_FIRE = 6;

const uint8_t LCD_RS = 64;
const uint8_t LCD_EN = 63;
const uint8_t LCD_D4 = 62;
const uint8_t LCD_D5 = 61;
const uint8_t LCD_D6 = 60;
const uint8_t LCD_D7 = 59;

const uint8_t LED_RED = 58;
const uint8_t LED_GREEN = 57;
const uint8_t LED_BLUE = 56;

// LEDs inside the missile switches to show the armed state
const uint8_t MISSILE_LED = 38;

} // namespace pinout

#endif
