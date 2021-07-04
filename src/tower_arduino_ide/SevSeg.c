#include "SevSeg.h"
#include "sd_handler.h"
#include "tower_pin_defines.h"
//#include <avr/io.h>
#include<stdint.h>

//array of segements that must be displayed to display the correct number
static uint8_t digitCodeMap[] = {
  //GFEDCBA  Segments      7-segment map:
  0b00111111, // 0   "0"          AAA
  0b00000110, // 1   "1"         F   B
  0b01011011, // 2   "2"         F   B
  0b01001111, // 3   "3"          GGG
  0b01100110, // 4   "4"         E   C
  0b01101101, // 5   "5"         E   C
  0b01111101, // 6   "6"          DDD  DP
  0b00000111, // 7   "7"
  0b01111111, // 8   "8"
  0b01101111, // 9   "9"
  0b01110111, // A "A"
  0b01111100, // b "b"
  0b00111001, // C "C"
  0b01011110, // d "d"
  0b01111001, // E "E"
  0b01110001 // F "F"
};

static uint8_t * digitCodes[2];

static const unsigned digitPinsIn[] = {
    PIN_SEVENSEG_D1,
    PIN_SEVENSEG_D2
};
static const unsigned segmentPinsIn[] = {
    PIN_SEVENSEG_A,
    PIN_SEVENSEG_B,
    PIN_SEVENSEG_C,
    PIN_SEVENSEG_D,
    PIN_SEVENSEG_E,
    PIN_SEVENSEG_F,
    PIN_SEVENSEG_G,
    PIN_SEVENSEG_DP
};

//functions
static void setDigitCodes(uint8_t digits);

void start_SevSeg () {//sets the values of the digitpin and segment pins in the struct, pinmode all of the pins, and turns them all off
  return;
  pinMode(digitPinsIn[0], OUTPUT);
  pinMode(digitPinsIn[1], OUTPUT);

  for (int x = 0; x < 8; x++) {
    pinMode(segmentPinsIn[x], OUTPUT);	//sets digits pins as outputs
    digitalWrite(segmentPinsIn[x], HIGH);	//turns segment pins off
  }
}

void refresh_SevSeg() {//refreshes display and lights up the segments
  return;
  //write to pin 1
  digitalWrite(digitPinsIn[0], HIGH);//turns first segment pin on
  digitalWrite(digitPinsIn[1], LOW);//turns off second digit
  for (int i = 0; i < 2; i++) { //goes throught each digit code and lights up appriopriate segment for each one
    for (int j = 0; j < 8; j++) {
      digitalWrite(segmentPinsIn[j], (*digitCodes[i] & (1<<j)) ? LOW : HIGH);
    }
    delay(10);//this is so that 2 numbers can be displayed instantaneously
    //swap two digits, on next iteration of loop we write to other two digits
    digitalWrite(digitPinsIn[0], LOW);//turns off first digit
    digitalWrite(digitPinsIn[1], HIGH);//turns on second digit
  }
}

void setNewNum_SevSeg (uint8_t numToShow) {//sets the number the user wants
  setDigitCodes(numToShow);
}

static void setDigitCodes(uint8_t numToShow) { //this function gets the digit code of each number in the 'digit' array
  // Set the digitCode for each digit in the display(digit array)
  digitCodes[0] = digitCodeMap + (numToShow & 0xF);
  digitCodes[1] = digitCodeMap + ((numToShow >> 4) & 0xF);

  //if sd card is not active, turn on the decimal points
  if(!sd_active())
  {
    *digitCodes[0] |= 0x80;
    *digitCodes[1] |= 0x80;
  }
  else
  {
    *digitCodes[0] &= ~(0x80);
    *digitCodes[1] &= ~(0x80);
  }
}
