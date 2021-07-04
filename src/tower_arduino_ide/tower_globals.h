#ifndef TOWER_GLOBALS_H
#define TOWER_GLOBALS_H

#include "shared_types.h"
#include "nodeio.ioio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define millis_offset millis

//state that the client last asked us to do
actuator_state_t* get_requested_state();
//state that we are currently outputting to the actuators
actuator_state_t* get_current_state();

daq_holder_t* get_global_current_daq();

void apply_state();
void reset_request();

//global functions that we just need
void init_outputs();
void goto_safe_mode();

//callback for nodeio.ioio to give us updates on stuff
void tower_handle_rocketcan_update(const system_state *update);

#ifdef __cplusplus
}
#endif

#endif //ifndef TOWER_GLOBALS_H