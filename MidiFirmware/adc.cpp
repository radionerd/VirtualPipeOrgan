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
   static unsigned callCount;
   if ( (++callCount &3 ) == 0 ) // run at 40ms intervals
   {
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
    //profile.PEnd(PROFILE_ADC);
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
   
   //CompositeSerial.write(VT100_CLEAR);  
   CompositeSerial.write(ANSI_CLEAR);
   sprintf(buff,"%sExpression Pedal/ADC Results\r\n    ADC   AVG   REP MidiCh CCommand\r\n",ANSI_CURSOR_00);
   //SUI.DisplayTitle(buff);
   CompositeSerial.write(buff);   
   for ( int gpioId = MIN_ADC_GPIOID ; gpioId <= MAX_ADC_GPIOID ; gpioId++ ) {
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
} 
