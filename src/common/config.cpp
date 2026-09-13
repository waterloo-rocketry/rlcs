#include "config.hpp"

ActuatorMessage build_safe_state(const ActuatorMessage &current_state) {
  return ActuatorMessage{
      .v305 = false,
      .v301 = false,
      .v405 = false,
      .tank_heating_1 = false,
      .tank_heating_2 = false,
      .ignition_primary = false,
      .ignition_secondary = false,
  };
}