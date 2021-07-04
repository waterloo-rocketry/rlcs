#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include<stdint.h>
#include<stdbool.h>

//the main type for holding the state of the actuators
//can be used for desired or actual actuator state
//the definition of these in these comments are the canon
typedef struct {
    unsigned char valve_1 : 1;    //1 = open, 0 = closed
    unsigned char valve_2 : 1;    //1 = open, 0 = closed
    unsigned char valve_3 : 1;    //1 = open, 0 = closed
    unsigned char valve_4 : 1;    //1 = open, 0 = closed
    unsigned char injector_valve : 1;       //1 = open, 0 = closed
    unsigned char ignition_power : 1;       //1 = firing, 0 = not firing
    unsigned char ignition_select : 1;      //1 = secondary; 0 = primary
} __attribute__((packed)) actuator_state_t;

int actuator_compare(const actuator_state_t* s, const actuator_state_t* q);

typedef enum {
    DAQ_VALVE_OPEN,
    DAQ_VALVE_CLOSED,
    DAQ_VALVE_UNK,
    DAQ_VALVE_ILLEGAL
} valve_state_t;

typedef struct {
    uint16_t pressure1; //fill tank pressure
    uint16_t pressure2; //fill line pressure
    uint16_t pressure3; //rocket tank pressure
    uint16_t rocket_mass;                      //between 0 and 999 (measured in decipounds. Don't judge me)
    uint16_t ign_pri_current;                  //between 0 and 999 (measured in centiamps. these dumb units are for easy human readable output)
    uint16_t ign_sec_current;                  //between 0 and 999 (measured in centiamps)
    unsigned char valve_1_lsw_open;              //0 when low, 1 when high (which means remote fill is open)
    unsigned char valve_1_lsw_closed;            //0 when low, 1 when high (which means remote fill is closed)
    unsigned char valve_2_lsw_open;              //0 when low, 1 when high (which means remote vent is open)
    unsigned char valve_2_lsw_closed;            //0 when low, 1 when high (which means remote vent is closed)
    unsigned char valve_3_lsw_open;            //0 when low, 1 when high (which means the actuator is extended)
    unsigned char valve_3_lsw_closed;           //0 when low, 1 when high (which means the actuator is retracted)
    unsigned char valve_4_lsw_open;      //0 when low, 1 when high (which means ox pressurant is open)
    unsigned char valve_4_lsw_closed;    //0 when low, 1 when high (which means ox pressurant is closed)
    unsigned char injector_valve_lsw_open;      //0 when low, 1 when high (which means ox pressurant is open)
    unsigned char injector_valve_lsw_closed;    //0 when low, 1 when high (which means ox pressurant is closed)
    
    // new data fields for RocketCAN stuff
    unsigned char num_boards_connected;
    unsigned char bus_is_powered;
    bool any_errors_detected;
    valve_state_t injector_valve_state;
    valve_state_t rocketvent_valve_state;

    uint32_t bus_batt_mv;
    uint32_t vent_batt_mv;
    uint32_t rlcs_main_batt_mv;
    uint32_t rlcs_actuator_batt_mv;
} daq_holder_t;

#ifdef __cplusplus
}
#endif

#endif //ifndef SHARED_TYPES_H
