#ifndef __PIN_H
#define __PIN_H


#ifdef __cplusplus
extern "C" {
#endif
typedef struct GPIOPinAssignment {
  int portPin ;
  char *description;
  int function;
  int ccCommand;
} GPIOPinAssignment;

extern const struct GPIOPinAssignment pinCfg[43];

enum PinFunction {
  IO_SPARE = 0,
  IO_RESERVED, /* Used by Blue pill board for debug or boot, available for use as GPIO with some complications */
  OP_LED,
  OP_SCAN,
  IP_SCAN,
  IP_ADC,
  IP_ADC_LED_DIM,
  OP_SR_CLOCK,
  OP_SR_DATA,
  OP_SR_ENABLE,
  IP_SR_DATA,
  TM1637_CK,
  TM1637_DA,
  COMMON_DATA,
  IP_CONTACT,
  I2C_SDA,
  I2C_SCL,
  USB_M,
  USB_P,
  V5,
  VBAT,
  V3_3,
  GND,
  SWCLK,
  SWDIO,
  BOOT1,
  NRST,
  IO_WS2812
};

//const int NUM_SHIFT_REGISTER_BITS = 96; // 8 * 12 ; // 8 bits per shift register, 4 divisions(3 keyboards+pedals), 3 shift registers each division
#define NUM_SHIFT_REGISTER_BITS   8*12
/* const struct GPIOPinAssignment pin[]  = {
  // ordered in sequence around the PCB to assist wiring up
  // Rational for pin assignment
  // ADC 1-8, I2C and LED are predefined
  // All digital inputs are chosen to be 5V tolerant (makes driving from 5V shift registers optional)
  // Scan inputs grouped together
  // shift register io are grouped together
  // scan outputs assigned in sequence
  // STM32 IC Pins 1-22,7
  { VBAT, "VBAT", IO_RESERVED },
  { PC13, "PC13", OP_LED   }, // OP1 PCB LED active low. Sink max 3mA, Source 3mA, Not 5V Tolerant
  { PC14, "PC14", OP_SCAN  }, // OP2  Sink max 3mA, Source 3mA, Not 5V Tolerant
  { PC15, "PC15", OP_SCAN  }, // OP3  Sink max 3mA, Source 3mA, Not 5V Tolerant
  { PA0, "PA_0" , IP_ADC  }, // ADC1 Expression Shoe Swell / Crescendo
  { PA1, "PA_1" , IP_ADC  }, // ADC2 Expression Shoe Swell / Crescendo
  { PA2, "PA_2" , IP_ADC  }, // ADC3 Expression Shoe Swell / Crescendo
  { PA3, "PA_3" , IP_ADC  }, // ADC4 Expression Shoe Swell / Crescendo
  { PA4, "PA_4" , IP_ADC  }, // ADC5 LED Brightness?
  { PA5, "PA_5" , IP_ADC  }, // ADC6
  { PA6, "PA_6" , IP_ADC  }, // ADC7
  { PA7, "PA_7" , IP_ADC  }, // ADC8
  { PB0, "PB_0" , IP_ADC  }, // ADC9
  { PB1, "PB_1" , IP_ADC  }, // ADC10
  { PB10, "PB10" , IP_SCAN }, // IP 1
  { PB11, "PB11" , IP_SCAN }, // IP 2
  { NRST, "NRST", IO_RESERVED },
  { V3_3, "3V3" , IO_RESERVED },
  {  GND, "GND" , IO_RESERVED },
  {  GND, "GND" , IO_RESERVED },
  // STM32 IC Pins 25-46
  { PB12, "PB12", IP_SCAN }, // IP 3
  { PB13, "PB13", IP_SCAN }, // IP 4
  { PB14, "PB14", IP_SCAN }, // IP 5
  { PB15, "PB15", IP_SCAN }, // IP 6
  { PA8, "PA_8", IP_SCAN }, // IP 7
  { PA9, "PA_9", IP_SCAN }, // IP 8
  { PA10, "PA10", IP_SCAN }, // IP 9
  { PA11, "PA11", USB_M },
  { PA12, "PA12", USB_P },
  { PA15, "PA15", IP_SCAN }, // IP10
  { PB3, "PB_3", IP_SCAN }, // IP11
  { PB4, "PB_4", IP_SCAN }, // IP12
  { PB5, "PB_5", TM1637_CK },
  { PB6, "PB_6", I2C_SCL }, // I2C SCL1 LCD Displays
  { PB7, "PB_7", COMMON_DATA }, // I2C SDA1 LCD Displays and SR_DATA combined. 4094 Pin 2 Shift registers are used for button input and associated LED indicator output
  { PB8, "PB_8", IP_SR_DATA  }, // 4094 Pin3
  { PB9, "PB_9", OP_SR_CLOCK, SHIFT_REGISTER_BITS }, // 4094 Pin
  { V5 , "+5V" , IO_RESERVED },
  { GND , "GND", IO_RESERVED },
  { V3_3, "3V3", IO_RESERVED },
  { PA13, "PA13", SWDIO }, // SWDIO - Located on Debug connector - Install boot loader before using for GPIO
  { PA14, "PA14", SWCLK }, // SWCLK - Located on Debug connector - Install boot loader before using for GPIO
  { PB2, "PB_2", BOOT1 }, // Boot1 - To use for GPIO remove Boot 1 link, and short R10 (100k) using 270R resistor
}; */
const int NUM_GPIO_PINS = sizeof(pinCfg) / sizeof( pinCfg[0] );


#ifdef __cplusplus
}
#endif


#endif /* __PIN_H */
