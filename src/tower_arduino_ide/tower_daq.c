#include "tower_daq.h"
#include "shared_types.h"
#include "tower_pin_defines.h"
#include "Arduino.h"
#include "sd_handler.h"

//used for holding intermediate values
static struct {
    //all analog daq values range from 0 to 1023 (10 bit ADC)
    //so they are stored in 16 bit unsigneds
    uint16_t mass;
    uint16_t pressure1;
    uint16_t pressure2;
    uint16_t curr_ignition_pri;
    uint16_t curr_ignition_sec;

    uint32_t battery_main;
    uint32_t battery_actuators;

    //all limit switches are digital values (either 1 or 0)
    //these could be packed into a bit field, but I'm not 
    //at all worried about space, so am currently using uint8_t's
    uint8_t lsw_valve_1_opn;
    uint8_t lsw_valve_1_cls;
    uint8_t lsw_valve_2_opn;
    uint8_t lsw_valve_2_cls;
    uint8_t lsw_valve_3_opn;
    uint8_t lsw_valve_3_cls;
    uint8_t lsw_valve_4_opn;
    uint8_t lsw_valve_4_cls;
    uint8_t lsw_injector_valve_opn;
    uint8_t lsw_injector_valve_cls;
} window_holder[WINDOW_WIDTH];


void init_daq_pins() {
    pinMode(PIN_DAQ_PRESSURE1, INPUT);
    pinMode(PIN_DAQ_PRESSURE2, INPUT);
    pinMode(PIN_DAQ_MASS, INPUT);
    pinMode(PIN_CURRENT_IGNITION_PRI, INPUT);
    pinMode(PIN_CURRENT_IGNITION_SEC, INPUT);
    pinMode(PIN_BATTERY_MAIN, INPUT);
    pinMode(PIN_BATTERY_ACTUATORS, INPUT);
    pinMode(PIN_LIMITSW_VALVE_1_OPN, INPUT);
    pinMode(PIN_LIMITSW_VALVE_1_CLS, INPUT);
    pinMode(PIN_LIMITSW_VALVE_2_OPN, INPUT);
    pinMode(PIN_LIMITSW_VALVE_2_CLS, INPUT);
    pinMode(PIN_LIMITSW_VALVE_3_OPN, INPUT);
    pinMode(PIN_LIMITSW_VALVE_3_CLS, INPUT);
    pinMode(PIN_LIMITSW_VALVE_4_OPN, INPUT);
    pinMode(PIN_LIMITSW_VALVE_4_CLS, INPUT);
    pinMode(PIN_LIMITSW_INJECTOR_VALVE_OPN, INPUT);
    pinMode(PIN_LIMITSW_INJECTOR_VALVE_CLS, INPUT);
    
    //set all the values in our window holder to 0
    memset(&window_holder, 0, sizeof(window_holder));
}

static int window_holder_index = 0;
void read_daq_pins() {
    //read all analog values into window holder
    window_holder[window_holder_index].mass = 
        analogRead(PIN_DAQ_MASS);
    window_holder[window_holder_index].pressure1 = 
        analogRead(PIN_DAQ_PRESSURE1);
    window_holder[window_holder_index].pressure2 = 
        analogRead(PIN_DAQ_PRESSURE2);
    window_holder[window_holder_index].curr_ignition_pri = 
        analogRead(PIN_CURRENT_IGNITION_PRI);
    window_holder[window_holder_index].curr_ignition_sec = 
        analogRead(PIN_CURRENT_IGNITION_SEC);
    window_holder[window_holder_index].battery_main =
        analogRead(PIN_BATTERY_MAIN);
    window_holder[window_holder_index].battery_actuators =
        analogRead(PIN_BATTERY_ACTUATORS);

    //now read in all the digital values
    window_holder[window_holder_index].lsw_valve_1_opn =
        digitalRead(PIN_LIMITSW_VALVE_1_OPN);
    window_holder[window_holder_index].lsw_valve_1_cls =
        digitalRead(PIN_LIMITSW_VALVE_1_CLS);
    window_holder[window_holder_index].lsw_valve_2_opn =
        digitalRead(PIN_LIMITSW_VALVE_2_OPN);
    window_holder[window_holder_index].lsw_valve_2_cls =
        digitalRead(PIN_LIMITSW_VALVE_2_CLS);
    window_holder[window_holder_index].lsw_valve_3_opn =
        digitalRead(PIN_LIMITSW_VALVE_3_OPN);
    window_holder[window_holder_index].lsw_valve_3_cls =
        digitalRead(PIN_LIMITSW_VALVE_3_CLS);
    window_holder[window_holder_index].lsw_valve_4_opn =
        digitalRead(PIN_LIMITSW_VALVE_4_OPN);
    window_holder[window_holder_index].lsw_valve_4_cls =
        digitalRead(PIN_LIMITSW_VALVE_4_CLS);
    window_holder[window_holder_index].lsw_injector_valve_opn =
        digitalRead(PIN_LIMITSW_INJECTOR_VALVE_OPN);
    window_holder[window_holder_index].lsw_injector_valve_cls =
        digitalRead(PIN_LIMITSW_INJECTOR_VALVE_CLS);
    
    //increment window counter, check if it's bigger than the window,
    //if so, set it to 0
    if(++window_holder_index >= WINDOW_WIDTH)
        window_holder_index = 0;
}



void compute_daq_values(daq_holder_t* output) {
    //max value of analog read is 2^10, so we can add up up to 2^6
    //samples in a uint16_t without worrying about overflow
    for(int i = 0; i < WINDOW_WIDTH; i++){
        output->pressure1                  += window_holder[i].pressure1;
        output->pressure2                  += window_holder[i].pressure2;
        output->rocket_mass                += window_holder[i].mass;
        output->ign_pri_current            += window_holder[i].curr_ignition_pri;
        output->ign_sec_current            += window_holder[i].curr_ignition_sec;
        output->rlcs_main_batt_mv          += window_holder[i].battery_main;
        output->rlcs_actuator_batt_mv      += window_holder[i].battery_actuators;
        output->valve_1_lsw_open             += window_holder[i].lsw_valve_1_opn;
        output->valve_1_lsw_closed           += window_holder[i].lsw_valve_1_cls;
        output->valve_2_lsw_open             += window_holder[i].lsw_valve_2_opn;
        output->valve_2_lsw_closed           += window_holder[i].lsw_valve_2_cls;
        output->valve_3_lsw_open           += window_holder[i].lsw_valve_3_opn;
        output->valve_3_lsw_closed          += window_holder[i].lsw_valve_3_cls;
        output->valve_4_lsw_open     += window_holder[i].lsw_valve_4_opn;
        output->valve_4_lsw_closed   += window_holder[i].lsw_valve_4_cls;
        output->injector_valve_lsw_open     += window_holder[i].lsw_injector_valve_opn;
        output->injector_valve_lsw_closed   += window_holder[i].lsw_injector_valve_cls;
        
    }

    //apply scaling values. These need calibration, so I'll just use
    //placeholders for now. There are defined in the header
    output->pressure1 *= PRESSURE1_SCALE;
    output->pressure1 /= WINDOW_WIDTH;
    if(output->pressure1 > (-PRESSURE1_OFFSET)){
        output->pressure1 += PRESSURE1_OFFSET;
    } else {
        output->pressure1 = 0;
    }

    output->pressure2 *= PRESSURE2_SCALE;
    output->pressure2 /= WINDOW_WIDTH;
    if(output->pressure2 > (-PRESSURE2_OFFSET)){
        output->pressure2 += PRESSURE2_OFFSET;
    } else {
        output->pressure2 = 0;
    }

    output->rocket_mass *= MASS_SCALE;
    output->rocket_mass /= WINDOW_WIDTH;
    if(output->rocket_mass > (-MASS_OFFSET)){
        output->rocket_mass += MASS_OFFSET;
    } else {
        output->rocket_mass = 0;
    }

    output->ign_pri_current *= IGNPRI_SCALE;
    output->ign_pri_current /= WINDOW_WIDTH;
    output->ign_pri_current += IGNPRI_OFFSET;

    output->ign_sec_current *= IGNSEC_SCALE;
    output->ign_sec_current /= WINDOW_WIDTH;
    output->ign_sec_current += IGNSEC_OFFSET;

    output->rlcs_main_batt_mv *= MAIN_BATT_SCALE;
    output->rlcs_main_batt_mv /= WINDOW_WIDTH;
    output->rlcs_main_batt_mv += MAIN_BATT_OFFSET;

    output->rlcs_actuator_batt_mv *= ACTUATOR_BATT_SCALE;
    output->rlcs_actuator_batt_mv /= WINDOW_WIDTH;
    output->rlcs_actuator_batt_mv += ACTUATOR_BATT_OFFSET;

    //the on off values just need to be divided by the width
    output->valve_1_lsw_open = (output->valve_1_lsw_open / WINDOW_WIDTH) != 0;
    output->valve_1_lsw_closed = (output->valve_1_lsw_closed / WINDOW_WIDTH) != 0;
    output->valve_2_lsw_open = (output->valve_2_lsw_open / WINDOW_WIDTH) != 0;
    output->valve_2_lsw_closed = (output->valve_2_lsw_closed / WINDOW_WIDTH) != 0;
    output->valve_3_lsw_open = (output->valve_3_lsw_open / WINDOW_WIDTH) != 0;
    output->valve_3_lsw_closed = (output->valve_3_lsw_closed / WINDOW_WIDTH) != 0;
    output->valve_4_lsw_open = (output->valve_4_lsw_open / WINDOW_WIDTH) != 0;
    output->valve_4_lsw_closed = (output->valve_4_lsw_closed / WINDOW_WIDTH) != 0;
    output->injector_valve_lsw_open = (output->injector_valve_lsw_open / WINDOW_WIDTH) != 0;
    output->injector_valve_lsw_closed = (output->injector_valve_lsw_closed / WINDOW_WIDTH) != 0;
    
    //log what we just computed
    // TODO
    rlcslog_log_daq_values(output);
}