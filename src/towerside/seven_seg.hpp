#ifndef SEVEN_SEG_H
#define SEVEN_SEG_H

#include "common/config.hpp"

namespace seven_seg {

void setup();
void display(const ActuatorMessage &state);
void tick();

} // namespace seven_seg

#endif
