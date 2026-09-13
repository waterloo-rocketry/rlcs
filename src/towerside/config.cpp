#include "actuators.hpp"
#include "config.hpp"
#include "errors.hpp"
#include "sensors.hpp"

namespace config {

struct Actuators {
  actuator::I2C ov101{2};
  actuator::I2C ov102{3};
  actuator::I2C ov103{4};
  actuator::I2C nv201{5};
  actuator::I2C cdv401{6};
  actuator::I2C qd301{7};
  actuator::I2C ov302{8};
  actuator::I2C pyro_valve{9};
  actuator::I2C rocket_charging{10};
  actuator::Ignition ignition_primary{1};
  actuator::Ignition ignition_secondary{11};
  actuator::Heater heater_1{16};
  actuator::Heater heater_2{17};
} ACTUATORS;

void apply(const ActuatorMessage &command) {
  ACTUATORS.ov101.set(command.ov101);
  ACTUATORS.ov102.set(command.ov102);
  ACTUATORS.ov103.set(command.ov103);
  // ACTUATORS.ov301.set(!command.ov301);
  ACTUATORS.nv201.set(command.nv201);
  ACTUATORS.cdv401.set(command.cdv401);
  ACTUATORS.qd301.set(command.qd301);
  ACTUATORS.ov302.set(command.ov302);
  ACTUATORS.pyro_valve.set(command.pyro_valve);
  ACTUATORS.rocket_charging.set(command.rocket_charging);
  ACTUATORS.ignition_primary.set(command.ignition_primary);
  ACTUATORS.ignition_secondary.set(command.ignition_secondary);
  ACTUATORS.heater_1.set(command.tank_heating_1);
  ACTUATORS.heater_2.set(command.tank_heating_2);
}

SensorMessage build_sensor_message() {
  Serial.println (ACTUATORS.ignition_primary.get_current_ma(0));
  return SensorMessage{
      .towerside_main_batt_mv = sensors::get_main_batt_mv(),
      .towerside_actuator_batt_mv = sensors::get_actuator_batt_mv(),
      .error_code = errors::pop(),
      .towerside_armed = sensors::is_armed(),
      .has_contact = sensors::has_contact(),
      .ignition_primary_ma = ACTUATORS.ignition_primary.get_current_ma(0),
      .ignition_secondary_ma = ACTUATORS.ignition_secondary.get_current_ma(0),
      .ov101_state = ACTUATORS.ov101.get_state(),
      .ov102_state = ACTUATORS.ov102.get_state(),
      .cdv401_state = ACTUATORS.cdv401.get_state(),
      .nv201_state = ACTUATORS.nv201.get_state(),
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
