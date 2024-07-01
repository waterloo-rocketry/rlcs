#include <xc.h>

#include "relay_general.h"

static uint16_t curr_sense = 0;
static uint16_t thermistor = 0;
static uint16_t _24v_sense = 0;
static uint16_t kelvin_n = 0;
static uint16_t kelvin_p = 0;

uint16_t read_analog_inputs(uint8_t port) {
    ADCON0 = 0x01 | (port << 2); // Turn ADC on, select port to read from
    ADCON0 |= 1 << 1; // set b[1] "go" bit
    uint8_t done_bit;
    do { // wait for ADC to complete (go bit switches to 0 automatically when done)
        done_bit = ADCON0 & (1 << 1);
    } while (done_bit); // while go bit is on (AD conversion in progress)

    uint16_t adc_result =
        ((uint16_t)ADRESH << 8) | ADRESL; // combine two 8bit values into a 16bit value
    if (port == CHANNEL_THERMISTOR) {
        thermistor = adc_result;
    } else if (port == CHANNEL_CURR_SENSE) {
        curr_sense = adc_result;
    } else if (port == CHANNEL_24V_SENSE) {
        _24v_sense = adc_result;
    } else if (port == CHANNEL_KELVIN_N) {
        kelvin_n = adc_result;
    } else if (port == CHANNEL_KELVIN_P) {
        kelvin_p = adc_result;
    }

    ADCON0 = 0x00; // Turn ADC off return;

    return adc_result;
}

uint16_t get_analog_inputs(uint8_t port) {
    if (port == CHANNEL_THERMISTOR) {
        return thermistor;
    } else if (port == CHANNEL_CURR_SENSE) {
        return curr_sense;
    } else if (port == CHANNEL_24V_SENSE) {
        return _24v_sense;
    } else if (port == CHANNEL_KELVIN_N) {
        return kelvin_n;
    } else if (port == CHANNEL_KELVIN_P) {
        return kelvin_p;
    }
    return 0;
}

void set_power_on(void) {
    set_power(true);
}

void set_power_off(void) {
    set_power(false);
}

void set_led_on(void) {
    set_led(true);
}

void set_led_off(void) {
    set_led(false);
}

void set_power(bool out) {
    if (out) {
        LATAbits.LATA2 = 1;
    } else {
        LATAbits.LATA2 = 0;
    }
}

void set_led(bool out) {
    if (out) {
        LATBbits.LATB5 = 1;
    } else {
        LATBbits.LATB5 = 0;
    }
}

void led_heartbeat(void) {
    static bool led_on = false;
    if (led_on) {
        set_led_off();
        led_on = false;
    } else {
        set_led_on();
        led_on = true;
    }
}
