#include "actuators.hpp"
#include "config.hpp"
#include "errors.hpp"
#include "sensors.hpp"

namespace config {

struct Actuators {
  actuator::I2C ov101{1};
  actuator::I2C ov102{2};
  actuator::I2C nv201{3};
  actuator::I2C nv202{4};
  actuator::I2C fill_disconnect{5};
  actuator::Ignition ignition_primary{6};
  actuator::Ignition ignition_secondary{7};
  actuator::I2C rocket_power{8};
  actuator::Heater heater_1{16};
  actuator::Heater heater_2{17};
} ACTUATORS;

void apply(const ActuatorMessage &command) {
  ACTUATORS.ov101.set(command.ov101);
  ACTUATORS.ov102.set(command.ov102);
  ACTUATORS.nv201.set(command.nv201);
  ACTUATORS.nv202.set(command.nv202);
  ACTUATORS.fill_disconnect.set(command.fill_disconnect);
  ACTUATORS.ignition_primary.set(command.ignition_primary);
  ACTUATORS.ignition_secondary.set(command.ignition_primary); // fire both ignitions in response to ignition_primary
  ACTUATORS.rocket_power.set(!command.rocket_power); // inverted
  ACTUATORS.heater_1.set(command.tank_heating_1);
  ACTUATORS.heater_2.set(command.tank_heating_2);
}

SensorMessage build_sensor_message() {
  return SensorMessage{
      .towerside_main_batt_mv = sensors::get_main_batt_mv(),
      .towerside_actuator_batt_mv = sensors::get_actuator_batt_mv(),
      .error_code = errors::pop(),
      .towerside_armed = sensors::is_armed(),
      .has_contact = sensors::has_contact(),
      .ignition_primary_ma = ACTUATORS.ignition_primary.get_current_ma(1),
      .ignition_secondary_ma = ACTUATORS.ignition_secondary.get_current_ma(1),
      .ov101_state = ACTUATORS.ov101.get_state(),
      .ov102_state = ACTUATORS.ov102.get_state(),
      .nv201_state = ACTUATORS.nv201.get_state(),
	  .nv202_state = ACTUATORS.nv202.get_state(),
	  .heater_thermistor_1 = ACTUATORS.heater_1.get_thermistor(),
	  .heater_thermistor_2 = ACTUATORS.heater_2.get_thermistor(),
	  .heater_current_ma_1 = ACTUATORS.heater_1.get_current_ma(),
	  .heater_current_ma_2 = ACTUATORS.heater_2.get_current_ma(),
	  .heater_batt_mv_1 = ACTUATORS.heater_1.get_batt_voltage(),
	  .heater_batt_mv_2 = ACTUATORS.heater_2.get_batt_voltage(),
	  .heater_kelvin_low_mv_1 = ACTUATORS.heater_1.get_kelvin_low_voltage(),
	  .heater_kelvin_low_mv_2 = ACTUATORS.heater_2.get_kelvin_low_voltage(),
	  .heater_kelvin_high_mv_1 = ACTUATORS.heater_1.get_kelvin_high_voltage(),
	  .heater_kelvin_high_mv_2 = ACTUATORS.heater_2.get_kelvin_high_voltage()
	  };
}

} // namespace config
