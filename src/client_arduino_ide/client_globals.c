#include "client_globals.h"
#include "client_pin_defines.h"
#include "shared_types.h"
#include "Arduino.h"


static actuator_state_t global_button_state, global_tower_state;
static daq_holder_t global_current_daq = {
    .pressure1 = 999,
    .pressure2 = 999,
    .pressure3 = 999,
    .rocket_mass = 0xFFFF,
    .ign_pri_current = 0xFFFF,
    .ign_sec_current = 0xFFFF,
    .valve_1_lsw_open = 0,
    .valve_1_lsw_closed = 0,
    .valve_2_lsw_open = 0,
    .valve_2_lsw_closed = 0,

    .rocketvent_valve_state = DAQ_VALVE_UNK,
    .injector_valve_state = DAQ_VALVE_UNK,

    .valve_3_lsw_open = 0,
    .valve_3_lsw_closed = 0
};

typedef uint8_t button_t;
static struct {
    button_t valve_1;
    button_t valve_2;
    button_t valve_3;
    button_t valve_4;
    button_t injector;
    button_t ignition_pri;
    button_t ignition_sec;
    button_t ignition_fire;
} button_debounce[DEBOUNCE_WIDTH];
static uint8_t button_debounce_index = 0;

//boolean to track whether we need to recalculate global button state before
//handing it back to the caller
static uint8_t global_button_state_tainted = 1;
actuator_state_t *get_button_state()
{
    if (global_button_state_tainted) {
        //recalculate button state. We do calculations here for
        //software debouncing purposes
        button_t valve_1 = 1;
        button_t valve_2 = 1;
        button_t valve_3 = 1;
        button_t valve_4 = 1;
        button_t injector = 1;
        button_t ignition_pri = 1;
        button_t ignition_sec = 1;
        button_t ignition_fire = 1;
        for (int i = 0; i < DEBOUNCE_WIDTH; i++) {
            //each button is only considered active (high)
            //if all readings in the past DEBOUNCE width
            //readings were active
            valve_1 &= button_debounce[i].valve_1;
            valve_2 &= button_debounce[i].valve_2;
            valve_3 &= button_debounce[i].valve_3;
            valve_4 &= button_debounce[i].valve_4;
            injector &= button_debounce[i].injector;
            ignition_pri &= button_debounce[i].ignition_pri;
            ignition_sec &= button_debounce[i].ignition_sec;
            ignition_fire &= button_debounce[i].ignition_fire;
        }

        //now decode the debounced button states into an actuator_state_t
        global_button_state.valve_1 =
            valve_1 ? 1 : 0;
        global_button_state.valve_2 =
            valve_2 ? 1 : 0;
        global_button_state.valve_3 =
            valve_3 ? 1 : 0;
        global_button_state.injector_valve =
            injector ? 1 : 0;
        global_button_state.valve_4 =
            valve_4 ? 1 : 0;
        
        //by default, ignition relays are both off
        global_button_state.ignition_power = global_button_state.ignition_select = 0;
        if (ignition_fire) {
            if (ignition_pri && !ignition_sec) {
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

actuator_state_t *get_tower_state()
{
    return &global_tower_state;
}

daq_holder_t *get_tower_daq()
{
    return &global_current_daq;
}


void read_all_buttons()
{
    button_debounce[button_debounce_index].valve_1 =
        digitalRead(PIN_SWITCH_VALVE_1) == HIGH;
    button_debounce[button_debounce_index].valve_2 =
        digitalRead(PIN_SWITCH_VALVE_2) == HIGH;
    button_debounce[button_debounce_index].valve_3 =
        digitalRead(PIN_SWITCH_VALVE_3) == HIGH;
    button_debounce[button_debounce_index].valve_4 =
        digitalRead(PIN_SWITCH_VALVE_4) == HIGH;
    button_debounce[button_debounce_index].injector =
        digitalRead(PIN_SWITCH_INJECTOR) == HIGH;
    button_debounce[button_debounce_index].ignition_pri =
        digitalRead(PIN_SWITCH_IGNITION_PRI) == HIGH;
    button_debounce[button_debounce_index].ignition_sec =
        digitalRead(PIN_SWITCH_IGNITION_SEC) == HIGH;
    //fire is active low
    button_debounce[button_debounce_index].ignition_fire =
        digitalRead(PIN_SWITCH_IGNITION_FIRE) == LOW;

    if ( (++button_debounce_index) >= DEBOUNCE_WIDTH )
        button_debounce_index = 0;

    //mark boolean to
    global_button_state_tainted = 1;
}

void init_buttons()
{
    pinMode(PIN_SWITCH_VALVE_1, INPUT);
    pinMode(PIN_SWITCH_VALVE_2, INPUT);
    pinMode(PIN_SWITCH_VALVE_3, INPUT);
    pinMode(PIN_SWITCH_VALVE_4, INPUT);
    pinMode(PIN_SWITCH_INJECTOR, INPUT);
    pinMode(PIN_SWITCH_IGNITION_PRI, INPUT);
    pinMode(PIN_SWITCH_IGNITION_SEC, INPUT);
    pinMode(PIN_SWITCH_IGNITION_FIRE, INPUT_PULLUP);

    //zero out all the buttons
    memset(&button_debounce, 0, sizeof(button_debounce));

    //battery sensing
    pinMode(PIN_BATTERY, INPUT);

    //LEDs. Turn on the red one first
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_BLUE, OUTPUT);
    digitalWrite(PIN_LED_RED, HIGH);
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_BLUE, LOW);

    //switch LEDS
    pinMode(PIN_LED_SWITCH_1, OUTPUT);
    digitalWrite(PIN_LED_SWITCH_1, HIGH);
    pinMode(PIN_LED_SWITCH_2, OUTPUT);
    digitalWrite(PIN_LED_SWITCH_2, HIGH);
    pinMode(PIN_LED_SWITCH_3, OUTPUT);
    digitalWrite(PIN_LED_SWITCH_3, HIGH);
    pinMode(PIN_LED_SWITCH_4, OUTPUT);
    digitalWrite(PIN_LED_SWITCH_4, HIGH);
    pinMode(PIN_LED_SWITCH_5, OUTPUT);
    digitalWrite(PIN_LED_SWITCH_5, HIGH);
    pinMode(PIN_LED_SWITCH_6, OUTPUT);
    digitalWrite(PIN_LED_SWITCH_6, HIGH);
    pinMode(PIN_LED_SWITCH_7, OUTPUT);
    digitalWrite(PIN_LED_SWITCH_7, HIGH);
    pinMode(PIN_LED_SWITCH_8, OUTPUT);
    digitalWrite(PIN_LED_SWITCH_8, HIGH);
    pinMode(PIN_LED_SWITCH_IGNITION_PRI, OUTPUT);
    digitalWrite(PIN_LED_SWITCH_IGNITION_PRI, HIGH);
    pinMode(PIN_LED_SWITCH_IGNITION_SEC, OUTPUT);
    digitalWrite(PIN_LED_SWITCH_IGNITION_SEC, HIGH);
    pinMode(PIN_LED_SWITCH_IGNITION_FIRE, OUTPUT);
    digitalWrite(PIN_LED_SWITCH_IGNITION_FIRE, HIGH);
    
}

void key_switch_init(){
  // tie the pin to GND
  pinMode(PIN_KEY_SWITCH_GND, OUTPUT);
  digitalWrite(PIN_KEY_SWITCH_GND, LOW);
  //set as input with pullup
  pinMode(PIN_KEY_SWITCH_INPUT, INPUT_PULLUP);
}

static int led_state = 0;
void set_radio_status(int state)
{

    if (state == led_state)
        return;
    //If state is true, then turn on green led
    if ((led_state = state)) {
        digitalWrite(PIN_LED_GREEN, HIGH);
        digitalWrite(PIN_LED_RED, LOW);
    } else {
        digitalWrite(PIN_LED_GREEN, LOW);
        digitalWrite(PIN_LED_RED, HIGH);
    }
    //update led_state
    led_state = state;
}

void set_switch_LEDs(bool state){
  digitalWrite(PIN_LED_SWITCH_1, !state);
  digitalWrite(PIN_LED_SWITCH_2, !state);
  digitalWrite(PIN_LED_SWITCH_3, !state);
  digitalWrite(PIN_LED_SWITCH_4, !state);
  digitalWrite(PIN_LED_SWITCH_5, !state);
  digitalWrite(PIN_LED_SWITCH_6, !state);
  digitalWrite(PIN_LED_SWITCH_7, !state);
  digitalWrite(PIN_LED_SWITCH_8, !state);
  digitalWrite(PIN_LED_SWITCH_IGNITION_PRI, !state);
  digitalWrite(PIN_LED_SWITCH_IGNITION_SEC, !state);
  digitalWrite(PIN_LED_SWITCH_IGNITION_FIRE, !state);
  
}

//globals for how long it's been since we've made requests to the tower
unsigned long global_time_last_tower_state_req = 0;
const unsigned long global_tower_update_interval = 1000; //request every second
const unsigned long global_radio_timeout = 5000; //we've lost radio contact
unsigned long global_time_last_tower_daq_req = 0;
const unsigned long global_tower_daq_update_interval = 1000; //request daq every 1 seconds
//how often to flush the buffered output to the SD card
unsigned long global_time_last_output_flush = 0;
const unsigned long global_output_flush_interval = 30000; //every 30 seconds

unsigned long global_time_last_logged_daq = 0;
