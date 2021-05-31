#ifndef LCD_H
#define LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "shared_types.h"
#include "client_globals.h"

void lcd_init(); //called in setup
void lcd_update(daq_holder_t *input_dat); //called every time in loop()
void display_new_error(const char *error);
#ifdef __cplusplus
}
#endif

#endif
