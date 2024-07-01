#include "seven_seg.hpp"
#include "common/mock_arduino.hpp"

#include "pinout.hpp"
#include "sensors.hpp"

namespace seven_seg {

// Mapping from digits [list indices] to which segment LEDs need to be lit.
const uint8_t digitMap[] = {
  // GFEDCBA            7-segment map:
  0b00111111, // 0          AAA
  0b00000110, // 1         F   B
  0b01011011, // 2         F   B
  0b01001111, // 3          GGG
  0b01100110, // 4         E   C
  0b01101101, // 5         E   C
  0b01111101, // 6          DDD  DP
  0b00000111, // 7
  0b01111111, // 8
  0b01101111, // 9
  0b01110111, // A
  0b01111100, // b
  0b00111001, // C
  0b01011110, // d
  0b01111001, // E
  0b01110001  // F
};

// Mapping from LED segments (interpreted as A=0, B=1, ...) to output pins.
const uint8_t pinoutMap[] = {
  pinout::SEVENSEG_A,
  pinout::SEVENSEG_B,
  pinout::SEVENSEG_C,
  pinout::SEVENSEG_D,
  pinout::SEVENSEG_E,
  pinout::SEVENSEG_F,
  pinout::SEVENSEG_G
};

uint8_t digit_values[2] = {0, 0};
uint8_t current_digit = 0;

void set_digit(uint8_t digit, uint8_t value) {
  digitalWrite(pinout::SEVENSEG_D1, false); // Turn both digits off to avoid cross-talk during the switch
  digitalWrite(pinout::SEVENSEG_D2, false);
  for (uint8_t i = 0; i < 7; i++) {
    digitalWrite(pinoutMap[i], !(digitMap[value] & (1 << i))); // segments active low
  }
  digitalWrite(pinout::SEVENSEG_DP, true);
  digitalWrite(pinout::SEVENSEG_D1, digit == 0); // select which digit to write to
  digitalWrite(pinout::SEVENSEG_D2, digit == 1);
}

void setup() {
  pinMode(pinout::SEVENSEG_D1, OUTPUT);
  pinMode(pinout::SEVENSEG_D2, OUTPUT);
  for (uint8_t i = 0; i < 7; i++) {
    pinMode(pinoutMap[i], OUTPUT);
  }
  pinMode(pinout::SEVENSEG_DP, OUTPUT);
}

void display(const ActuatorMessage &state) {
  digit_values[0] = static_cast<uint8_t>(state.ov101) << 0 |
                    static_cast<uint8_t>(state.ov102) << 1 |
                    static_cast<uint8_t>(state.nv201) << 2 |
                    static_cast<uint8_t>(state.nv202) << 3;
  digit_values[1] = static_cast<uint8_t>(state.fill_disconnect) << 0 |
                    static_cast<uint8_t>(state.tank_heating_1) << 1 |
                    static_cast<uint8_t>(state.ignition_primary) << 2 |
                    static_cast<uint8_t>(state.ignition_secondary) << 3;
}

void tick() {
  current_digit = 1 - current_digit;
  set_digit(current_digit, digit_values[current_digit]);
}

} // namespace seven_seg
