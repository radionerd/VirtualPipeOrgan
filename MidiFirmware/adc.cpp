#include <Arduino.h>
#include <USBComposite.h>
#include "adc.h"
#include "led.h"
#include "mymidi.h"
#include "pin.h"
#include "profile.h"
#include "USBSerialUI.h"

const int MIN_ADC_GPIOID =  6;
const int MAX_ADC_GPIOID = 13;
const int MIDI_CC_BASE = 20;

const int SCALE_0_TO_127 = 32;  // Range 0-127. May need adjustment depending on swell pedal shoe geometry

extern myMidi midi;
 
 void ADC::Scan ( void ) {
   static unsigned long last_time=0;

   // scan at 40ms intervals for consistant performance
   // multiples of mains cycle period eg 20,40,60ms etc
   // balance between response time and message flood
   unsigned long time_now = micros();
   if ( time_now - last_time >= 40000 ) {
    profile.PStart(PROFILE_ADC);
    if ( last_time == 0 ) // stagger run times to minimise max loop time
      last_time += PID_TIME_OFFSET_US * PROFILE_ADC;
    last_time += 40000;
    //for ( int gpioId = 0 ; gpioId < NUM_GPIO_PINS ; gpioId++ ) {
    for ( int gpioId = MIN_ADC_GPIOID ; gpioId <= MAX_ADC_GPIOID ; gpioId++ ) {
     switch ( SUI.LiveConfigs[gpioId].function ) {
      case IP_ADC :
      {
        pinMode (  pinCfg[gpioId].portPin, INPUT );
        int ain = analogRead( pinCfg[gpioId].portPin ); // a value between 0-4095
        // Filter the adc input to get a stable result
        int prevInput = SUI.LiveConfigs[gpioId].input;
        const int FILTER_SAMPLES = 4;
        SUI.LiveConfigs[gpioId].input = ( prevInput * ( FILTER_SAMPLES - 1 ) + ain ) / FILTER_SAMPLES; // Moving average
        int prev_result = prevInput / SCALE_0_TO_127; // scale 0-4095 to 0-127 for midi cc command
        int new_result  = SUI.LiveConfigs[gpioId].input / SCALE_0_TO_127; // scale 0-4095 to 0-127 for midi cc command
        int error = abs( SUI.LiveConfigs[gpioId].error - SUI.LiveConfigs[gpioId].input);
        // Avoid hunting between adjacent numbers while allowing min and max scale
        if( new_result != prev_result ) {
           if ( ( error >= ( SCALE_0_TO_127/2 ) ) || ( new_result == 0 ) || ( new_result == 127 ) ) {
            if ( error > SCALE_0_TO_127 * 3 )
              SUI.LiveConfigs[gpioId].input = ain;
            SUI.LiveConfigs[gpioId].error = SUI.LiveConfigs[gpioId].input;
            int midi_channel      = midi.getKeyboardChannel();;
            int cc_command_number = SUI.LiveConfigs[gpioId].count + MIDI_CC_BASE; // Midi CC command range
            led.toggle(); // Indicate USB signalling PCB LED
            midi.sendControlChange(midi_channel,cc_command_number, new_result );
            SUI.RequestDisplayUpdate();
            if ( SUI.Cfg.Bits.hasEventLog ) {
                char buff[110];
                sprintf(buff,"%10luus MidiControl Ch=%-2d  Cont=%d  Val=%d ADC\r\n",micros(),midi_channel+1,cc_command_number, new_result );
                CompositeSerial.write(buff);
            }
          }
        }
      }
      break;
     }
    }
    profile.PEnd(PROFILE_ADC);
   }
 }
 
void ADC::Begin(void) {
  // Read current potentiometer input to avoid a message stream at startup
   for ( int gpioId = MIN_ADC_GPIOID ; gpioId <= MAX_ADC_GPIOID ; gpioId++ ) {
//   for ( int gpioId = 0 ; gpioId < NUM_GPIO_PINS ; gpioId++ ) {
     switch ( SUI.LiveConfigs[gpioId].function ) {
      case IP_ADC :
      {
        pinMode (  pinCfg[gpioId].portPin, INPUT);
        SUI.LiveConfigs[gpioId].input = analogRead( pinCfg[gpioId].portPin  ); // a value between 0-4095
      }
     }
   }
}

void ADC::Print(void) {
   char buff[80];
   
   CompositeSerial.write(VT100_CLEAR);
   sprintf(buff,"Expression Pedal/ADC Results\r\n    ADC   AVG   REP MidiCh CCommand\r\n");
   //SUI.DisplayTitle(buff);
   CompositeSerial.write(buff);   
   for ( int gpioId = MIN_ADC_GPIOID ; gpioId <= MAX_ADC_GPIOID ; gpioId++ ) {
//   for ( int gpioId = 0 ; gpioId < NUM_GPIO_PINS ; gpioId++ ) {
     switch ( SUI.LiveConfigs[gpioId].function ) {
      case IP_ADC :
      {
        int midi_channel      = midi.getKeyboardChannel();
        int cc_command_number = SUI.LiveConfigs[gpioId].count + MIDI_CC_BASE; // Midi CC command range Next 8 values
        sprintf( buff,"%d%6d%6d%6d%6d%6d\r\n",
          SUI.LiveConfigs[gpioId].count, 
          analogRead( pinCfg[gpioId].portPin ), 
          SUI.LiveConfigs[gpioId].input,
          SUI.LiveConfigs[gpioId].input / SCALE_0_TO_127,
          midi_channel+1,
          cc_command_number ); // a value between 0-4095
        CompositeSerial.write(buff);
      }
     }
   }   
   //SUI.RequestDisplayUpdate();
/*   for ( int gpioId = 0 ; gpioId < NUM_GPIO_PINS ; gpioId++ ) {
     switch ( SUI.LiveConfigs[gpioId].function ) {
       case IP_ADC :
       {
         pinMode (  pinCfg[gpioId].portPin, INPUT);
         sprintf( buff,"     %d   %4d  %d \r\n", analogRead( pinCfg[gpioId].portPin ), midi_channel,midi_cc_command_number ); // a value between 0-4095
         CompositeSerial.write(buff);
       }
     }
   } */
} 
