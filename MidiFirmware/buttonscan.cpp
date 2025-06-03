#include <Arduino.h>
#include <USBComposite.h>
#include "buttonscan.h"
#include "color_wheel.h"
#include "led.h"
#include "mymidi.h"
#include "pin.h"
#include "profile.h"
#include "USBSerialUI.h"

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
uint32_t ButtonScan::ScanSR(uint32_t *sr_input_list,uint32_t*sr_outputs ) {
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
    // Experiment with baclighting for dark buttons
    if ( SUI.Cfg.Bits.hasButtonLEDBacklit ) {
      unsigned long time_now = micros();
      *PB07_BB = 1; // Set all buttons on for 1ms
      for ( int index = 1; index <= NUM_SHIFT_REG_OUTPUTS;index++) {
        *PB11_BB = 0;
        *PB11_BB = 1;
      }
      while ( ( micros() - time_now ) < 1000 );
    }
	  // Load output LEDs
    if ( SUI.Cfg.Bits.hasButtonLEDInvert  ) {
	   for ( int index = 1; index <= NUM_SHIFT_REG_OUTPUTS;index++) {
	    *PB07_BB = sr_outputs[NUM_SHIFT_REG_OUTPUTS-index]^1;
	    *PB11_BB = 0;
	    *PB11_BB = 1;
	   }
	  } else {
     for ( int index = 1; index <= NUM_SHIFT_REG_OUTPUTS;index++) {
      *PB07_BB = sr_outputs[NUM_SHIFT_REG_OUTPUTS-index]^0;
      *PB11_BB = 0;
      *PB11_BB = 1;
     }
	  }
	  *PB07_BB = 1; // Tidy up for scope monitoring
	  return active_inputs;
}

void ButtonScan::Scan(void) {

  static unsigned long last_time=0;
  unsigned long time_now = micros();
  if ( time_now - last_time < 20000 ) return;
  profile.PStart(PROFILE_BUTTONS);
  last_time += 20000; // 20ms 50 times per second update rate
  const int midi_velocity = 63;
  const unsigned long AUTO_REPEAT_DELAY_US = 2000000; // 2 seconds
  const unsigned long AUTO_REPEAT_PERIOD_US = 333000; // 0.333 seconds

  static int active_buttons=0;
  int result = ScanSR( sr_input_list,sr_outputs );
  // NB The strange sr_input_list interface yields a fast inactive scan result
  if ( ( active_buttons != result ) || active_buttons ) { 
    active_buttons = result;
//    if ( active_buttons ) { // removed for auto repeat
      for ( int j = 0 ; j < active_buttons ; j++ ) {
        // Flag all new active buttons
        if ( ShiftRegImage[sr_input_list[j] ].Input == 0 ) {
          ShiftRegImage[ sr_input_list[j] ].Input = 2; // Flag just becoming active
          ShiftRegImage[ sr_input_list[j] ].Time = time_now; // record time for auto repeat
          if ( SUI.GetAppOnlineTime() == 0 ) {
            sr_outputs[  sr_input_list[j] ] = 1; // Button test when not online
          }
          //ShiftRegImage[ sr_input_list[j] ].Output = ! ShiftRegImage[ sr_input_list[j] ].Output; // DEBUG Toggle outputs
          int midi_note = sr_input_list[j];
          if( ShiftRegImage[ 0 ].Input && ( midi_note != 0 ) ) { // Shifted key?
              midi_note += NUM_SHIFT_REG_OUTPUTS ;
          }
          int midi_channel = midi.getButtonChannel() ;
          profile.PStart(PROFILE_MIDI_OUT_TO_SYSEX_IN);
          led.on();
          midi.sendNoteOn ( midi_channel, midi_note, midi_velocity );
          SUI.monitorNoteOn   ( midi_channel, midi_note, midi_velocity, SUI.DEV_BUTTON  );
          SUI.RequestDisplayUpdate();
          if ( midi_note==0 )
            if ( SUI.Cfg.Bits.hasPB2PA13PA14Scan )
               LEDStripCtrl( LED_STRIP_BUTTON_ON ); 
        } else {
          ShiftRegImage[ sr_input_list[j] ].Input = 2; // Flag is active
//          unsigned long elapsed_time_us = time_now - ShiftRegImage[ sr_input_list[j] ].Time;
          //char buf[80];
          //sprintf(buf,"Tnow=%lu Telapsed=%lu\r\n",time_now,elapsed_time_us);
          //CompositeSerial.write(buf);
          if ( (sr_input_list[j]!=0 ) && ( ( time_now - ShiftRegImage[ sr_input_list[j] ].Time ) > AUTO_REPEAT_DELAY_US ) ) {
            ShiftRegImage[ sr_input_list[j] ].Time += AUTO_REPEAT_PERIOD_US;
            //ShiftRegImage[ sr_input_list[j] ].Output = ! ShiftRegImage[ sr_input_list[j] ].Output; // DEBUG Toggle outputs
            int midi_note = sr_input_list[j];
            if( ShiftRegImage[ 0 ].Input && ( midi_note != 0 ) ) { // Shifted key?
              midi_note += NUM_SHIFT_REG_OUTPUTS ;
            }
            int midi_channel = midi.getButtonChannel() ;
            led.on();
            midi.sendNoteOn ( midi_channel, midi_note, midi_velocity );
            SUI.monitorNoteOn   ( midi_channel, midi_note, midi_velocity, SUI.DEV_BUTTON  );
            SUI.RequestDisplayUpdate();
          }
        }
        // sr_outputs[sr_input_list[j]] = ShiftRegImage[sr_input_list[j]].Output; // DEBUG without GrandOrgue Update LED displays
      }
//    }
    for ( int i = 0 ; i < NUM_SHIFT_REG_OUTPUTS ; i++ ) {
      if ( ShiftRegImage[ i ].Input ) {
        if ( -- ShiftRegImage[ i ].Input == 0 ) { // Detect just going inactive
          // Send OFF to MIDI
          int midi_channel = midi.getButtonChannel() ;
          int midi_note = i ;
          if ( SUI.GetAppOnlineTime() == 0 ) {
            sr_outputs[ midi_note ] = 0; // Button test when not online
          }
          if( ShiftRegImage[ 0 ].Input && ( midi_note != 0 ) ) { // Shifted key?
              midi_note += NUM_SHIFT_REG_OUTPUTS ;
          }
          led.off();
          midi.sendNoteOff ( midi_channel, midi_note, midi_velocity );
          SUI.monitorNoteOff   ( midi_channel, midi_note, midi_velocity, SUI.DEV_BUTTON  );
          SUI.RequestDisplayUpdate();
          if ( midi_note==0 )
            if ( SUI.Cfg.Bits.hasPB2PA13PA14Scan )
              LEDStripCtrl( LED_STRIP_BUTTON_OFF ); 
        }
      }
    }
  }
  profile.PEnd  (PROFILE_BUTTONS);
}

void ButtonScan::Print(void) {
  char buff[80];
  
  CompositeSerial.write(VT100_CLEAR);
  sprintf(buff, "Shift Register Midi Channel %d Note Numbers", midi.getButtonChannel()+1 );
  SUI.DisplayTitle( buff );
  for ( int sr = 0 ; sr < NUM_SHIFT_REGS ; sr++ ) {
    sprintf(buff, "\r\nSR%2d  ", 1 + sr );
    for ( int i = 0 ; i < SHIFT_REG_SIZE ; i++ ) {
      char ip = ' ';
      char op = ' ';
      if ( ShiftRegImage[sr*SHIFT_REG_SIZE+i].Input ) ip='i';
      // if ( ShiftRegImage[sr*8+i].Output) op='o';
      if ( sr_outputs[sr*SHIFT_REG_SIZE+i] ) op='o';
      sprintf(buff + strlen(buff), "%c%02d%c ", ip, sr * 8 + i, op );
    }
    CompositeSerial.write(buff);
  }
  CompositeSerial.write("\r\n\ni = input on, o = output on");
}

void ButtonScan::SetLED(int output, int value ) {
  if ( output < NUM_SHIFT_REG_OUTPUTS )
  {
    sr_outputs[output] = value;
  }
}
