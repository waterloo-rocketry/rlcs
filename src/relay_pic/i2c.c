#include "i2c.h"
#include "relay_general.h"

uint16_t i2cSlaveRecv; // Data received on i2c
uint16_t i2cSlaveSend; // Data to be sent on i2c

void i2c_slave_init(uint16_t address) {
    SSPSTAT = 0x80;
    SSPADDbits.SSPADD = address << 1; //7 bit addressing, LSB is unused
    SSPCON = 0x36;
    SSPCON2 = 0x01;
    TRISB1 = 1; // SDA
    TRISB4 = 1; // SCL
    GIE = 1;
    PEIE = 1;
    SSP1IF = 0;
    SSP1IE = 1;
}

#define TIMEOUT 200
void i2c_handle_interrupt(void) {
    uint16_t temp;
    uint16_t timeout = 0;
    SSPCONbits.CKP = 0;

    SSP1IF = 0;

    // If overflow or collision.
    if ((SSPCONbits.SSPOV) || (SSPCONbits.WCOL)) {
        temp = SSPBUF; // Read the previous value to clear the buffer
        SSPCONbits.SSPOV = 0; // Clear the overflow flag
        SSPCONbits.WCOL = 0;   // Clear the collision bit
        SSPCONbits.CKP = 1;
        return;
    }

    // If last byte was Address + write
    if (!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) {
        while(!BF && timeout < TIMEOUT) timeout++;
        temp = SSP1BUF;
        BF = 0;
        SSPCONbits.CKP = 1;
        while(!BF && timeout < TIMEOUT) timeout++;
        i2cSlaveRecv = SSP1BUF;
        if (timeout < TIMEOUT) {
            // LSB is power, second bit is select
            if (i2cSlaveRecv & (1 << 1)) {
                set_select_on();
            } else {
                set_select_off();
            }
            if (i2cSlaveRecv & 1) {
                set_power_on();
            } else {
                set_power_off();
            }
        }
        SSPCONbits.CKP = 1;
        BF = 0;
        // We already set this above but for some reason we need to set it here too
        SSP1IF = 0;
    }
    // If this is a read
    else if (SSPSTATbits.R_nW) {
        for (uint8_t read_pointer = 0; read_pointer < 4; read_pointer++) {
            temp = SSPBUF;
            BF = 0;
            if (read_pointer == 0) {
            SSPBUF = (get_lim2() << 1) | get_lim1();
            } else if (read_pointer == 1) {
                SSPBUF = (uint8_t)(get_analog_inputs(CURR_SENSE_1));
            } else if (read_pointer == 2) {
                SSPBUF = (uint8_t)(get_analog_inputs(CURR_SENSE_1) >> 8);
            } else if (read_pointer == 3) {
                SSPBUF = (uint8_t)(get_analog_inputs(CURR_SENSE_2));
            } else if (read_pointer == 4) {
                SSPBUF = (uint8_t)(get_analog_inputs(CURR_SENSE_2) >> 8);
            }
            SSPCONbits.CKP = 1;
            while(SSPSTATbits.BF && timeout < TIMEOUT) timeout++;
            if (SSP1CON2bits.ACKSTAT) {
                break;
            }
        }
    }
    SSPCONbits.CKP = 1;
}
