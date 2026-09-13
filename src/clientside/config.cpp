#include "config.hpp"
#include "common/mock_arduino.hpp"
#include "pinout.hpp"

namespace config {

ActuatorMessage build_command_message() {
  return ActuatorMessage{
      .v305 = digitalRead(pinout::MISSILE_SWITCH_1),
      .v301 = !digitalRead(pinout::MISSILE_SWITCH_2),
      .v405 = digitalRead(pinout::MISSILE_SWITCH_3),
      .tank_heating_1 = digitalRead(pinout::MISSILE_SWITCH_8),
      .tank_heating_2 = digitalRead(pinout::MISSILE_SWITCH_8),
      .ignition_primary =
          digitalRead(pinout::MISSILE_SWITCH_IGNITION_PRI) &&
          !digitalRead(pinout::MISSILE_SWITCH_IGNITION_FIRE), // active low
      .ignition_secondary = // fire secondary ignition with primary ignition switch
        digitalRead(pinout::MISSILE_SWITCH_IGNITION_PRI) &&
        !digitalRead(pinout::MISSILE_SWITCH_IGNITION_FIRE), // active low
  };
}

} // namespace config
