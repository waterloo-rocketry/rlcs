#include "lcd.hpp"
#include "common/mock_arduino.hpp"

#include <stdio.h>

#include "common/shared_types.hpp"
#include "config.hpp"
#include "hardware.hpp"
#include "pinout.hpp"

namespace lcd {

LiquidCrystal liquid_crystal{pinout::LCD_RS, pinout::LCD_EN, pinout::LCD_D4,
                             pinout::LCD_D5, pinout::LCD_D6, pinout::LCD_D7};

void print_valve_position(uint16_t pos) {
  switch (pos) {
  case ActuatorPosition::ActuatorPosition::error:
    liquid_crystal.print("ERR");
    break;
  case ActuatorPosition::ActuatorPosition::closed:
    liquid_crystal.print("CLS");
    break;
  case ActuatorPosition::ActuatorPosition::open:
    liquid_crystal.print("OPN");
    break;
  case ActuatorPosition::ActuatorPosition::unknown:
    liquid_crystal.print("UNK");
    break;
  default:
    liquid_crystal.print("???");
    break;
  }
};


void print_decimal_value(unsigned int num) {
  char buf[4];
  snprintf(buf, 4, "%03u", num);
  liquid_crystal.print(buf);
}

void print_decimal_value_wide(unsigned int num) {
  char buf[7];
  snprintf(buf, 7, "%05u", num);
  buf[5] = buf[4];
  buf[4] = buf[3];
  buf[3] = buf[2];
  buf[2] = '.';
  buf[6] = '\0';
  liquid_crystal.print(buf);
}
	
void setup() {
  liquid_crystal.begin(20, 4);
  liquid_crystal.clear();
  liquid_crystal.setCursor(2, 1);
  liquid_crystal.print("Waiting for data");
  liquid_crystal.setCursor(14, 3);
  liquid_crystal.print("CB:");
  print_decimal_value(hardware::get_batt_dv());
}

void update(SensorMessage msg) {
  liquid_crystal.setCursor(0, 0);
  liquid_crystal.print("V1:");
  print_valve_position(msg.valve_1_state);

  liquid_crystal.print(" V2:");
  print_valve_position(msg.valve_2_state);

  liquid_crystal.print(" V3:");
  print_valve_position(msg.valve_3_state);

  liquid_crystal.setCursor(0, 1);
  liquid_crystal.print("IP:");
  print_decimal_value_wide(msg.ignition_primary_ma);
 
  liquid_crystal.print(" IS:");
  print_decimal_value_wide(msg.ignition_secondary_ma);

  liquid_crystal.print(" ");

  liquid_crystal.setCursor(0, 2);
  liquid_crystal.print("EC:");
  print_decimal_value(msg.error_code);

  liquid_crystal.print(" CON:");
  liquid_crystal.print(msg.has_contact ? "Y" : "N");

  liquid_crystal.print(" ARM:");
  liquid_crystal.print(msg.towerside_armed ? "Y" : "N");

  liquid_crystal.setCursor(0, 3);
  liquid_crystal.print("TM:");
  print_decimal_value(msg.towerside_main_batt_mv / 100);

  liquid_crystal.print(" TA:");
  print_decimal_value(msg.towerside_actuator_batt_mv / 100);

  liquid_crystal.print(" CB:");
  print_decimal_value(hardware::get_batt_dv());
}

} // namespace lcd
