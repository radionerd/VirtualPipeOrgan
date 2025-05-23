#include <Arduino.h>
#include <USBComposite.h>
#include "adc.h"
#include "mymidi.h"
#include "pin.h"
#include "profile.h"
#include "USBSerialUI.h"
 
// extern PROFILE profile;
// extern GPIOPinConfig LiveConfigs[];
 
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
    for ( int gpioId = 0 ; gpioId < NUM_GPIO_PINS ; gpioId++ ) {
     switch ( SUI.LiveConfigs[gpioId].function ) {
      case IP_ADC :
      {
        pinMode (  pinCfg[gpioId].portPin, INPUT );
        int ain = analogRead( pinCfg[gpioId].portPin ); // a value between 0-4095
        // Filter the adc input to get a stable result
        int prevInput = SUI.LiveConfigs[gpioId].input;
        const int FILTER_SAMPLES = 4;
        SUI.LiveConfigs[gpioId].input = ( prevInput * ( FILTER_SAMPLES - 1 ) + ain ) / FILTER_SAMPLES; // Moving average
        const int SCALE_0_TO_127 = 32;  // Range 0-127. May need adjustment depending on swell pedal shoe geometry
        int prev_result = prevInput / SCALE_0_TO_127; // scale 0-4095 to 0-127 for midi cc command
        int new_result  = SUI.LiveConfigs[gpioId].input / SCALE_0_TO_127; // scale 0-4095 to 0-127 for midi cc command
        int error = abs( SUI.LiveConfigs[gpioId].error - SUI.LiveConfigs[gpioId].input);
        // Avoid hunting between adjacent numbers while allowing min and max scale
        if( new_result != prev_result ) {
           if ( ( error >= ( SCALE_0_TO_127/2 ) ) || ( new_result == 0 ) || ( new_result == 127 ) ) {
            if ( error > SCALE_0_TO_127 * 3 )
              SUI.LiveConfigs[gpioId].input = ain;
            SUI.LiveConfigs[gpioId].error = SUI.LiveConfigs[gpioId].input;
            int midi_channel      = SUI.Cfg.Bits.midiChannelOffset;
            int cc_command_number = SUI.LiveConfigs[gpioId].count + 0x20; // FIXME Midi CC command range
            digitalWrite(LED_BUILTIN, true ); // Indicate USB signalling PCB LED
            midi.sendControlChange(midi_channel,cc_command_number, new_result );
            if ( SUI.Cfg.Bits.hasEventLog ) {
                char buff[80];
                sprintf(buff,"MidiControl Ch=%-2d  Cont=%d  Val=%d  ADC\r\n",midi_channel+1,cc_command_number, new_result );
                CompositeSerial.write(buff);
                SUI.RequestDisplayUpdate();
            }
            digitalWrite(LED_BUILTIN, false );
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
   for ( int gpioId = 0 ; gpioId < NUM_GPIO_PINS ; gpioId++ ) {
     switch ( SUI.LiveConfigs[gpioId].function ) {
      case IP_ADC :
      {
        pinMode (  pinCfg[gpioId].portPin, INPUT);
        SUI.LiveConfigs[gpioId].input = analogRead( pinCfg[gpioId].portPin ); // a value between 0-4095
      }
     }
   }
}

void ADC::Print(void) {
} 
