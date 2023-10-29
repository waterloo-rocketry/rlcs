#include "common/config.cpp" // cursed subfolder compile
#include "common/communication.hpp"
#include "config.hpp"
#include "hardware.hpp"
#include "lcd.hpp"

void setup() {
  hardware::setup();
  hardware::set_status_startup();
  lcd::setup();

  Serial.begin(115200); // USB connection
  Serial3.begin(9600);  // Towerside connection

  Communicator<ActuatorMessage, SensorMessage> towerside_communicator{
      Serial3, config::COMMUNICATION_RESET_MS};
  Communicator<config::USBMessage, int> usb_communicator{
      Serial, config::COMMUNICATION_RESET_MS};
  unsigned long last_sent_time = 0;
  ActuatorMessage last_switch_positions = build_safe_state(ActuatorMessage());
  SensorMessage last_sensor_msg;

  hardware::set_status_disconnected();
  // Avoid the status showing as connected for the first few seconds on
  // startup if we aren't really
  bool any_messages_received = false;
  while (true) {
    towerside_communicator.read_byte();
    if (towerside_communicator.get_message(&last_sensor_msg)) {
      any_messages_received = true;
      lcd::update(last_sensor_msg);
    }

    bool has_contact = towerside_communicator.seconds_since_last_contact() <
                       config::COMMUNICATION_TIMEOUT_S;
    if (has_contact && any_messages_received) {
      hardware::set_status_connected();
    } else {
      hardware::set_status_disconnected();
    }

    bool armed = hardware::is_armed();
    hardware::set_missile_leds(armed);
    if (armed) {
      last_switch_positions = config::build_command_message();
    }
    last_switch_positions.client_side_arm_status = armed;

    if ((millis() > last_sent_time + config::COMMAND_MESSAGE_INTERVAL_MS)) {
      // condition: passed COMMAND_MESSAGE_INTERVAL_MS since last time sent message
      last_sent_time = millis();
      towerside_communicator.send(last_switch_positions);

      usb_communicator.send(config::USBMessage{
        .actuator_msg = last_switch_positions,
        .sensor_msg = last_sensor_msg
      });
    }
  }
}

void loop() {}
