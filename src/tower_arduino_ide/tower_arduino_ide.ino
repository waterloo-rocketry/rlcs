/* switch keylock CKM12BFW01-001
 *  14.2mmm new keyswitch diameter
 *  old keyswitch 18.2mm diameter
 *  
 */


#include "tower_pin_defines.h"
#include "tower_fsm.h"
#include "tower_globals.h"
#include "tower_daq.h"
#include "radio_comms.h"
#include "nodeio.ioio.h"
#include "shared_types.h"
#include "SevSeg.h"
#include "Arduino.h"
#include "sd_handler.h"
#include "linac.h"
#include "i2c.h"

void setup() {
    //initialize all outputs
    key_switch_init();
    i2c_init();
    init_outputs();
    init_daq_pins();
    radio_init();
    nio_init();
    start_SevSeg();
    sd_init();
    if(sd_active()){
        rlcslog("I'm the tower");
    }


}

//updated by the FSM whenever the client requests state or daq
unsigned long time_last_contact = 0;
//goto safe mode after 10 seconds of radio silence
const unsigned long global_min_time_between_contacts = 10000;

//used for the SD card handler
extern unsigned long global_time_last_output_flush;
//flush the rlcslog to sd card every 10 seconds
extern const unsigned long global_output_flush_interval;

extern unsigned long global_time_last_logged_daq;
extern const unsigned long global_time_between_daq_logs;

uint8_t to_put_on_sevenseg = 0b00000000;

void loop() {
    //check for inputs from radio
    while(xbee_bytes_available()){
        //update FSM, which will deal with command processing
        push_radio_char(xbee_get_byte());
    }
    //get all the daq updates
    read_daq_pins();
    //if we haven't calculated (and thus logged) the daq values in some
    //amount of time, do that now
    if (millis() - global_time_last_logged_daq > global_time_between_daq_logs) {
        get_global_current_daq();
    }

    //check time last contact
    if (millis_offset() - time_last_contact > global_min_time_between_contacts) {
        //goto safe mode
        goto_safe_mode();
    } else {
        //we've heard from the client recently, so the rocket should be awake
        nio_power_bus();
    }

    //deal with the nodeio.ioio stuff
    nio_refresh();

    //deal with linear actuator

    linac_refresh();

    //TODO: If the state us unknown change the display to 'UU'
    //put the current state on the the seven segment display. see convert_state_to_segment for formatting documentation
    to_put_on_sevenseg = convert_state_to_segment(*get_current_state());
    setNewNum_SevSeg(to_put_on_sevenseg);
    refresh_SevSeg();

    //check how long it's been since we flushed the log
    if(millis() - global_time_last_output_flush > global_output_flush_interval){
        flush();
    }
}
