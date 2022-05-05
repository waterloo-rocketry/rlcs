#include "common/mock_arduino.hpp"
#include "common/communication/receiver.hpp"
#include "common/communication/sender.hpp"
#include "common/tickable.hpp"
#include "config.hpp"
#include "data_handlers.hpp"
#include "daq.hpp"
#include "hardware.hpp"
#include "pinout.hpp"

void setup() {
  Hardware::setup();
  Config::setup();
  auto connection = Communication::SerialConnection(Serial);
  auto lcd_handler = DataHandler::LCDDisplay(SensorID::rlcs_main_batt_mv,
                                             SensorID::rlcs_actuator_batt_mv,
                                             SensorID::healthy_actuators,
                                             SensorID::fill_valve_state,
                                             SensorID::vent_valve_state,
                                             SensorID::injector_valve_state,
                                             SensorID::ignition_primary_ma,
                                             SensorID::ignition_secondary_ma);
  auto encoder = Communication::HexEncoder<ActuatorCommand>();
  auto decoder = Communication::HexDecoder<SensorData>();
  auto receiver = Communication::MessageReceiver<SensorData>(decoder, connection,
                                                            &lcd_handler);
  auto sender = Communication::MessageSender<ActuatorCommand>(encoder, connection);

  unsigned long last_message_sent = 0;
  ActuatorCommand last_switch_positions = DAQ::get_switch_positions();
  while (true) {
    Tickable::trigger_tick();
    if (millis() - last_message_sent > Config::SEND_STATUS_INTERVAL_MS) {
      last_message_sent = millis();

      bool armed = !digitalRead(Pinout::KEY_SWITCH_IN);
      if (armed) {
        last_switch_positions = DAQ::get_switch_positions();
      }
      for (uint8_t i = 0; i < sizeof(Pinout::MISSILE_LEDS) / sizeof(Pinout::MISSILE_LEDS[0]); i++) {
        digitalWrite(Pinout::MISSILE_LEDS[i], !armed);
      }

      sender.send(last_switch_positions);
    }
  }
}

void loop() {
}