#include <Arduino.h>
#include "buttonscan.h"

/*
 * Fast Button Scan of 74HC164 shift register connected buttons and LEDs
 * Call button scan with ptrs to input and output array,
 * returns number of active button inputs
 */

// Lamp test feature on startup
ButtonScan::ButtonScan(void){ 
  pinMode(PB12,INPUT_PULLDOWN); // data in
  pinMode(PB11,OUTPUT); // Clock
  pinMode(PB7,OUTPUT); // Data out
  *PB07_BB = 1;
  ShiftRegN8ClockBB(NUM_SHIFT_REGS); // Set all shift reg outputs to 1 
}

void ButtonScan::ShiftRegN8ClockBB( int count ) {
	  do {
	    PB11_BB[0] = 0;
	    PB11_BB[0] = 1;
	    PB11_BB[0] = 0;
	    PB11_BB[0] = 1;
	    PB11_BB[0] = 0;
	    PB11_BB[0] = 1;
	    PB11_BB[0] = 0;
	    PB11_BB[0] = 1;
	    PB11_BB[0] = 0;
	    PB11_BB[0] = 1;
	    PB11_BB[0] = 0;
	    PB11_BB[0] = 1;
	    PB11_BB[0] = 0;
	    PB11_BB[0] = 1;
	    PB11_BB[0] = 0;
	    PB11_BB[0] = 1;
	  } while ( -- count > 0 );
	}


// 43.6us when a button is active
// 19.2us when all buttons inactive
uint32_t ButtonScan::Scan(uint32_t *sr_input_list,uint32_t*sr_outputs, bool LEDInvert) {
	  static int active_inputs=0;
	  pinMode(PB12,INPUT_PULLDOWN); // data in
	  pinMode(PB11,OUTPUT); // Clock
	  pinMode(PB7,OUTPUT); // Data out

	  if ( active_inputs == 0 ) { // Commented out for DEBUG
	    // Detect if any buttons have become active
	    *PB07_BB = 1;
	    ShiftRegN8ClockBB(NUM_SHIFT_REGS); // Set all shift reg outputs to 1
      *PB11_BB = 0;
      *PB11_BB = 1;
	    active_inputs = *PB12_BB;
	  }
	  if ( active_inputs ) {
	    // Detect which buttons are active
	    active_inputs = 0;
	    *PB07_BB = 0;
	    ShiftRegN8ClockBB(NUM_SHIFT_REGS); // Clear all shift reg outputs to zero
	    *PB07_BB = 1; // Move a single 1 along all shift reg outputs recording active buttons
	    *PB11_BB = 0; // Output one clock pulse
	    *PB11_BB = 1;
	    *PB07_BB = 0;
	    for ( int index = 0; index < NUM_SHIFT_REG_OUTPUTS;index++) {
        *PB11_BB = 0;
	      if ( *PB12_BB != 0 ) {
		      unsigned long time = micros();
		      while ( (micros() < time+4 ) && *PB12_BB ); // fall time delay to discharge line capacitance if previous input was active
          time = micros();
          while ( micros() < time+2 ); // fall time delay to discharge line capacitance if previous input was active
		      if ( *PB12_BB != 0 ) {
		        sr_input_list[active_inputs++] = index; // record button number
		      }
	      }
	      *PB11_BB = 1; // Early rising clock to allow propogation delay to/from far away shift registers        *PB11_BB = 1; // Early rising clock to allow propogation delay to/from far away shift registers
	    }
	  }
	  // Load output LEDs
    if ( LEDInvert ) {
	   for ( int index = 1; index <= NUM_SHIFT_REG_OUTPUTS;index++) {
	    *PB07_BB = sr_outputs[NUM_SHIFT_REG_OUTPUTS-index]^1;
	    *PB11_BB = 0;
	    *PB11_BB = 1;
	   }
	  } else {
     for ( int index = 1; index <= NUM_SHIFT_REG_OUTPUTS;index++) {
      *PB07_BB = sr_outputs[NUM_SHIFT_REG_OUTPUTS-index];
      *PB11_BB = 0;
      *PB11_BB = 1;
     }
	  }
	  *PB07_BB = 1; // Tidy up for scope monitoring
	  return active_inputs;
}
