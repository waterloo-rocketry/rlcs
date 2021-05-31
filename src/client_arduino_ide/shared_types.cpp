#include "shared_types.h"

//returns 1 if they're the same, else returns 0
int actuator_compare(const actuator_state_t* s, const actuator_state_t* q)
{
    return  s->valve_1 == q->valve_1 &&
            s->valve_2 == q->valve_2 &&
            s->valve_3 == q->valve_3 &&
            s->valve_4 == q->valve_4 &&
            s->injector_valve == q->injector_valve &&
            s->ignition_power == q->ignition_power &&
            s->ignition_select == q->ignition_select;
}
