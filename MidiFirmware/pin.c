#include <Arduino.h>
#include "pin.h"

const GPIOPinAssignment pinCfg[43]  = {
  // ordered in sequence around the PCB to assist wiring up
  // Rationale for pin assignment:
  // ADC 1-8, I2C and LED are predefined
  // All digital inputs are chosen to be 5V tolerant (makes driving from 5V shift registers a future option)
  // Scan inputs grouped together
  // shift register io are grouped together
  // scan outputs assigned in ascending sequence
  // STM32 IC Pins 1-22,7
  { VBAT, "VBAT", IO_RESERVED },
  { PC13, "PC13", OP_LED   }, // OP1 PCB LED active low. Sink max 3mA, Source 3mA, Not 5V Tolerant
  { PC14, "PC14", OP_SCAN,7 }, // CN14  1 OUT
  { PC15, "PC15", OP_SCAN,6 }, // CN14  2 OUT 
  { PA0, "PA_0" , OP_SCAN,5 }, // CN14  3 OUT
  { PA1, "PA_1" , OP_SCAN,4 }, // CN14  4 OUT
  { PA2, "PA_2" , IP_SCAN,15 }, // CN14  5 IN
  { PA3, "PA_3" , IP_SCAN,14 }, // CN14  6 IN
  { PA4, "PA_4" , IP_SCAN,13 }, // CN14  7 IN 
  { PA5, "PA_5" , IP_SCAN,12 }, // CN14  8 IN 
  { PA6, "PA_6" , IP_SCAN,11 }, // CN14  9 IN 
  { PA7, "PA_7" , IP_SCAN,10 }, // CN14 10 IN 
  { PB0, "PB_0" , IP_SCAN,9 }, // CN14 11 IN 
  { PB1, "PB_1" , IP_SCAN,8 }, // CN14 12 IN 
  { PB10, "PB10" ,TM1637_CK },
  { PB11, "PB11" ,OP_SR_CLOCK, NUM_SHIFT_REGISTER_BITS }, // CLK  Buttons  74HC164
  { NRST, "NRST", IO_RESERVED }, 
  { V3_3, "3V3" , IO_RESERVED },
  {  GND, "GND" , IO_RESERVED },
  {  GND, "GND" , IO_RESERVED },
  // STM32 IC Pins 25-46
  { PB12, "PB12", IP_SR_DATA }, // DIN Buttons
  { PB13, "PB13", IP_SCAN,0x0 }, // CN13 13 IN
  { PB14, "PB14", IP_SCAN,0x1 }, // CN13 12 IN
  { PB15, "PB15", IP_SCAN,0x2 }, // CN13 11 IN
  { PA8,  "PA_8", IP_SCAN,0x3 }, // CN13 10 IN
  { PA9,  "PA_9", IP_SCAN,0x4 }, // CN13  9 IN
  { PA10, "PA10", IP_SCAN,0x5 }, // CN13  8 IN
  { PA11, "PA11", USB_M },
  { PA12, "PA12", USB_P },
  { PA15, "PA15", IP_SCAN,6 }, // CN13  6 IN
  { PB3,  "PB_3", IP_SCAN,7 }, // CN13  5 IN
  { PB4,  "PB_4", OP_SCAN,0 }, // CN13  4 OUT
  { PB5,  "PB_5", OP_SCAN,1 }, // CN13  3 OUT
  { PB6,  "PB_6", I2C_SCL }, // I2C SCL1 LCD Displays
  { PB7,  "PB_7", I2C_SDA }, // I2C SDA1 LCD Displays and SR_DATA combined. 4094 Pin 2 Shift registers are used for button input and associated LED indicator output
  { PB8,  "PB_8", OP_SCAN,2 }, // CN13  2 OUT
  { PB9,  "PB_9", OP_SCAN,3 }, // CN13  1 OUT
  { V5 ,  "+5V" , IO_RESERVED },
  { GND , "GND",  IO_RESERVED },
  { V3_3, "3V3",  IO_RESERVED },
  { PA13, "PA13", IO_SPARE }, // SWDIO - Located on Debug connector - Install boot loader before using for GPIO
  { PA14, "PA14", IO_SPARE }, // SWCLK - Located on Debug connector - Install boot loader before using for GPIO
  { PB2,  "PB_2", BOOT1 }, // Boot1 - To use for GPIO remove Boot 1 link, and short R10 (100k) using 270R resistor
};
