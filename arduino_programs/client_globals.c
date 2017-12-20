#include "client_globals.h"
#include "client_pin_defines.h"
#include "shared_types.h"
#include "Arduino.h"

static actuator_state_t global_button_state, global_tower_state;
static daq_holder_t global_current_daq;

typedef uint8_t button_t;
static struct {
    button_t remotefill;
    button_t remotevent;
    button_t rocketvalve;
    button_t linactuator;
    button_t ignition_pri;
    button_t ignition_sec;
    button_t ignition_fire;
} button_debounce[DEBOUNCE_WIDTH];
static uint8_t button_debounce_index = 0;

//boolean to track whether we need to recalculate global button state before
//handing it back to the caller
static uint8_t global_button_state_tainted = 1;
actuator_state_t* get_button_state(){
    if(global_button_state_tainted) {
        //recalculate button state. We do calculations here for
        //software debouncing purposes
        button_t remotefill = 1;
        button_t remotevent = 1;
        button_t rocketvalve = 1;
        button_t linactuator = 1;
        button_t ignition_pri = 1;
        button_t ignition_sec = 1;
        button_t ignition_fire = 1;
        for(int i = 0; i < DEBOUNCE_WIDTH; i++) {
            //each button is only considered active (high)
            //if all readings in the past DEBOUNCE width
            //readings were active
            remotefill &= button_debounce[i].remotefill;
            remotevent &= button_debounce[i].remotevent;
            rocketvalve &= button_debounce[i].rocketvalve;
            linactuator &= button_debounce[i].linactuator;
            ignition_pri &= button_debounce[i].ignition_pri;
            ignition_sec &= button_debounce[i].ignition_sec;
            ignition_fire &= button_debounce[i].ignition_fire;
        }

        //now decode the debounced button states into an actuator_state_t
        global_button_state.remote_fill_valve =
            remotefill ? 1 : 0;
        global_button_state.remote_vent_valve =
            remotevent ? 1 : 0;
        global_button_state.run_tank_valve =
            rocketvalve ? 1 : 0;
        global_button_state.linear_actuator =
            linactuator ? 1 : 0;

        //by default, ignition relays are both off
        global_button_state.ignition_power = global_button_state.ignition_select = 0;
        if(ignition_fire) {
            if(ignition_pri && !ignition_sec) {
                global_button_state.ignition_power = 1;
            } else if ( !ignition_pri && ignition_sec) {
                global_button_state.ignition_power = 1;
                global_button_state.ignition_select = 1;
            } else {
                //either both missile switches are thrown, or neither
                //either way, don't turn on either of the relays
            }
        }

        global_button_state_tainted = 0;
    }
    return &global_button_state;
}

actuator_state_t* get_tower_state(){
    return &global_tower_state;
}

daq_holder_t* get_tower_daq(){
    return &global_current_daq;
}


void read_all_buttons(){
    button_debounce[button_debounce_index].remotefill =
        digitalRead(PIN_SWITCH_REMOTEFILL) == HIGH;
    button_debounce[button_debounce_index].remotevent =
        digitalRead(PIN_SWITCH_REMOTEVENT) == HIGH;
    button_debounce[button_debounce_index].rocketvalve =
        digitalRead(PIN_SWITCH_ROCKETVALVE) == HIGH;
    button_debounce[button_debounce_index].linactuator =
        digitalRead(PIN_SWITCH_LINACTUATOR) == HIGH;
    button_debounce[button_debounce_index].ignition_pri =
        digitalRead(PIN_SWITCH_IGNITION_PRI) == HIGH;
    button_debounce[button_debounce_index].ignition_sec =
        digitalRead(PIN_SWITCH_IGNITION_SEC) == HIGH;
    button_debounce[button_debounce_index].ignition_fire =
        digitalRead(PIN_SWITCH_IGNITION_FIRE) == HIGH;

    if( (++button_debounce_index) >= DEBOUNCE_WIDTH )
        button_debounce_index = 0;

    //mark boolean to
    global_button_state_tainted = 1;
}

void init_buttons(){
    pinMode(PIN_SWITCH_REMOTEFILL, INPUT);
    pinMode(PIN_SWITCH_REMOTEVENT, INPUT);
    pinMode(PIN_SWITCH_ROCKETVALVE, INPUT);
    pinMode(PIN_SWITCH_LINACTUATOR, INPUT);
    pinMode(PIN_SWITCH_IGNITION_PRI, INPUT);
    pinMode(PIN_SWITCH_IGNITION_SEC, INPUT);
    pinMode(PIN_SWITCH_IGNITION_FIRE, INPUT);

    //zero out all the buttonsk
    memcpy(0,&button_debounce, sizeof(button_debounce));
}

//globals for how long it's been since we've made requests to the tower
unsigned long global_time_last_tower_state_req = 0;
const unsigned long global_tower_update_interval = 3000; //request every 3 seconds
unsigned long global_time_last_tower_daq_req = 0;
unsigned long global_tower_daq_update_interval = 3000; //request daq every 3 seconds

