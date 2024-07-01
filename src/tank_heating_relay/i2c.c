#include "i2c.h"
#include "relay_general.h"

uint16_t i2cSlaveRecv; // Data received on i2c
uint16_t i2cSlaveSend; // Data to be sent on i2c

void i2c_slave_init(uint16_t address) {
    SSPSTAT = 0x80;
    SSPADDbits.SSPADD = address << 1; // 7 bit addressing, LSB is unused
    SSPCON = 0x36;
    SSPCON2 = 0x01;
    TRISB1 = 1; // SDA
    TRISB4 = 1; // SCL
    GIE = 1;
    PEIE = 1;
    SSP1IF = 0;
    SSP1IE = 1;
}

uint8_t i2c_reg_select = 0;

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
        SSPCONbits.WCOL = 0; // Clear the collision bit
        SSPCONbits.CKP = 1;
        return;
    }

    // If last byte was Address + write
    if (!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) {
        while (!BF && timeout < TIMEOUT) {
            timeout++;
        }
        temp = SSP1BUF;
        BF = 0;
        SSPCONbits.CKP = 1;
        while (!BF && timeout < TIMEOUT) {
            timeout++;
        }
        i2cSlaveRecv = SSP1BUF;
        if (timeout < TIMEOUT) {
            // LSB is power
            if (i2cSlaveRecv & 1) {
                set_power_on();
            } else {
                set_power_off();
            }
			// Bit 1-3 are register select
			i2c_reg_select = (i2cSlaveRecv >> 1) & 7;
        }
        SSPCONbits.CKP = 1;
        BF = 0;
        // We already set this above but for some reason we need to set it here too
        SSP1IF = 0;
    }
    // If this is a read
    else if (SSPSTATbits.R_nW) {
        static uint8_t read_pointer;
        // If this is the first byte in a read
        if (!SSPSTATbits.D_nA) {
            read_pointer = 0;
        }
        temp = SSPBUF;

		if(read_pointer == 0) {
			switch(i2c_reg_select) {
			case 0:
				SSPBUF = (uint8_t)(get_analog_inputs(CHANNEL_THERMISTOR) & 0xFF);
				break;
			case 1:
				SSPBUF = (uint8_t)(get_analog_inputs(CHANNEL_CURR_SENSE) & 0xFF);
				break;
			case 2:
				SSPBUF = (uint8_t)(get_analog_inputs(CHANNEL_24V_SENSE) & 0xFF);
				break;
			case 3:
				SSPBUF = (uint8_t)(get_analog_inputs(CHANNEL_KELVIN_N) & 0xFF);
				break;
			case 4:
				SSPBUF = (uint8_t)(get_analog_inputs(CHANNEL_KELVIN_P) & 0xFF);
				break;
                default:
			    SSPBUF = 0;
			}
		} else if (read_pointer == 1) {
			switch(i2c_reg_select) {
			case 0:
				SSPBUF = (uint8_t)(get_analog_inputs(CHANNEL_THERMISTOR) >> 8);
				break;
			case 1:
				SSPBUF = (uint8_t)(get_analog_inputs(CHANNEL_CURR_SENSE) >> 8);
				break;
			case 2:
				SSPBUF = (uint8_t)(get_analog_inputs(CHANNEL_24V_SENSE) >> 8);
				break;
			case 3:
				SSPBUF = (uint8_t)(get_analog_inputs(CHANNEL_KELVIN_N) >> 8);
				break;
			case 4:
				SSPBUF = (uint8_t)(get_analog_inputs(CHANNEL_KELVIN_P) >> 8);
				break;
            default:
			    SSPBUF = 0;
			}
		}

        read_pointer++;
        SSPCONbits.CKP = 1;
    }
    SSPCONbits.CKP = 1;
}
