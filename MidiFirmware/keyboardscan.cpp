#include <Arduino.h>
#include <USBComposite.h>
#include "keyboardscan.h"

KeyboardScan::KeyboardScan(){

/*  pinMode(PB13,INPUT_PULLDOWN); // data in 0x01
  pinMode(PB14,INPUT_PULLDOWN); // data in 0x02
  pinMode(PB15,INPUT_PULLDOWN); // data in 0x04
  pinMode(PA8 ,INPUT_PULLDOWN); // data in 0x08
  
  pinMode(PA9 ,INPUT_PULLDOWN); // data in 0x10
  pinMode(PA10,INPUT_PULLDOWN); // data in 0x20
  pinMode(PA15,INPUT_PULLDOWN); // data in 0x40
  pinMode(PB3 ,INPUT_PULLDOWN); // data in 0x80

  pinMode(PB1 ,INPUT_PULLDOWN); // data in 0x0100
  pinMode(PB0 ,INPUT_PULLDOWN); // data in 0x0200
  pinMode(PA7 ,INPUT_PULLDOWN); // data in 0x0400
  pinMode(PA6 ,INPUT_PULLDOWN); // data in 0x0800

  pinMode(PA5 ,INPUT_PULLDOWN); // data in 0x1000
  pinMode(PA4 ,INPUT_PULLDOWN); // data in 0x2000
  pinMode(PA3 ,INPUT_PULLDOWN); // data in 0x4000
  pinMode(PA2 ,INPUT_PULLDOWN); // data in 0x8000

  pinMode(PB4 ,OUTPUT);
  pinMode(PB5 ,OUTPUT);
  pinMode(PB8 ,OUTPUT);
  pinMode(PB9 ,OUTPUT);

  pinMode(PA1 ,OUTPUT);
  pinMode(PA0 ,OUTPUT);
  pinMode(PC15,OUTPUT);
  pinMode(PC14,OUTPUT); */ 
}
uint32_t KeyboardScan::get_input(bool pedalboard) {
  const volatile uint32_t* IN_0X0001 = (uint32_t*) (BB + ( (PORTB+IDR)<<5 ) + ((uint32_t) 13 << 2 ));
  const volatile uint32_t* IN_0X0002 = (uint32_t*) (BB + ( (PORTB+IDR)<<5 ) + ((uint32_t) 14 << 2 ));
  const volatile uint32_t* IN_0X0004 = (uint32_t*) (BB + ( (PORTB+IDR)<<5 ) + ((uint32_t) 15 << 2 ));
  const volatile uint32_t* IN_0X0008 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  8 << 2 ));

  const volatile uint32_t* IN_0X0010 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  9 << 2 ));
  const volatile uint32_t* IN_0X0020 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t) 10 << 2 ));
  const volatile uint32_t* IN_0X0040 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t) 15 << 2 ));
  const volatile uint32_t* IN_0X0080 = (uint32_t*) (BB + ( (PORTB+IDR)<<5 ) + ((uint32_t)  3 << 2 ));

  const volatile uint32_t* IN_0X0100 = (uint32_t*) (BB + ( (PORTB+IDR)<<5 ) + ((uint32_t)  1 << 2 ));
  const volatile uint32_t* IN_0X0200 = (uint32_t*) (BB + ( (PORTB+IDR)<<5 ) + ((uint32_t)  0 << 2 ));
  const volatile uint32_t* IN_0X0400 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  7 << 2 ));
  const volatile uint32_t* IN_0X0800 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  6 << 2 ));

  const volatile uint32_t* IN_0X1000 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  5 << 2 ));
  const volatile uint32_t* IN_0X2000 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  4 << 2 ));
  const volatile uint32_t* IN_0X4000 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  3 << 2 ));
  const volatile uint32_t* IN_0X8000 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  2 << 2 ));

  if ( ! pedalboard ) {
    return ((*IN_0X0001) <<  0 ) +
           ((*IN_0X0002) <<  1 ) +
           ((*IN_0X0004) <<  2 ) +
           ((*IN_0X0008) <<  3 ) +
           ((*IN_0X0010) <<  4 ) +
           ((*IN_0X0020) <<  5 ) +
           ((*IN_0X0040) <<  6 ) +
           ((*IN_0X0080) <<  7 ) +
           ((*IN_0X0100) <<  8 ) +
           ((*IN_0X0200) <<  9 ) +
           ((*IN_0X0400) << 10 ) +
           ((*IN_0X0800) << 11 ) +
           ((*IN_0X1000) << 12 ) +
           ((*IN_0X2000) << 13 ) +
           ((*IN_0X4000) << 14 ) +
           ((*IN_0X8000) << 15 );
  } else {
    return ((*IN_0X0001) <<  0 ) +
           ((*IN_0X0002) <<  1 ) +
           ((*IN_0X0004) <<  2 ) +
           ((*IN_0X0008) <<  3 ) +
           ((*IN_0X0010) <<  4 ) +
           ((*IN_0X0020) <<  5 ) +
           ((*IN_0X0040) <<  6 ) +
           ((*IN_0X0080) <<  7 ) ;
  }
}

// Measured 
// ~17us inactive inputs
// ~+24us to scan all inputs with active pull down
// Each scan line active for 1.84us
uint32_t KeyboardScan::Scan(uint32_t *kb_input,uint32_t*kb_image, bool pedalboard )
{
  static unsigned long lastTime=0;
  unsigned long time_now = micros();
  if ( time_now - lastTime < 1000 ) return 0; // 1ms scan intervals to reduce emi
  lastTime=time_now;
  volatile uint32_t* OUT1 = (uint32_t*) (BB + ( (PORTB+ODR)<<5 ) + ((uint32_t)  4 << 2 ));
  volatile uint32_t* OUT2 = (uint32_t*) (BB + ( (PORTB+ODR)<<5 ) + ((uint32_t)  5 << 2 ));
  volatile uint32_t* OUT3 = (uint32_t*) (BB + ( (PORTB+ODR)<<5 ) + ((uint32_t)  8 << 2 ));
  volatile uint32_t* OUT4 = (uint32_t*) (BB + ( (PORTB+ODR)<<5 ) + ((uint32_t)  9 << 2 ));

  volatile uint32_t* OUT5 = (uint32_t*) (BB + ( (PORTA+ODR)<<5 ) + ((uint32_t)  1 << 2 ));
  volatile uint32_t* OUT6 = (uint32_t*) (BB + ( (PORTA+ODR)<<5 ) + ((uint32_t)  0 << 2 ));
  volatile uint32_t* OUT7 = (uint32_t*) (BB + ( (PORTC+ODR)<<5 ) + ((uint32_t) 15 << 2 ));
  volatile uint32_t* OUT8 = (uint32_t*) (BB + ( (PORTC+ODR)<<5 ) + ((uint32_t) 14 << 2 ));

  volatile uint32_t* Outputn[] = { OUT1,OUT2,OUT3,OUT4,OUT5,OUT6,OUT7,OUT8 };
  int change = 0;

  // DEBUG If active input re-initialise, caused by Test command?
  /*if ( get_input() )*/ {
    pinMode(PB13,INPUT_PULLDOWN); // data in 0x01
    pinMode(PB14,INPUT_PULLDOWN); // data in 0x02
    pinMode(PB15,INPUT_PULLDOWN); // data in 0x04
    pinMode(PA8 ,INPUT_PULLDOWN); // data in 0x08
  
    pinMode(PA9 ,INPUT_PULLDOWN); // data in 0x10
    pinMode(PA10,INPUT_PULLDOWN); // data in 0x20
    pinMode(PA15,INPUT_PULLDOWN); // data in 0x40
    pinMode(PB3 ,INPUT_PULLDOWN); // data in 0x80

    if ( ! pedalboard ) {
      pinMode(PB1 ,INPUT_PULLDOWN); // data in 0x0100
      pinMode(PB0 ,INPUT_PULLDOWN); // data in 0x0200
      pinMode(PA7 ,INPUT_PULLDOWN); // data in 0x0400
      pinMode(PA6 ,INPUT_PULLDOWN); // data in 0x0800

      pinMode(PA5 ,INPUT_PULLDOWN); // data in 0x1000
      pinMode(PA4 ,INPUT_PULLDOWN); // data in 0x2000
      pinMode(PA3 ,INPUT_PULLDOWN); // data in 0x4000
      pinMode(PA2 ,INPUT_PULLDOWN); // data in 0x8000
    }

    pinMode(PB4 ,OUTPUT);
    pinMode(PB5 ,OUTPUT);
    pinMode(PB8 ,OUTPUT);
    pinMode(PB9 ,OUTPUT);

    pinMode(PA1 ,OUTPUT);
    pinMode(PA0 ,OUTPUT);
    pinMode(PC15,OUTPUT);
    pinMode(PC14,OUTPUT);  
   
  }
  for ( int scan_line = 0 ; scan_line < NUM_KEYBOARD_OUTPUTS ; scan_line++ ) {
    *Outputn[scan_line] = 1; // raise output scan line
    volatile uint32_t input = get_input(pedalboard); // read 16 input lines
    *Outputn[scan_line] = 0 ; // lower output scan line
    kb_input[scan_line] = input;
    if ( kb_input[scan_line] != kb_image[scan_line] ) {
      change++;
    }
    if ( input ) {
      activePulldown(pedalboard);
    }
  }
  return change;
}

    /* Code generator for fast discharge of active input cables
      char buff[120];      
      sprintf(buff,"pedalboard=%d\r\n",pedalboard);
      CompositeSerial.write(buff);
      sprintf(buff,"&GPIOA->regs->CRL = %08x \r\n", &GPIOA->regs->CRL);
      CompositeSerial.write(buff);
      sprintf(buff," GPIOA->regs->CRL = %08x \r\n", GPIOA->regs->CRL);
      CompositeSerial.write(buff);
      sprintf(buff,"&GPIOA->regs->CRH = %08x \r\n", &GPIOA->regs->CRH);
      CompositeSerial.write(buff);
      sprintf(buff," GPIOA->regs->CRH = %08x \r\n", GPIOA->regs->CRH);
      CompositeSerial.write(buff);
      sprintf(buff,"&GPIOB->regs->CRL = %08x \r\n", &GPIOB->regs->CRL);
      CompositeSerial.write(buff);
      sprintf(buff," GPIOB->regs->CRL = %08x \r\n", GPIOB->regs->CRL);
      CompositeSerial.write(buff);
      sprintf(buff,"&GPIOB->regs->CRH = %08x \r\n", &GPIOB->regs->CRH);
      CompositeSerial.write(buff);
      sprintf(buff," GPIOB->regs->CRH = %08x \r\n\n", GPIOB->regs->CRH);
      CompositeSerial.write(buff);

      const int input_pin [] ={PB13,PB14,PB15,PA8,PA9,PA10,PA15,PB3,PB1,PB0,PA7,PA6,PA5,PA4,PA3,PA2 };
      int nip = 16 ;
      if ( pedalboard ) nip = 8 ;
      for (int ip = 0 ; ip < nip ; ip++ ) {
        pinMode( input_pin[ip],OUTPUT);
      }
      int acrl,acrh,bcrl,bcrh;
      sprintf(buff,"&GPIOA->regs->CRL = %08x \r\n", &GPIOA->regs->CRL);
      CompositeSerial.write(buff);
      sprintf(buff," GPIOA->regs->CRL = %08x \r\n", acrl=GPIOA->regs->CRL);
      CompositeSerial.write(buff);
      sprintf(buff,"&GPIOA->regs->CRH = %08x \r\n", &GPIOA->regs->CRH);
      CompositeSerial.write(buff);
      sprintf(buff," GPIOA->regs->CRH = %08x \r\n", acrh=GPIOA->regs->CRH);
      CompositeSerial.write(buff);
      sprintf(buff,"&GPIOB->regs->CRL = %08x \r\n", &GPIOB->regs->CRL);
      CompositeSerial.write(buff);
      sprintf(buff," GPIOB->regs->CRL = %08x \r\n", bcrl=GPIOB->regs->CRL);
      CompositeSerial.write(buff);
      sprintf(buff,"&GPIOB->regs->CRH = %08x \r\n", &GPIOB->regs->CRH);
      CompositeSerial.write(buff);
      sprintf(buff," GPIOB->regs->CRH = %08x \r\n\n", bcrh=GPIOB->regs->CRH);
      CompositeSerial.write(buff);
      for (int ip = 0 ; ip < nip ; ip++ ) {
        pinMode( input_pin[ip],INPUT_PULLDOWN);
      }
      // Code generator
      sprintf(buff," if ( pedalboard == %d ) \r\n { \r\n", pedalboard);
      CompositeSerial.write(buff);
      sprintf(buff,"   GPIOA->regs->CRL ^= %08x ;\r\n", GPIOA->regs->CRL^acrl);
      CompositeSerial.write(buff);
      sprintf(buff,"   GPIOA->regs->CRH ^= %08x ;\r\n", GPIOA->regs->CRH^acrh);
      CompositeSerial.write(buff);
      sprintf(buff,"   GPIOB->regs->CRL ^= %08x ;\r\n", GPIOB->regs->CRL^bcrl);
      CompositeSerial.write(buff);
      sprintf(buff,"   GPIOB->regs->CRH ^= %08x ;\r\n }\r\n", GPIOB->regs->CRH^bcrh);
      CompositeSerial.write(buff);
    } */

void KeyboardScan::activePulldown(bool pedalboard ) {
    // To avoid long discharge times use pull down resistors 4k7 or less
    // OR active pull down by setting input pin briefly to output low
    // Tested with 330pF loading on input lines
    if ( pedalboard == 1 ) 
    { 
      // Set 8 input lines to output 0 @ 50MHz fall time
      GPIOA->regs->CRL ^= 0x00000000 ; // Redundant, but stays for constant discharge timing
      GPIOA->regs->CRH ^= 0xb0000bbb ;
      GPIOB->regs->CRL ^= 0x0000b000 ;
      GPIOB->regs->CRH ^= 0xbbb00000 ;
      // Revert 16 lines to input pull down
      GPIOA->regs->CRL ^= 0x00000000 ;
      GPIOA->regs->CRH ^= 0xb0000bbb ;
      GPIOB->regs->CRL ^= 0x0000b000 ;
      GPIOB->regs->CRH ^= 0xbbb00000 ;
    } else {
      // Set 16 input lines to output 0 @ 50MHz fall time
      GPIOA->regs->CRL ^= 0xbbbbbb00 ;
      GPIOA->regs->CRH ^= 0xb0000bbb ;
      GPIOB->regs->CRL ^= 0x0000b0bb ;
      GPIOB->regs->CRH ^= 0xbbb00000 ;
      // Revert 16 lines to input pull down
      GPIOA->regs->CRL ^= 0xbbbbbb00 ;
      GPIOA->regs->CRH ^= 0xb0000bbb ;
      GPIOB->regs->CRL ^= 0x0000b0bb ;
      GPIOB->regs->CRH ^= 0xbbb00000 ;
    }

    /* Alternative time out for input line discharge
    int count = 0;
    while ( ( ++count < 10 ) && input ) { // wait for active inputs to decay to inactive
      input = get_input(pedalboard);
    }
    count *= 2 ; // wait for about double the same time again to guard against noise
    while ( --count > 0 ) { 
      input = get_input(pedalboard);
    } if ( pedalboard == 0 ) 
    */
}
