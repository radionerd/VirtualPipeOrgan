#ifndef __LED_H
#define __LED_H


#ifdef __cplusplus
extern "C" {
#endif


//  led.h 

//  Copyright (C)2022 richard (dot) jones (dot ) 1952 ( at ) gmail (dot ) com

// License MIT

class LED {
  private:
    const int ON  = 0;
    const int OFF = 1;
    const unsigned long HOLDOFF_US = 5000000L;
    int ledpin;
    int led_state;
    unsigned long holdoff;
  public:
   LED ( int pin = LED_BUILTIN ) {
     ledpin = pin;
     pinMode ( ledpin ,  OUTPUT );
     holdoff = micros();
   }
   
   void on ( void )   { digitalWrite( ledpin , ON ); led_state =  ON; holdoff = micros() + HOLDOFF_US; }
   
   void off( void )   { digitalWrite( ledpin , OFF); led_state = OFF; holdoff = micros() + HOLDOFF_US; }
   
   void toggle (void) { digitalWrite( ledpin , (++led_state)&1 ); holdoff = micros() + HOLDOFF_US; }
   
   void service(void) {
     unsigned long time = micros();
     if ( time > holdoff ) {
       holdoff = 0; // Guard against wrap around
       unsigned int pulse_width = ( time & 0x1FFFFF ) >> 6 ; // 2097152us or 0x200000us period
       if ( pulse_width > 16383 )
         pulse_width = 32768 - pulse_width;
       int led_state = HIGH;
       // DEBUG See if breathing LED can co-exist with WS2812 driver
       if ( pulse_width > 15383)
         pulse_width = 15383;
       if ( pulse_width < 1000 )
         pulse_width = 1000;
       if ( pulse_width > (time & 16383 ) )  
         led_state = LOW;
       digitalWrite(LED_BUILTIN, led_state );   // update the LED    
     }
   }
};

extern LED led; // or led(PC13); choose your own LED gpio pin

#ifdef __cplusplus
}
#endif


#endif /* __LED_H */
