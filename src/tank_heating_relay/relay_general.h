#ifndef RELAY_GENERAL_H
#define RELAY_GENERAL_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define CHANNEL_THERMISTOR 3
#define CHANNEL_CURR_SENSE 1
#define CHANNEL_24V_SENSE 0
#define CHANNEL_KELVIN_N 9
#define CHANNEL_KELVIN_P 10

uint16_t read_analog_inputs(uint8_t port);

uint16_t get_analog_inputs(uint8_t port);

void set_power_on(void);

void set_power_off(void);

void set_led_on(void);

void set_led_off(void);

void led_heartbeat(void);

//internal functions
void set_power(bool out);
void set_led(bool out);

#endif /* RELAY_GENERAL_H */
