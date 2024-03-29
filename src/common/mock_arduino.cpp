#include "mock_arduino.hpp"

#ifndef ARDUINO

extern void setup();
extern void loop();

MockSerial Serial;
MockSerial Serial2;
MockSerial Serial3;
TwoWire Wire;

unsigned long millis() {
  static unsigned long n;
  // increment the time every call to millis() so stuff happens
  n += 80;
  return n;
}
uint16_t analogRead(uint8_t pin __unused) {
  return 512;
}
bool digitalRead(uint8_t pin __unused) {
  return false;
}
void digitalWrite(uint8_t pin, bool value) {
  std::cout << "Writing " << (int)pin << " " << value << std::endl;
}
void pinMode(uint8_t, bool) {}

int main() {
  setup();
  while (true) {
    loop();
  }
}

#endif
