#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#include "common/config.hpp"

namespace config {

void apply(const ActuatorMessage &command);
SensorMessage build_sensor_message();

constexpr uint16_t COMMUNICATION_TIMEOUT_S = 10; // Report loss of contact after this many seconds without contact
constexpr uint16_t SAFE_STATE_TIMEOUT_S = 180; // Go to safe state after this many seconds without contact
constexpr unsigned long SENSOR_MSG_INTERVAL_MS = 1000; // Rate to send sensor messages at
constexpr unsigned long COMMUNICATION_RESET_MS = 50; // maximum time between successive characters in the same message

} // namespace config

#endif
