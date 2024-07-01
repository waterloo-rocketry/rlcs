#include "config.hpp"

ActuatorMessage build_safe_state(const ActuatorMessage &current_state) {
  return ActuatorMessage{
      .ov101 = false,
      .ov102 = true,
      .nv201 = false,
      .nv202 = true,
	  .vent_valve = true,
	  .injector_valve = current_state.injector_valve,
	  .fill_dump_valve = true,
	  .fill_disconnect = current_state.fill_disconnect,
	  .rocket_power = current_state.rocket_power,
	  .tank_heating_1 = false,
	  .tank_heating_2 = false,
      .ignition_primary = false,
      .ignition_secondary = false,
  };
}
