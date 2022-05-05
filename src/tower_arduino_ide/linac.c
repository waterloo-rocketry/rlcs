#include "linac.h"
#include "i2c.h"
#include "Arduino.h"

//macros
#define LINAC_POWERED_TIME_MOVING 2000 // who needs duty cycling time anyways
#define LINAC_UNPOWERED_TIME_MOVING 50
//turn on and off this many times while moving
#define LINAC_CYCLES_MOVING 1 // lets do it all in one go
//don't move for 2 seconds after moving
#define LINAC_COOLDOWN_TIME 50 // cooldown time? where we're going we don't need cooldown time

static enum {
    LINAC_UNKNOWN, //what it is at startup
    LINAC_EXTENDED,
    LINAC_RETRACTED
} current_linac_state = LINAC_UNKNOWN;

static enum {
    LINAC_COOLDOWN,
    LINAC_MOVING_POWERED,
    LINAC_MOVING_UNPOWERED,
    LINAC_READY
} current_timer_state = LINAC_READY;

//the time when the next thing has to be done. In LINAC_MOVING, this
//variable holds the time (in milliseconds from startup) that we need to
//stop sending power to the linear actuator
static unsigned long next_state_transition = 0;
static unsigned long cycles_remaining_in_move = 0;

//public functions
void linac_init()
{
    i2c_set_valve_power(I2C_VALVE_3, 0);
    i2c_set_valve_select(I2C_VALVE_3, 0);
}

void linac_refresh()
{
    switch(current_timer_state)
    {
    case LINAC_MOVING_POWERED:
        //state exit conditions
        if(millis() >= next_state_transition){
            //stop sending power to the linear actuator
            i2c_set_valve_power(I2C_VALVE_3, 0);
            //setup next state
            current_timer_state = LINAC_MOVING_UNPOWERED;
            //we become ready after LINAC_COOLDOWN_TIME seconds
            next_state_transition = millis() + LINAC_UNPOWERED_TIME_MOVING;
        }
        break;
    case LINAC_MOVING_UNPOWERED:
        if(millis() >= next_state_transition){
            //decide whether to go to cooldown
            if(--cycles_remaining_in_move == 0){
                //go to cooldown
                current_timer_state = LINAC_COOLDOWN;
                next_state_transition = millis() + LINAC_COOLDOWN_TIME;
            } else {
                //power up again
                i2c_set_valve_power(I2C_VALVE_3, 1);
                //setup time to come back to unpowered mode
                current_timer_state = LINAC_MOVING_POWERED;
                next_state_transition = millis() + LINAC_POWERED_TIME_MOVING;
            }
        }
        break;
    case LINAC_COOLDOWN:
        //state exit conditions
        if(millis() >= next_state_transition){
            //goto ready state
            current_timer_state = LINAC_READY;
            //don't need to change next_state_transition because
            //we only leave ready state when we're told to do something
            //with the actuator
        }
        break;
    case LINAC_READY: //don't do anything
    default:
        break;
    }
}

uint8_t linac_extend()
{
    //if we're not ready to extend it, like, don't do anything
    if(current_timer_state != LINAC_READY)
        return false;
    //if we think that the linear actuator is currently extended, we
    //return true (since it's extended, the caller has what they want)
    if(current_linac_state == LINAC_EXTENDED)
        return true;

    //at this point, we know that we're allowed to extend the linear
    //actuator. So do that. the select pin needs to be written high
    //for this to work right
    i2c_set_valve_select(I2C_VALVE_3, 1);
    i2c_set_valve_power(I2C_VALVE_3, 1);

    //remember to stop moving after a certain amount of time
    current_timer_state = LINAC_MOVING_POWERED;
    cycles_remaining_in_move = LINAC_CYCLES_MOVING;
    next_state_transition = millis() + LINAC_POWERED_TIME_MOVING;
    current_linac_state = LINAC_EXTENDED;
    //it worked. So return true
    return true;
}

//logic largely copied from linac_extend, hence the lack of explanation
//comments
uint8_t linac_retract()
{
    if(current_timer_state != LINAC_READY)
        return false;
    if(current_linac_state == LINAC_RETRACTED)
        return true;

    i2c_set_valve_select(I2C_VALVE_3, 0);
    i2c_set_valve_power(I2C_VALVE_3, 1);

    current_timer_state = LINAC_MOVING_POWERED;
    cycles_remaining_in_move = LINAC_CYCLES_MOVING;
    next_state_transition = millis() + LINAC_POWERED_TIME_MOVING;
    current_linac_state = LINAC_RETRACTED;

    return true;
}
