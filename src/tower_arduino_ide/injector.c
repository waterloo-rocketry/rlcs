#include "injector.h"
#include "Arduino.h"
#include "tower_pin_defines.h"
#include "tower_globals.h"

// don't want to delete, but we aren't using liquid any time soon
/*
#define FUEL_OPEN_DELAY 0 //time between open comand, and beginning of opening, in ms
#define OX_OPEN_DELAY 800 //time between open comand, and beginning of opening, in ms
#define START_MOVE_LENGTH 400 // for fuel

unsigned int fuel_injector_powered_time = 0; //time the valve is on while moving
unsigned int fuel_injector_unpowered_time = 50; //time the valve is off while moving
static unsigned long time_of_fuel_injector_open = 0;
static unsigned long time_of_fuel_injector_powerdown = 0;
static unsigned long time_of_fuel_injector_powerup = 0;
static unsigned long time_end_fuel_injector_start_move = 0;
static unsigned long fuel_injector_powerup_cycle = 0;

static unsigned long time_of_ox_injector_open = 0;


static enum{
  OFF,
  ARMED,
  START_MOVE,
  POWERED,
  UNPOWERED
} fuel_injector_state = OFF;

static enum{
  OX_OFF,
  OX_ARMED
} ox_injector_state = OX_OFF;

void fuel_injector_refresh(){
  if(get_global_current_daq()->fuel_injector_valve_lsw_open){
    fuel_injector_state = OFF;
  }
  switch(fuel_injector_state){
    case OFF:
      //do nothing
      break;

    case ARMED:
      if(millis() >= time_of_fuel_injector_open){
        digitalWrite(PIN_FUEL_INJECTOR_VALVE_POWER, HIGH);
        digitalWrite(PIN_FUEL_INJECTOR_VALVE_SELECT, LOW);
        fuel_injector_state = START_MOVE;
        time_end_fuel_injector_start_move = millis() + START_MOVE_LENGTH;
      }
      break;
    case START_MOVE:
      if(millis() >= time_end_fuel_injector_start_move){
        digitalWrite(PIN_FUEL_INJECTOR_VALVE_POWER, LOW);
        //digitalWrite(PIN_FUEL_INJECTOR_VALVE_POWER, HIGH);
        
        fuel_injector_state = POWERED;
        time_of_fuel_injector_powerdown = millis() + fuel_injector_powered_time;
      }
      break;
    case POWERED:
      if(millis() >= time_of_fuel_injector_powerdown){  // time to turn off
        fuel_injector_powerup_cycle++;
        digitalWrite(PIN_FUEL_INJECTOR_VALVE_POWER, LOW);
        //digitalWrite(PIN_FUEL_INJECTOR_VALVE_SELECT, HIGH);
        time_of_fuel_injector_powerup = millis() + fuel_injector_unpowered_time;  //set when we should turn the motor back on
        fuel_injector_state = UNPOWERED;        
      }
      break;
    case UNPOWERED:
      if(millis() >= time_of_fuel_injector_powerup){  // time to turn back on
        digitalWrite(PIN_FUEL_INJECTOR_VALVE_POWER, HIGH);
        //digitalWrite(PIN_FUEL_INJECTOR_VALVE_SELECT, LOW);
        fuel_injector_powered_time++;
        time_of_fuel_injector_powerdown = millis() + fuel_injector_powered_time;  //set when we should turn the motor off
        fuel_injector_state = POWERED;        
      }
      break;

     default:
      break;
    
  }
}

void ox_injector_refresh(){
  switch(ox_injector_state){
    case OX_OFF:
      //do nothing
      break;

    case OX_ARMED:
      if(millis() >= time_of_ox_injector_open){
        digitalWrite(PIN_OX_INJECTOR_VALVE_POWER, HIGH);
        digitalWrite(PIN_OX_INJECTOR_VALVE_SELECT, LOW);
        ox_injector_state = OX_OFF;
      }
      break;

    default:
      break;
  }
}

void open_fuel_injector(){
  if(fuel_injector_state == OFF){
    fuel_injector_state = ARMED;
    time_of_fuel_injector_open = millis() + FUEL_OPEN_DELAY;
  }
}

void close_fuel_injector(){
      digitalWrite((uint8_t) PIN_FUEL_INJECTOR_VALVE_POWER, HIGH);
      digitalWrite((uint8_t) PIN_FUEL_INJECTOR_VALVE_SELECT, HIGH);
      fuel_injector_state = OFF;
      fuel_injector_powered_time = 0;
      fuel_injector_powerup_cycle = 0;

}

void open_ox_injector(){
  if(ox_injector_state == OX_OFF){
    ox_injector_state = OX_ARMED;
    time_of_ox_injector_open = millis() + OX_OPEN_DELAY;
  }
}

void close_ox_injector(){
      digitalWrite((uint8_t) PIN_OX_INJECTOR_VALVE_POWER, HIGH);
      digitalWrite((uint8_t) PIN_OX_INJECTOR_VALVE_SELECT, HIGH);
      ox_injector_state = OX_OFF;

}

*/
