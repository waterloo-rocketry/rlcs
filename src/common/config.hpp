#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H

#include "mock_arduino.hpp"
#include "shared_types.hpp"

// These types act as generic containers for actuators and sensors respectively. This forces
// code to expicitely refer to actuators and sensor by their names rather than generic indexes,
// which then lets the compiler ensure that there are no mismatches.
//
// Note: The reason these are structs and not classes (and don't have constructors) is so we can use named struct initialization, ie:
//    ActuatorMessage<bool> command = {
//        .valve_1 = true,
//        .valve_2 = false,
//        ...etc
//    };
// When we compile with -Wextra, gcc will warn us about missing members.

// Disable struct field padding
; // random semicolon to fix clangd warning bug, see: https://stackoverflow.com/questions/72456118/why-does-clang-give-a-warning-unterminated-pragma-pack-push-at-end-of-f
#pragma pack(push, 1)
struct ActuatorMessage {
  bool ov101; // Ox Fill
  bool ov102; // Ox Drain
  bool nv201; // N2 Purge
  bool nv202; // N2 Drain (Solenoid)
  bool vent_valve; // To DAQ laptop only
  bool injector_valve; // To DAQ laptop only
  bool fill_dump_valve; // To DAQ laptop only
  bool fill_disconnect;
  bool rocket_power;
  bool tank_heating_1;
  bool tank_heating_2;
  bool ignition_primary;
  bool ignition_secondary;

  bool operator==(const ActuatorMessage &other) const {
    return !memcmp(this, &other, sizeof(ActuatorMessage));
  }
};

ActuatorMessage build_safe_state(const ActuatorMessage &current_state);

struct SensorMessage {
  // Battery Voltages
  uint16_t towerside_main_batt_mv;
  uint16_t towerside_actuator_batt_mv;
  // Actuator health
  uint16_t error_code;
  bool towerside_armed;
  bool has_contact;
  // Ignition currents
  uint16_t ignition_primary_ma;
  uint16_t ignition_secondary_ma;
  // Actuator states
  ActuatorPosition::ActuatorPosition ov101_state;
  ActuatorPosition::ActuatorPosition ov102_state;
  ActuatorPosition::ActuatorPosition nv201_state;
  ActuatorPosition::ActuatorPosition nv202_state;
  // Tank Heating
  uint16_t heater_thermistor_1;
  uint16_t heater_thermistor_2;
  uint16_t heater_current_ma_1;
  uint16_t heater_current_ma_2;
  uint16_t heater_batt_mv_1;
  uint16_t heater_batt_mv_2;
  uint16_t heater_kelvin_low_mv_1;
  uint16_t heater_kelvin_low_mv_2;
  uint16_t heater_kelvin_high_mv_1;
  uint16_t heater_kelvin_high_mv_2;
};
#pragma pack(pop)

#endif
