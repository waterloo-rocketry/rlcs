#include "LCD.h"
#include <LiquidCrystal.h>
#include <Arduino.h>
#include "client_pin_defines.h"

/* We have a 4x20 LCD to work with, we need to display the following data
 *
 *x fill tank pressure -not for liquid tests
 *x line pressure -not for liquid tests
 *x rocket mass -not for liquid tests
 *x run tank pressure -not for liquid tests
 *  battery voltages:
 *    RLCS client
 *    RLCS tower
 *    RocketCAN bus battery
 *    RocketCAN vent battery
 *  valve positions:
 *x   Fill
 *x   Line Vent -not for liquid tests
 *x   Tank Vent
 *X   Ox Pressurant
 *X   Fuel Pressurant
 *X   Ox Injector
 *x   Fuel Injector
 *x   Fill arm (kind of a valve) -not for liquid tests
 *x ignition currents (2)
 *
 *  Here's how we're going to lay all that out. The top row of the LCD
 *  will always contain the following text
 *  RF:___ RV:___ OP:___
 *
 *  These are the positions of (remote fill, remote vent, and Ox Pressurant)
 *
 *  The next line will always display the following valves:
 *  OI:___ FI:___ FP:___
 *  (those are Ox Injector, Fuel Injector, and
 *  Fuel Pressurant)
 *
 *  The next line is as follows
 *  IP:___ IS:___  BAT:
 *  Those are, in order, Primary Ignition Current, Secondary Ignition Current, and Tower Actuators Battery
 *
 *  The fourth line is as follows
 *  TA:___ TM:___ CB:___ 
 *  those values are all in 100s of millivolts, the acronyms stand for "tower actuators",
 *  "tower main", and "Client Battery".
 *
 */

static LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4,
                         PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
void lcd_init()
{
    lcd.begin(20, 4);
    lcd.setCursor(2, 1);
    lcd.print("WAITING FOR DATA");
}

static void display_valves_line(unsigned char fill_open, unsigned char fill_closed, 
                                unsigned char vent_open, unsigned char vent_closed,
                                unsigned char ox_pres_open, unsigned char ox_pres_closed);
static void display_valves_line2(unsigned char ox_injector_open, unsigned char ox_injector_closed, 
                                 unsigned char fuel_injector_open, unsigned char fuel_injector_closed,
                                 unsigned char fuel_pres_open, unsigned char fuel_pres_closed);
static void display_current_line(uint16_t pri_current, uint16_t sec_current);
static void display_battery_line(uint16_t tower_actuator_batt, uint16_t tower_main_batt, uint16_t client_batt);

// not using these
static void display_ground_batt_line(uint16_t client_batt, uint16_t tower_batt);
static void display_rocket_batt_line(uint16_t flight_bus_batt,
                                     uint16_t flight_vent_batt);
static void marquee_current_error();
static void marquee_next_error();

//static variables we keep track of in order to show marquee error msgs
static bool currently_marqueeing_error = false;
static char buffer_to_marquee[10][21];
static uint8_t marquee_buffer_index = 0;
static uint8_t num_errors_queued = 0;
static long time_last_error_displayed = 0;

void lcd_update(daq_holder_t *input)
{
    //two of the LCD lines used to switch back and forth between two lines
    //every two seconds. We used this variable to keep track of which
    //line should be displayed at the current time
    static bool display_first_line = false; // changed to false per my evil scemes mentioned below.
    //timer to keep track of when to switch between lines
    //static long time_last_line_switch = 0;
    //if we need to switch lines, do so

    // I have hijacked these lines so I can always see what the state of the valve connected to disconnect is. Bwa ha ha ha ha!
     /*
    if (millis() - time_last_line_switch > 2000) {
        display_first_line = !display_first_line;
        time_last_line_switch = millis();
    }
    */

    //display the top line
    display_valves_line(input->valve_1_lsw_open, input->valve_1_lsw_closed,
                        input->valve_2_lsw_open, input->valve_2_lsw_closed,
                        input->valve_3_lsw_open, input->valve_3_lsw_closed);

    //display the second line
    display_valves_line2(input->valve_4_lsw_open, input->valve_4_lsw_closed,
                         input->injector_valve_lsw_open, input->injector_valve_lsw_closed,
                         input->valve_3_lsw_open, input->valve_3_lsw_closed);

    //display the third line
    display_current_line(input->ign_pri_current, input->ign_sec_current);

    //display the fourth line
    display_battery_line(input->rlcs_actuator_batt_mv, input->rlcs_main_batt_mv, client_battery);

    // the code below was used with the old alternating system
    
    /*if (display_first_line) {
        //the one with the rocket mass in it
        display_mass_line(input->injector_valve_state, input->pressure1,
                          input->rocket_mass);
    } else {
        //the one with the remote disconnect status
        display_disconnect_line(input->injector_valve_state, input->pressure2,
                                input->linac_lsw_extend,
                                input->linac_lsw_retract);
    }

    //this one's fun
    if (currently_marqueeing_error) {
        marquee_current_error();
    } else if (num_errors_queued != 0 &&
               millis() - time_last_error_displayed > 9000) {
        marquee_next_error();
    } else {
        if (display_first_line) {
            display_ground_batt_line(input->rlcs_actuator_batt_mv, input->rlcs_main_batt_mv);
        } else {
            display_rocket_batt_line(input->bus_batt_mv, input->vent_batt_mv);
        }
    }
    */
}

void display_new_error(const char *error)
{
    if (num_errors_queued == 10)
        return;
    uint8_t write_index = (marquee_buffer_index + num_errors_queued ) % 10;
    strncpy(buffer_to_marquee[write_index], error, sizeof(buffer_to_marquee[0]));
    Serial.println(error);
    num_errors_queued++;
}

static void display_valves_line(unsigned char valve_1_open, unsigned char valve_1_closed, 
                                unsigned char valve_2_open, unsigned char valve_2_closed,
                                unsigned char valve_3_open, unsigned char valve_3_closed)
{
    lcd.setCursor(0, 0);
    char line[21];
        snprintf(line, 21, "V1:%s V2:%s V3:%s",
             (valve_1_open && !valve_1_closed) ? "OPN" :
             (!valve_1_open && valve_1_closed) ? "CLS" : "UNK",
             (valve_2_open && !valve_2_closed) ? "OPN" :
             (!valve_2_open && valve_2_closed) ? "CLS" : "UNK",
             (valve_3_open && !valve_3_closed) ? "OPN" :
             (!valve_3_open && valve_3_closed) ? "CLS" : "UNK");
    lcd.print(line);
}

static void display_valves_line2(unsigned char valve_4_open, unsigned char valve_4_closed, 
                                 unsigned char injector_open, unsigned char injector_closed,
                                 unsigned char fuel_pres_open, unsigned char fuel_pres_closed)
{
    lcd.setCursor(0, 1);
    char line[21];
    snprintf(line, 21, "V4:%s IJ:%s FP:%s",
             (valve_4_open && !valve_4_closed) ? "OPN" :
             (!valve_4_open && valve_4_closed) ? "CLS" : "UNK",
             (injector_open && !injector_closed) ? "OPN" :
             (!injector_open && injector_closed) ? "CLS" : "UNK",
             (fuel_pres_open && !fuel_pres_closed) ? "OPN" :
             (!fuel_pres_open && fuel_pres_closed) ? "CLS" : "UNK");
    lcd.print(line);
}

static void display_current_line(uint16_t pri_current, uint16_t sec_current)
{
    lcd.setCursor(0, 2);
    if (pri_current > 999)
        pri_current = 999;
    if (sec_current > 999)
        sec_current = 999;
    char line[21];
    snprintf(line, 21, "IP:%03u IS:%03u  BAT:", pri_current, sec_current);
    lcd.print(line);
}

static void display_battery_line(uint16_t tower_actuator_batt, uint16_t tower_main_batt, uint16_t client_batt)
{
     lcd.setCursor(0, 3);
    tower_actuator_batt = tower_actuator_batt/100; //the function acepts mV, but we will only display 100s of mV
    tower_main_batt = tower_main_batt/100;
    client_batt = client_batt/100;
    if (tower_actuator_batt > 999)
        tower_actuator_batt = 999;
    if (tower_main_batt > 999)
        tower_main_batt = 999;
    if (client_batt > 999)
        client_batt = 999;
    char line[21];
    snprintf(line, 21, "TA:%03u TM:%03u CB:%03u", tower_actuator_batt, tower_main_batt, client_batt);
    lcd.print(line);
}

static void display_disconnect_line(valve_state_t injector_state,
                                    uint16_t fill_line_pressure, unsigned char linac_extend,
                                    unsigned char linac_retract)
{
    lcd.setCursor(0, 2);
    if (fill_line_pressure > 999)
        fill_line_pressure = 999;
    char line[21];
    snprintf(line, 21, "IV:%s PL:%03u RD:%s",
             (injector_state == DAQ_VALVE_OPEN) ? "OPN" :
             (injector_state == DAQ_VALVE_CLOSED) ? "CLS" :
             (injector_state == DAQ_VALVE_UNK) ? "UNK" : "ILL",
             fill_line_pressure,
             (linac_extend && !linac_retract) ? "OPN" : //Being used as 3rd valve, not disconnect TODO: when adding code for hybrid/liquid loadout, add a macro here
             (!linac_extend && linac_retract) ? "CLS" : "UNK");
    lcd.print(line);
}

//Not using any of these

static void display_ground_batt_line(uint16_t tower_act, uint16_t tower_main)
{
    lcd.setCursor(0, 3);
    char line[21];
    snprintf(line, 21, "BAT TA:%05uTM:%05u", tower_act, tower_main);
    lcd.print(line);
}

static void display_rocket_batt_line(uint16_t flight_bus_batt,
                                     uint16_t flight_vent_batt)
{
    lcd.setCursor(0, 3);
    char line[21];
    snprintf(line, 21, "BAT FB:%05uFV:%05u", flight_bus_batt, flight_vent_batt);
    lcd.print(line);
}

static void marquee_current_error()
{
    //exit conditions
    if (millis() - time_last_error_displayed > 5000) {
        currently_marqueeing_error = false;
        num_errors_queued--;
        marquee_buffer_index++;
        if (marquee_buffer_index >= 10)
            marquee_buffer_index = 0;
    }

    char line[21];
    snprintf(line, 21, "%s", buffer_to_marquee[marquee_buffer_index]);
    lcd.setCursor(0, 3);
    lcd.print(line);
}

static void marquee_next_error()
{
    time_last_error_displayed = millis();
    lcd.setCursor(0, 3);
    lcd.print("                    ");
    currently_marqueeing_error = true;
}
