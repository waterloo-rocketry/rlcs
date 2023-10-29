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
  bool valve_1;
  bool valve_2;
  bool valve_3;
  bool valve_4;
  bool vent_valve;
  bool injector_valve;
  bool ignition_primary;
  bool ignition_secondary;
  bool rocket_power;
  bool fill_disconnect;
  //<custom remote arming enum> remote_arming;
  bool client_side_arm_status;
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
  ActuatorPosition::ActuatorPosition valve_1_state;
  ActuatorPosition::ActuatorPosition valve_2_state;
  ActuatorPosition::ActuatorPosition valve_3_state;
  ActuatorPosition::ActuatorPosition valve_4_state;
  ActuatorPosition::ActuatorPosition fill_disconnect_state;
};
#pragma pack(pop)

#endif
