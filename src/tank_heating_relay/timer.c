#include <xc.h>
#include "timer.h"

/*
 * Magic numbers: timer is driven on a 500kHz clock and is 8 bits.
 * Timer rolls over every 256 counts, or every 512 microseconds.
 * MILLIS_INCREMENT = 0, since there are 0 ms added each rollover
 * MILLIS_REMAINDER = 512, added each time the timer is triggered
 * MILLIS_INCREMENT_CAP = 1000, number of microseconds before we tick over a millisecond
 *
 * Only the ratio between MILLIS_REMAINDER and MILLIS_INCREMENT_CAP matters, so
 * we can optimize things a little bit by dividing both by 8 so that the largest
 * value of internal_count can fit within 8 bits.
 *
 */
#define MILLIS_INCREMENT 0
#define MILLIS_REMAINDER 64
#define MILLIS_INCREMENT_CAP 125

 static uint32_t millis_counter = 0;

 uint32_t millis(void) {
    return millis_counter;
}

void timer0_init(void) {
    INTCONbits.TMR0IE = 1; //Enable timer interrupt
    OPTION_REGbits.TMR0CS = 0; //Use internal instruction clock cycle (4 mHz / 4))
    OPTION_REGbits.PS = 0b000; // Set 1:2 prescaler for 500 kHz
    OPTION_REGbits.PSA = 0; //Utilize prescalar assignment bit
}

/*
 * Based on Bresenham's algorithm and described here: http://romanblack.com/one_sec.htm
 */
void timer0_handle_interrupt(void) {
    static uint8_t internal_count = 0;

    millis_counter += MILLIS_INCREMENT;
    internal_count += MILLIS_REMAINDER;
    if (internal_count > MILLIS_INCREMENT_CAP) {
        internal_count -= MILLIS_INCREMENT_CAP;
        millis_counter++;
    }
}
