#ifndef CLIENT_GLOBALS_H
#define CLIENT_GLOBALS_H

#include "shared_types.h"
#ifdef __cplusplus
extern "C" {
#endif

//a button must read as flipped this many times
//in a row before it is seen as "on"
#define DEBOUNCE_WIDTH 3

#define millis_offset() (millis() + 5000)

extern unsigned int client_battery;

//sets the pin mode for the arming key pins
void key_switch_init();

//sets the pin mode for all the button pins to input
void init_buttons();

//returns the global button state
actuator_state_t* get_button_state();

//returns the global last received tower state
actuator_state_t* get_tower_state();

//returns the global last received tower daq state
daq_holder_t* get_tower_daq();

//reads all the buttons, based on pin numbers defined
//in client_pin_defines.h
void read_all_buttons();

//state = 0, turn red led on
//else, turn green led on
void set_radio_status(int state);

//sets the state of the LEDs on the switches
//true for on, false for off
void set_switch_LEDs(bool state);

#ifdef __cplusplus
}
#endif

#endif //ifndef CLIENT_GLOBALS_H
