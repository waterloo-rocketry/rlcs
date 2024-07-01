#include "config.hpp"
#include "common/mock_arduino.hpp"
#include "pinout.hpp"

namespace config {

ActuatorMessage build_command_message() {
  return ActuatorMessage{
      .ov101 = digitalRead(pinout::MISSILE_SWITCH_1),
      .ov102 = digitalRead(pinout::MISSILE_SWITCH_2),
      .nv201 = digitalRead(pinout::MISSILE_SWITCH_3),
      .nv202 = digitalRead(pinout::MISSILE_SWITCH_4),
	  .vent_valve = digitalRead(pinout::MISSILE_SWITCH_5),
	  .injector_valve = digitalRead(pinout::MISSILE_SWITCH_INJECTOR),
	  .fill_dump_valve = digitalRead(pinout::MISSILE_SWITCH_6),
      .fill_disconnect = digitalRead(pinout::MISSILE_SWITCH_7),
	  .rocket_power = digitalRead(pinout::MISSILE_SWITCH_IGNITION_SEC), // Out of switch, should use omnibus dashboard
	  .tank_heating_1 = digitalRead(pinout::MISSILE_SWITCH_8),
	  .tank_heating_2 = digitalRead(pinout::MISSILE_SWITCH_8),
      .ignition_primary =
          digitalRead(pinout::MISSILE_SWITCH_IGNITION_PRI) &&
          !digitalRead(pinout::MISSILE_SWITCH_IGNITION_SEC) &&
          !digitalRead(pinout::MISSILE_SWITCH_IGNITION_FIRE), // active low
      .ignition_secondary =
          digitalRead(pinout::MISSILE_SWITCH_IGNITION_SEC) &&
          !digitalRead(pinout::MISSILE_SWITCH_IGNITION_PRI) &&
          !digitalRead(pinout::MISSILE_SWITCH_IGNITION_FIRE), // active low
  };
}

} // namespace config
