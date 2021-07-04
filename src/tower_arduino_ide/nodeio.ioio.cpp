#include "nodeio.ioio.h"
#include "Arduino.h"
#include "sd_handler.h"
#include "radio_comms.h"
#define NODE_GROUND

//copied from RocketCAN
enum BOARD_STATUS {
    E_NOMINAL = 0,
    E_BUS_OVER_CURRENT,
    E_BUS_UNDER_VOLTAGE,
    E_BUS_OVER_VOLTAGE,
    E_BATT_UNDER_VOLTAGE,
    E_BATT_OVER_VOLTAGE,
    E_BOARD_FEARED_DEAD,
    E_NO_CAN_TRAFFIC,
    E_MISSING_CRITICAL_BOARD,
    E_RADIO_SIGNAL_LOST,
    E_VALVE_STATE,
    E_CANNOT_INIT_DACS,
    E_VENT_POT_RANGE,
    E_LOGGING,
    E_GPS,
    E_SENSOR,
    E_ILLEGAL_CAN_MSG,
    E_SEGFAULT,
    E_UNHANDLED_INTERRUPT,
    E_CODING_FUCKUP,
    E_BATT_OVER_CURRENT,
};
typedef struct {
    uint8_t board_id;
    enum BOARD_STATUS err_type;
    uint8_t byte4;
    uint8_t byte5;
    uint8_t byte6;
    uint8_t byte7;
} error_t;


//packets that can be sent over the radio
//their meanings are better understood if
//read the giant switch statement in the
//refresh function
#define NIO_STATE_COMMAND_HEADER '{'
#define NIO_STATE_REQUEST_HEADER '}'
#define NIO_ERROR_COMMAND_HEADER '!'

#define STATE_COMMAND_LEN 11
#define ERROR_COMMAND_LENGTH 8
#define SERIALIZED_OUTPUT_LEN 9

//static functions copy/pasted from cansw_radio
static char binary_to_base64(uint8_t binary);
static uint8_t base64_to_binary(char base64);
static char checksum(char *cmd);
static bool deserialize_state(system_state *state, const char *str);
static bool deserialize_error(error_t *state, const char *str);
static bool create_state_command(char *cmd, const system_state *state);


static void send_desired_system_state(void);


//header for updating sensor data
#include "tower_globals.h"
#include "sd_handler.h"

//it currently uses the Serial3 interface
#define RADIO_UART Serial3

//stores bytes that come in from the radio
char message_received_buffer[50];
int message_buffer_index = 0;

//the last received system state that we got from the rocket
static system_state last_received_rocket_state = {};

//the state that we want the rocket to be in
static system_state desired_rocket_state;

//helper functions for setting those states
//called from tower fsm loop
void nio_set_vent_desired(nio_actuator_state s)
{
    desired_rocket_state.vent_valve_state = s;
}
void nio_set_inj_desired(nio_actuator_state s)
{
    desired_rocket_state.injector_valve_state = s;
}

void nio_power_bus(void)
{
    desired_rocket_state.bus_is_powered = true;
}

void nio_depower_bus(void)
{
    desired_rocket_state.bus_is_powered = false;
}

//init function. Called at startup
void nio_init(void)
{
    while (!RADIO_UART);
    //make sure to set the radio module to 9600 baud.
    RADIO_UART.begin(9600);

    //set default values to show on the LCD
    daq_holder_t *daq = get_global_current_daq();
    daq->injector_valve_state = DAQ_VALVE_UNK;
    daq->rocketvent_valve_state = DAQ_VALVE_UNK;
    daq->pressure3 = 0; //set flight tank pressure to 0 until we know

    //by default, make the bus unpowered, the injector valve unknown
    //(depowered), and the vent valve open
    desired_rocket_state.bus_is_powered = false;
    desired_rocket_state.injector_valve_state = NOTHING;
    desired_rocket_state.vent_valve_state = VALVE_OPEN;
}

//different states the FSM can be in
enum nio_states {
    STATE_NONE,
    STATE_STATE_RECEIVE,
    STATE_ERROR_RECEIVE
};

nio_states current_fsm_state = STATE_NONE;
static long time_last_received_rocket_state = 0;
void nio_refresh()
{
//implement a fsm. This is largely based off the one used by RLCS.
//This isn't done atm (only works for vent slave). The explanatory
//comments will be updated when this function actually does stuff
    while (RADIO_UART.available()) {
        char x = RADIO_UART.read();
        switch (x) {
            case NIO_STATE_COMMAND_HEADER:
                current_fsm_state = STATE_STATE_RECEIVE;
                message_buffer_index = 0;
                break;
            case NIO_ERROR_COMMAND_HEADER:
                current_fsm_state = STATE_ERROR_RECEIVE;
                message_buffer_index = 1;
                message_received_buffer[0] = x;
                break;
            default:
                //check if the character is a base 64 one
                uint8_t temp = base64_to_binary(x);
                if (temp == 255) {
                    //not a base 64, do nothing
                    continue;
                } else {
                    //if there's room in the buffer, push this in
                    if (message_buffer_index < sizeof(message_received_buffer)) {
                        message_received_buffer[message_buffer_index] = x;
                        message_buffer_index++;
                    }
                    //check if we've received a full message
                    if (current_fsm_state == STATE_STATE_RECEIVE) {
                        if (message_buffer_index == (STATE_COMMAND_LEN - 2)) {
                            //compute the checksum of bytes 0 through STATE_COMMAND_LEN - 2
                            uint8_t expected_checksum = message_received_buffer[STATE_COMMAND_LEN - 3];
                            message_received_buffer[STATE_COMMAND_LEN - 3] = '\0';
                            uint8_t sum = checksum(message_received_buffer);
                            if (sum == expected_checksum) {
                                // log that the message came in
                                rlcslog("Message from rocketCAN");
                                rlcslog(message_received_buffer);
                                deserialize_state(&last_received_rocket_state, message_received_buffer);
                                tower_handle_rocketcan_update(&last_received_rocket_state);
                                time_last_received_rocket_state = millis();
                            }
                            current_fsm_state = STATE_NONE;
                        }
                    } else if (current_fsm_state == STATE_ERROR_RECEIVE) {
                        if (message_buffer_index == 9) {
                            /*
                             * Don't send error messages to the client. For
                             * reasons I don't understand and that I don't have
                             * time to debug, errors that shouldn't be there are
                             * popping up. The system works fine, and none of the
                             * errors that we really care about (battery low,
                             * injector valve in a weird state) can't be determined
                             * from other info on the client side LCD (battery
                             * voltages and valve positions are displayed
                             *
                            uint8_t expected_sum = message_received_buffer[8];
                            message_received_buffer[8] = '\0';
                            uint8_t sum = checksum(message_received_buffer);
                            error_t err;
                            if (sum == expected_sum && deserialize_error(&err, message_received_buffer + 1)) {
                                static uint8_t last_board_id, last_err_type, lb4, lb5, lb6, lb7;
                                if (last_board_id != err.board_id ||
                                    last_err_type != err.err_type ||
                                    lb4 != err.byte4 ||
                                    lb4 != err.byte4 ||
                                    lb4 != err.byte4 ||
                                    lb4 != err.byte4) {
                                    char buffer[80];
                                    sprintf(buffer, "%03x %02x %02x %02x %02x %02x", err.board_id, err.err_type, err.byte4, err.byte5, err.byte6, err.byte7);
                                    Serial.println(buffer);
                                    sprintf(buffer, "%03x%02x%02x%02x%02x%02x", err.board_id, err.err_type, err.byte4, err.byte5, err.byte6, err.byte7);
                                    write_to_xbee('!');
                                    char *i = buffer;
                                    while(*i) {
                                        write_to_xbee(*i++);
                                    }
                                    last_board_id = err.board_id;
                                    last_err_type = err.err_type;
                                    lb4 = err.byte4;
                                    lb5 = err.byte5;
                                    lb6 = err.byte6;
                                    lb7 = err.byte7;
                                }
                            }
                            */
                            current_fsm_state = STATE_NONE;
                        }
                    }
                }
                break;
        }
    }

    //if the desired state and last received state don't match, send a new command
    if (desired_rocket_state.injector_valve_state !=
        last_received_rocket_state.injector_valve_state ||
        desired_rocket_state.vent_valve_state !=
        last_received_rocket_state.vent_valve_state ||
        millis() - time_last_received_rocket_state > 5000) {
        send_desired_system_state();
    }

    //every three seconds we need to ask for what their current system state is
    static long last_time_requested_rocketcan_state = 0;
    if (millis() - last_time_requested_rocketcan_state > 3000) {
        last_time_requested_rocketcan_state = millis();
        RADIO_UART.write(NIO_STATE_REQUEST_HEADER);
    }
}

static long last_time_sent_system_state = 0;
static void send_desired_system_state(void)
{
    //rate limit
    if (millis() - last_time_sent_system_state < 500) {
        return;
    }
    //create message to send
    char buffer[30];
    if (create_state_command(buffer, &desired_rocket_state)) {
        //send message. create_state_command included null terminator.
        RADIO_UART.print(buffer);
    }
    last_time_sent_system_state  = millis();
}

/*
 * Begin functions copy/pasted from cansw_radio
 *
 * yes this is a terrible way of sharing code, but I'm not sure what else I
 * can hack together very quickly
 */
static char binary_to_base64(uint8_t binary)
{
    if (binary <= 25)
        return binary + 'A';
    if (binary <= 51)
        return (binary - 26) + 'a';
    if (binary <= 61)
        return (binary - 52) + '0';
    if (binary == 62)
        return '&';
    if (binary == 63)
        return '/';
    // ***TODO***: return an error in all other cases
    return 0;
}

static uint8_t base64_to_binary(char base64)
{
    if ('A' <= base64 && base64 <= 'Z')
        return base64 - 'A';
    if ('a' <= base64 && base64 <= 'z')
        return base64 - 'a' + 26;
    if ('0' <= base64 && base64 <= '9')
        return base64 - '0' + 52;
    if (base64 == '&')
        return 62;
    if (base64 == '/')
        return 63;
    // ***TODO***: return an error in all other cases
    return 255;
}

static bool serialize_state(const system_state *state, char *str)
{
    if (state == NULL)
        return false;
    if (str == NULL)
        return false;

    uint8_t raw = 0;
    // Bits 5-2 represent the number of boards connected
    raw |= (state->num_boards_connected & 0b00001111) << 2;
    // Bits 1-0 represent the injector valve state
    raw |= (state->injector_valve_state & 0x3);
    str[0] = binary_to_base64(raw);

    raw = 0;
    // Bits 5-4 represent the vent valve state
    raw |= ((state->vent_valve_state & 0x3) << 4);
    // Bits 3-0 are the top bits 9-6 of the tank pressure
    raw |= ((state->tank_pressure >> 6) & 0b1111);
    str[1] = binary_to_base64(raw);

    //use all the bits of this next one to hold bits 5-0 of tank pressure
    raw = (state->tank_pressure & 0b00111111);
    str[2] = binary_to_base64(raw);

    raw = 0;
    // Bit 5 represents whether the bus is should be powered
    if (state->bus_is_powered)
        raw |= 0b00100000;
    // Bit 4 represents whether errors have been detected
    if (state->any_errors_detected)
        raw |= 0b00010000;
    // Bits 3-0 are the top bits 13-10 of the bus battery voltage
    raw |= (state->bus_battery_voltage_mv >> 10) & 0xf;
    str[3] = binary_to_base64(raw);

    raw = 0;
    // Bits 5-0 hold bits 9-4 of the bus battery voltage
    raw = (state->bus_battery_voltage_mv >> 4) & 0x3f ;
    str[4] = binary_to_base64(raw);

    raw = 0;
    // Bits 5-2 hold bits 3-0 of the bus battery voltage
    raw = (state->bus_battery_voltage_mv & 0xf) << 2;
    // Bits 1-0 hold bits 13-12 of the vent battery voltage
    raw |= (state->vent_battery_voltage_mv >> 12) & 0x3;
    str[5] = binary_to_base64(raw);

    raw = 0;
    // Bits 5-0 hold bits 11-6 of the vent battery voltage
    raw = (state->vent_battery_voltage_mv >> 6) & 0x3f;
    str[6] = binary_to_base64(raw);

    raw = 0;
    // Bits 5-0 hold bits 5-0 of the vent battery voltage
    raw = (state->vent_battery_voltage_mv & 0x3f);
    str[7] = binary_to_base64(raw);

    str[8] = '\0';
    return true;
}

static bool deserialize_state(system_state *state, const char *str)
{
    if (state == NULL)
        return false;
    if (str == NULL)
        return false;

    uint8_t raw = base64_to_binary(str[0]);

    // Bits 5-2 represent the number of boards connected
    state->num_boards_connected = (raw & 0b00111100) >> 2;
    // Bits 1-0 represent the injector valve state
    state->injector_valve_state = (nio_actuator_state)(raw & 0x3);

    raw = base64_to_binary(str[1]);

    // Bits 5-4 represent the vent valve state
    state->vent_valve_state = (nio_actuator_state)((raw & 0x30) >> 4);
    // Bit 3-0 are bits 9-6 of tank pressure
    state->tank_pressure = ((uint16_t) (raw & 0xf)) << 6;

    raw = base64_to_binary(str[2]);
    state->tank_pressure |= (raw & 0x3f);

    raw = base64_to_binary(str[3]);
    // Bit 5 represents whether the bus should be powered
    state->bus_is_powered = raw & 0x20;
    // Bit 4 represents whether any errors are active
    state->any_errors_detected = raw & 0x10;
    // Bits 3-0 represent the top 13-10 bits of the bus battery voltage
    state->bus_battery_voltage_mv = (uint16_t) (raw & 0xf) << 10;

    raw = base64_to_binary(str[4]);
    // Bits 5-0 represent bits 9-4 of the bus battery voltage
    state->bus_battery_voltage_mv |= (raw & 0x3f) << 4;

    raw = base64_to_binary(str[5]);
    // Bits 5-2 represent bits 3-0 of the bus battery voltage
    state->bus_battery_voltage_mv |= (raw & 0x3c) >> 2;
    // Bits 1-0 represent bits 13-12 of the vent battery voltage
    state->vent_battery_voltage_mv = (uint16_t) (raw & 0x3) << 12;

    raw = base64_to_binary(str[6]);
    // Bits 5-0 represent bits 11-6 of the vent battery voltage
    state->vent_battery_voltage_mv |= (raw & 0x3f) << 6;

    raw = base64_to_binary(str[7]);
    // Bits 5-0 represent bits 5-0 of the vent battery voltage
    state->vent_battery_voltage_mv |= raw & 0x3f;


    return true;
}

static bool deserialize_error(error_t *err, const char *str)
{
    if (err == NULL)
        return false;
    if (str == NULL)
        return false;

    // A temporary variable to store decoded values
    uint8_t temp = 0;

    temp = base64_to_binary(str[0]);
    err->board_id = (temp & 0b00111100) >> 2;
    err->err_type = (temp & 0b00000011) << 4;

    temp = base64_to_binary(str[1]);
    err->err_type |= (temp & 0b00111100) >> 2;
    err->byte4 = (temp & 0b00000011) << 6;

    temp = base64_to_binary(str[2]);
    err->byte4 |= (temp & 0b00111111);

    temp = base64_to_binary(str[3]);
    err->byte5 = (temp & 0b00111111) << 2;

    temp = base64_to_binary(str[4]);
    err->byte5 |= (temp & 0b00110000) >> 4;
    err->byte6 = (temp & 0b00001111) << 4;

    temp = base64_to_binary(str[5]);
    err->byte6 |= (temp & 0b00111100) >> 2;
    err->byte7 = (temp & 0b00000011) << 6;

    temp = base64_to_binary(str[6]);
    err->byte7 |= (temp & 0b00111111);

    return true;
}

static bool compare_system_states(const system_state *s, const system_state *p)
{
    if (s == NULL)
        return false;
    if (p == NULL)
        return false;

    if (s->num_boards_connected != p->num_boards_connected)
        return false;
    if (s->injector_valve_state != p->injector_valve_state)
        return false;
    if (s->vent_valve_state != p->vent_valve_state)
        return false;
    if (s->bus_is_powered != p->bus_is_powered)
        return false;
    if (s->any_errors_detected != p->any_errors_detected)
        return false;

    return true;
}

static bool create_state_command(char *cmd, const system_state *state)
{
    if (cmd == NULL)
        return false;
    if (state == NULL)
        return false;

    char serialized[SERIALIZED_OUTPUT_LEN];
    if (!serialize_state(state, serialized))
        return false;

    // Do not copy the null terminator of the serialized string
    memcpy(cmd + 1, serialized, SERIALIZED_OUTPUT_LEN - 1);

    // Message start indicator
    cmd[0] = NIO_STATE_COMMAND_HEADER;
    // Checksum
    cmd[STATE_COMMAND_LEN - 2] = '\0';
    cmd[STATE_COMMAND_LEN - 2] = checksum(serialized);
    // Null terminator
    cmd[STATE_COMMAND_LEN - 1] = 0;

    return true;
}

static char checksum(char *cmd)
{
    uint8_t total = 0;
    uint8_t idx = 0;
    while (cmd[idx] != 0) {
        uint8_t curr = (uint8_t) cmd[idx];
        uint8_t odd_sum = 0;
        uint8_t even_sum = 0;
        for (uint8_t i = 0; i < 4; ++i) {
            odd_sum += 0b00000010 & curr;
            even_sum += 0b00000001 & curr;
            curr = curr >> 2;
        }
        total += odd_sum + 3 * even_sum;
        ++idx;
    }
    total %= 64;
    return binary_to_base64(total);
}
