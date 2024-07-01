#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <stdint.h>

#include "common/mock_arduino.hpp"
#include "common/shared_types.hpp"
#include "errors.hpp"

namespace actuator {

class Actuator {
public:
  // The suggested actuator interface is
  // void set(bool value)
  // void get_<sensor>_<units>()
  // but it can be modified based on specific actuator requirements
};

class I2C: public Actuator {
  uint8_t slave_address; // slave address we are controlling
  virtual bool get_power(bool value __unused) {
    return true;
  }
  virtual bool get_select(bool value) {
    return !value; // Valves are wired up such that select being off means the valve is open
  }
public:
  I2C(uint8_t slave_address):
    slave_address{slave_address} {}

  void set(bool value) {
    Wire.beginTransmission(slave_address);
    // Relay boards have two relays. One turns on power, and the other selects which direction to apply power to.
    // LSB is power, next bit is select.
    bool power = get_power(value);
    bool select = get_select(value);
    bool healthy = true;
    healthy &= Wire.write((select << 1) | power) == 1; // returns the number of bytes written, should be 1
    healthy &= Wire.endTransmission() == 0; // returns non-zero value if there was an error
    healthy &= !Wire.getWireTimeoutFlag(); // make sure timeout flag is not set
    if (!healthy) {
      errors::push(slave_address, ErrorCode::I2CWriteError);
    }
    Wire.clearWireTimeoutFlag(); // if the flag was set, clear it for next time
  }

  ActuatorPosition::ActuatorPosition get_state() {
    // Cast to uint8_t to avoid warning about ambiguous overload
    uint8_t received = Wire.requestFrom(slave_address, static_cast<uint8_t>(1)); // returns number of bytes receieved
    if (received != 1) {
      errors::push(slave_address, ErrorCode::I2CReadError);
      return ActuatorPosition::error;
    }
    uint8_t lims = Wire.read(); // first byte returned is the limit switch values
    if (lims == 0) return ActuatorPosition::unknown;
    if (lims == 1) return ActuatorPosition::open;
    if (lims == 2) return ActuatorPosition::closed;
    return ActuatorPosition::error;
  }

  uint16_t get_current_ma(uint8_t channel) {
    if (channel > 1) { // Index must be 0 or 1 for primary or secondary
      return SENSOR_ERR_VAL;
    }
    // Cast to uint8_t to avoid warning about ambiguous overload
    uint8_t received = Wire.requestFrom(slave_address, static_cast<uint8_t>(5)); // returns number of bytes received
    if (received != 5) {
      errors::push(slave_address, ErrorCode::I2CReadError);
      return SENSOR_ERR_VAL;
    }
    Wire.read(); // limit switch values, ignore
    if (channel == 1) { // if we want the secondary current
      Wire.read(); Wire.read(); // ignore the first (primary) current value (2 bytes)
    }
    uint16_t adcl = Wire.read(); // get the 16-bit current reading
    uint16_t adch = Wire.read();
    return ((adch << 8) | adcl) * 4; // adc / 1024 (10bit) * 4096mV (vref) / 10mohm / 100 adc scaler * 1000 mV/V
  }
};

class Ignition: public I2C {
  virtual bool get_power(bool value) override {
    return value;
  }
  virtual bool get_select(bool value) override {
    return value;
  }
public:
  Ignition(uint8_t slave_address): I2C(slave_address) {}
};

class Heater {
  uint8_t slave_address; // slave address we are controlling
  bool power_set;
  uint8_t reg_set;

public:

  Heater(uint8_t slave_address):
	  slave_address{slave_address},power_set{0},reg_set{0} {}

  void set(bool value) {
	power_set = value;
    Wire.beginTransmission(slave_address);
    // Relay boards have two relays. One turns on power, and the other selects which direction to apply power to.
    // LSB is power, next bit is select.
    bool healthy = true;
    healthy &= Wire.write(power_set | (reg_set << 1)) == 1; // returns the number of bytes written, should be 1
    healthy &= Wire.endTransmission() == 0; // returns non-zero value if there was an error
    healthy &= !Wire.getWireTimeoutFlag(); // make sure timeout flag is not set
    if (!healthy) {
      errors::push(slave_address, ErrorCode::I2CWriteError);
    }
    Wire.clearWireTimeoutFlag(); // if the flag was set, clear it for next time
  }

	void select_reg(uint8_t reg){
		reg_set = reg;
		Wire.beginTransmission(slave_address);
		// Relay boards have two relays. One turns on power, and the other selects which direction to apply power to.
		// LSB is power, next bit is select.
		bool healthy = true;
		healthy &= Wire.write(power_set | (reg_set << 1)) == 1; // returns the number of bytes written, should be 1
		healthy &= Wire.endTransmission() == 0; // returns non-zero value if there was an error
		healthy &= !Wire.getWireTimeoutFlag(); // make sure timeout flag is not set
		if (!healthy) {
			errors::push(slave_address, ErrorCode::I2CWriteError);
		}
		Wire.clearWireTimeoutFlag(); // if the flag was set, clear it for next time
	}

  uint16_t get_thermistor() {
	  select_reg(0);
    // Cast to uint8_t to avoid warning about ambiguous overload
	  uint8_t received = Wire.requestFrom(slave_address, static_cast<uint8_t>(2)); // returns number of bytes received
	  if (received != 2) {
			errors::push(slave_address, ErrorCode::I2CReadError);
			return SENSOR_ERR_VAL;
		}
		uint16_t adcl = Wire.read();
		uint16_t adch = Wire.read();
		return ((adch << 8) | adcl); // Return raw ADC values
	}
	
  uint16_t get_current_ma() {
	  select_reg(1);
    // Cast to uint8_t to avoid warning about ambiguous overload
	  uint8_t received = Wire.requestFrom(slave_address, static_cast<uint8_t>(2)); // returns number of bytes received
	  if (received != 2) {
			errors::push(slave_address, ErrorCode::I2CReadError);
			return SENSOR_ERR_VAL;
		}
	  uint16_t adcl = Wire.read();
	  uint16_t adch = Wire.read();
	  return ((adch << 8) | adcl) * 40; // adc / 1024 (10bit) * 4096mV (vref) / 1mohm / 100 adc scaler * 1000 mV/V
  }

	uint16_t get_batt_voltage() {
	  select_reg(2);
    // Cast to uint8_t to avoid warning about ambiguous overload
	  uint8_t received = Wire.requestFrom(slave_address, static_cast<uint8_t>(2)); // returns number of bytes received
	  if (received != 2) {
			errors::push(slave_address, ErrorCode::I2CReadError);
			return SENSOR_ERR_VAL;
		}
	  uint16_t adcl = Wire.read();
	  uint16_t adch = Wire.read();
	  return ((adch << 8) | adcl) * 28; // adc / 1024 (10bit) * 4096mV (vref) * 7.04
  }

	uint16_t get_kelvin_low_voltage() {
	  select_reg(3);
    // Cast to uint8_t to avoid warning about ambiguous overload
	  uint8_t received = Wire.requestFrom(slave_address, static_cast<uint8_t>(2)); // returns number of bytes received
	  if (received != 2) {
			errors::push(slave_address, ErrorCode::I2CReadError);
			return SENSOR_ERR_VAL;
		}
	  uint16_t adcl = Wire.read();
	  uint16_t adch = Wire.read();
	  return ((adch << 8) | adcl) * 28; // adc / 1024 (10bit) * 4096mV (vref) * 7.04
  }
	
	uint16_t get_kelvin_high_voltage() {
	  select_reg(4);
    // Cast to uint8_t to avoid warning about ambiguous overload
	  uint8_t received = Wire.requestFrom(slave_address, static_cast<uint8_t>(2)); // returns number of bytes received
	  if (received != 2) {
			errors::push(slave_address, ErrorCode::I2CReadError);
			return SENSOR_ERR_VAL;
		}
	  uint16_t adcl = Wire.read();
	  uint16_t adch = Wire.read();
	  return ((adch << 8) | adcl) * 28; // adc / 1024 (10bit) * 4096mV (vref) * 7.04
  }
};
	
} // namespace actuator

#endif
