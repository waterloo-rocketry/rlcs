#include "config.hpp"

ActuatorMessage build_safe_state(const ActuatorMessage &current_state) {
  return ActuatorMessage{
      .ov101 = false,
      .ov102 = false,
      .ov103 = false,
      .nv201 = false,
      .cdv401 = false,
      .qd301 = false,
      .ov302 = false,
      .pyro_valve = false,
      .rocket_charging = false,
      .tank_heating_1 = false,
      .tank_heating_2 = false,
      .ignition_primary = false,
      .ignition_secondary = false,
  };
}