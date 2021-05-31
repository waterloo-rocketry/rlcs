#include "tower_globals.h"
#include "tower_daq.h"
#include "tower_pin_defines.h"
#include "nodeio.ioio.h"
#include "sd_handler.h"
#include "linac.h"
#include "injector.h"
#include "Arduino.h"

static actuator_state_t global_requested_state;
static actuator_state_t global_current_state = {
    .valve_1 = 0,
    .valve_2 = 0,
    .valve_3 = 0,
    .valve_4 = 0,
    .injector_valve = 0,
    .ignition_power = 0,
    .ignition_select = 0
};

static daq_holder_t global_current_daq;

actuator_state_t* get_requested_state(){
    return &global_requested_state;
}

actuator_state_t* get_current_state(){
    return &global_current_state;
}

daq_holder_t* get_global_current_daq(){
    compute_daq_values(&global_current_daq);
    return &global_current_daq;
}

//copies requested_state to current_state, then applies requested state
//to the outputs
void apply_state(){

    if(global_requested_state.valve_1 != global_current_state.valve_1){
        //we need to change the valve_1 to what requested wants
        global_current_state.valve_1 = global_requested_state.valve_1;
        if(global_current_state.valve_1){
            //open the valve_1
            digitalWrite((uint8_t) PIN_VALVE_1_POWER, HIGH);
            digitalWrite((uint8_t) PIN_VALVE_1_SELECT, LOW);
        } else {
            //close the valve_1
            digitalWrite((uint8_t) PIN_VALVE_1_POWER, HIGH);
            digitalWrite((uint8_t) PIN_VALVE_1_SELECT, HIGH);
        }
    }

    if(global_requested_state.valve_2 != global_current_state.valve_2){
        //we need to change the valve_2 to what requested wants
        global_current_state.valve_2 = global_requested_state.valve_2;
        if(global_current_state.valve_2){
            //open the valve_2
            digitalWrite((uint8_t) PIN_VALVE_2_POWER, HIGH);
            digitalWrite((uint8_t) PIN_VALVE_2_SELECT, LOW);
        } else {
            //close the valve_2
            digitalWrite((uint8_t) PIN_VALVE_2_POWER, HIGH);
            digitalWrite((uint8_t) PIN_VALVE_2_SELECT, HIGH);
        }
    }

    if(global_requested_state.valve_3 != global_current_state.valve_3){
        //we need to change the valve_2 to what requested wants
        global_current_state.valve_3 = global_requested_state.valve_3;
        if(global_current_state.valve_3){
            //open the valve_3
            digitalWrite((uint8_t) PIN_VALVE_3_POWER, HIGH);
            digitalWrite((uint8_t) PIN_VALVE_3_SELECT, LOW);
        } else {
            //close the valve_3
            digitalWrite((uint8_t) PIN_VALVE_3_POWER, HIGH);
            digitalWrite((uint8_t) PIN_VALVE_3_SELECT, HIGH);
        }
    }

    if(global_requested_state.valve_4 != global_current_state.valve_4){
        //we need to change the valve_4 to what requested wants
        global_current_state.valve_4 = global_requested_state.valve_4;
        if(global_current_state.valve_4){
            //open the valve_4
            digitalWrite((uint8_t) PIN_VALVE_4_POWER, HIGH);
            digitalWrite((uint8_t) PIN_VALVE_4_SELECT, LOW);
        } else {
            //close the valve_4
            digitalWrite((uint8_t) PIN_VALVE_4_POWER, HIGH);
            digitalWrite((uint8_t) PIN_VALVE_4_SELECT, HIGH);
        }
    }
/*
    global_current_state.valve_3 = global_requested_state.valve_3;
    //tell nodeio.ioio that we want the vent to change
    if(global_requested_state.valve_3){
        //we want it open
        nio_set_vent_desired(VALVE_OPEN);
    }
    else {
        nio_set_vent_desired(VALVE_CLOSED);
    }
*/
    global_current_state.injector_valve = global_requested_state.injector_valve;
    //tell nodeio.ioio that we want the injector to change and change the injector valves
    if(global_requested_state.injector_valve){
        //we want it open
        nio_set_inj_desired(VALVE_OPEN);
        //open it
        digitalWrite((uint8_t) PIN_INJECTOR_VALVE_POWER, HIGH);
        digitalWrite((uint8_t) PIN_INJECTOR_VALVE_SELECT, LOW);

        
    }
    else {
        nio_set_inj_desired(VALVE_CLOSED);
        // close it
        digitalWrite((uint8_t) PIN_INJECTOR_VALVE_POWER, HIGH);
        digitalWrite((uint8_t) PIN_INJECTOR_VALVE_SELECT, HIGH);
        
    }

    if(global_requested_state.ignition_power != global_current_state.ignition_power){
        //we need to change the ignition_power to what requested wants
        global_current_state.ignition_power = global_requested_state.ignition_power;
        if(global_current_state.ignition_power){
            //decide whether to write to primary or secondary ignition
            if(global_requested_state.ignition_select) {
                //write to secondary
                digitalWrite((uint8_t) PIN_IGNITION_POWER, HIGH);
                digitalWrite((uint8_t) PIN_IGNITION_SELECT, LOW);
            } else {
                //write to primary
                digitalWrite((uint8_t) PIN_IGNITION_POWER, HIGH);
                digitalWrite((uint8_t) PIN_IGNITION_SELECT, HIGH);

            }
        } else {
            //remove power from ignition circuit
            digitalWrite((uint8_t) PIN_IGNITION_POWER, LOW);
        }
    }

    if(global_requested_state.ignition_select != global_current_state.ignition_select){
        //we need to change the ignition_select to what requested wants
        global_current_state.ignition_select = global_requested_state.ignition_select;
    }
}

//the client sent a NACK, so copy current state to requested state
void reset_request(){
    global_requested_state.valve_1 = global_current_state.valve_1;
    global_requested_state.valve_2 = global_current_state.valve_2;
    global_requested_state.valve_3 = global_current_state.valve_3;
    global_requested_state.valve_4 = global_current_state.valve_4;
    global_requested_state.injector_valve = global_current_state.injector_valve;
    global_requested_state.ignition_power = global_current_state.ignition_power;
    global_requested_state.ignition_select = global_current_state.ignition_select;
}

void init_outputs(){
    pinMode(PIN_VALVE_1_POWER, OUTPUT);
    pinMode(PIN_VALVE_1_SELECT, OUTPUT);
    pinMode(PIN_VALVE_2_POWER, OUTPUT);
    pinMode(PIN_VALVE_2_SELECT, OUTPUT);
    pinMode(PIN_VALVE_3_POWER, OUTPUT);
    pinMode(PIN_VALVE_3_SELECT, OUTPUT);
    pinMode(PIN_VALVE_4_POWER, OUTPUT);
    pinMode(PIN_VALVE_4_SELECT, OUTPUT);
    pinMode(PIN_INJECTOR_VALVE_POWER, OUTPUT);
    pinMode(PIN_INJECTOR_VALVE_SELECT, OUTPUT);

    pinMode(PIN_IGNITION_POWER, OUTPUT);
    pinMode(PIN_IGNITION_SELECT, OUTPUT);
    
    //close the valves. This is the safest startup state

    digitalWrite((uint8_t) PIN_VALVE_1_POWER, HIGH);
    digitalWrite((uint8_t) PIN_VALVE_1_SELECT, HIGH); //select pin going high should set it to close, if it's wired right

    digitalWrite((uint8_t) PIN_VALVE_2_POWER, HIGH);
    digitalWrite((uint8_t) PIN_VALVE_2_SELECT, HIGH);
    
    digitalWrite((uint8_t) PIN_VALVE_3_POWER, HIGH);
    digitalWrite((uint8_t) PIN_VALVE_3_SELECT, HIGH);

    digitalWrite((uint8_t) PIN_VALVE_4_POWER, HIGH);
    digitalWrite((uint8_t) PIN_VALVE_4_SELECT, HIGH);
    
    digitalWrite((uint8_t) PIN_INJECTOR_VALVE_POWER, HIGH);
    digitalWrite((uint8_t) PIN_INJECTOR_VALVE_SELECT, HIGH);

    //run tank valve
    //TODO, figure out how the run tank valve is going to work
    //likely involve a serial link to another arduino
    //I don't think this should be here, we have RocketCAN - Zach 2020-02-20

    linac_init();

    //ignition, set to off by default
    digitalWrite((uint8_t) PIN_IGNITION_POWER, LOW);
    digitalWrite((uint8_t) PIN_IGNITION_SELECT, LOW);

}

//called when client hasn't told us to do anything for a while
//assume comms are entirely lost, close all the valves (except the
//rocket tank vent valve)
void goto_safe_mode()
{
    //take the current requested state, set it to the safest possible
    //state, and then call apply_state
    actuator_state_t* requested = get_requested_state();

    //close
    requested->valve_1 = 0;
    //open
    requested->valve_2 = 1;
    //open
    requested->valve_3 = 1;
    //close
    requested->valve_4 = 0;
    //close
    requested->injector_valve = 0;

    //turn off ignition
    requested->ignition_power = 0;
    requested->ignition_select = 0;


    //apply that state
    apply_state();

    //tell rocketCAN to depower the system bus
    nio_depower_bus();
}

void tower_handle_rocketcan_update(const system_state *update)
{
    global_current_daq.num_boards_connected = update->num_boards_connected;
    global_current_daq.any_errors_detected = update->any_errors_detected;
    global_current_daq.pressure3 = update->tank_pressure;
    global_current_daq.injector_valve_state = update->injector_valve_state;
    global_current_daq.rocketvent_valve_state = update->vent_valve_state;
    global_current_daq.bus_batt_mv = update->bus_battery_voltage_mv;
    global_current_daq.vent_batt_mv = update->vent_battery_voltage_mv;
    //TODO, log to rlcslog
}

//global for how long it's been since the output log was flushed
//to the SD card
unsigned long global_time_last_output_flush = 0;
const unsigned long global_output_flush_interval = 10000;
//global for how long it's been since we logged daq values
unsigned long global_time_last_logged_daq = 0;
const unsigned long global_time_between_daq_logs = 1000;
