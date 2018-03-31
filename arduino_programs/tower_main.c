#include "tower_pin_defines.h"
#include "tower_fsm.h"
#include "tower_globals.h"
#include "radio_comms.h"
#include "shared_types.h"
#include "SevSeg.h"
#include "Arduino.h"

void setup() {
    //initialize all outputs
    init_outputs();
    radio_init();
}

unsigned long time_last_contact = 0;
//goto safe mode after 5 seconds of radio silence
const unsigned long global_min_time_between_contacts = 5000;

void loop() {
    //check for inputs from radio
    while(xbee_bytes_available()){
        //update FSM, which will deal with command processing
        push_radio_char(xbee_get_byte());
    }
    //get all the daq updates
    //this here's a TODO
    
    //check time last contact
    if (millis_offset() - time_last_contact > global_min_time_between_contacts) {
        //goto safe mode
        //this here's another TODO
    }

    //if the requested state and the current state aren't the same,
    //then request an acknowledgement for 
    if(!actuator_compare(get_requested_state(), get_current_state()))
        tower_request_ack(get_requested_state());

    //put the current state on the the seven segment display
    char to_put_on_sevenseg;
    if( convert_state_to_radio(get_current_state(), &to_put_on_sevenseg) ) {
        setNewNum_SevSeg( (uint8_t) to_put_on_sevenseg );
    }
    refresh_SevSeg();

    delay(100);
}