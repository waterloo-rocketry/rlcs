#include "hardware.hpp"
#include "common/mock_arduino.hpp"

namespace Hardware {

// Do any hardware-specific setup that doesn't have a good place elsewhere.
void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(10000);
  Wire.setWireTimeout(1000, true); // 1000uS = 1mS timeout, true = reset the bus in this case.
}

}