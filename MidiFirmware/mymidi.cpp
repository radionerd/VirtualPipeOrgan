#include <Arduino.h>
#include <USBComposite.h> // https://github.com/arpruss/USBComposite_stm32f1 Arduino version does not include Sysex support
#include "led.h"
#include "multilcd.h"
#include "mymidi.h"
#include "pin.h"
#include "profile.h"
#include "TM1637.h"
#include "USBSerialUI.h"

extern MultiLCD mlcd;
extern TM1637 SEG7; // 7 Segment display driver
extern PROFILE profile; // execution time profiler

void myMidi::handleNoteOff(unsigned int channel, unsigned int note, unsigned int velocity) {
      //led.on();
      // CompositeSerial.write("myMidiOff\r\n");
      SUI.handleNoteOff( channel,note,velocity );
      //led.off();
}

void myMidi::handleNoteOn( unsigned int channel, unsigned int note, unsigned int velocity ) {
      //led.on(); // Turn on LED briefly to indicate USB input
      // CompositeSerial.write("myMidiOn\r\n");
      SUI.handleNoteOn( channel, note, velocity );
      //led.off();
}

void myMidi::handleControlChange( unsigned int channel, unsigned int controller, unsigned int velocity ) {
      //led.on(); // Turn on LED briefly to indicate USB input
      SUI.handleControlChange( channel, controller, velocity ); 
      //led.off();
}

/*
Hauptwerk LCD panel 2x18 text message format:
Byte 1: 0xF0 – SyEx (system exclusive message start)
Byte 2: 0x7D - manufacturer ID (Milan Digital Audio)
Byte 3: 0x01 - message type code for Hauptwerk LCD output message
Byte 4: 0x00..0x7F - destination panel unique ID LSB (7-bit)
Byte 5: 0x00..0x7F - destination panel unique ID MSB (7-bit)-ignored by hwlcd4
Byte 6: color code 0-127 (0=off/black, 1=white, 2=red, 3=green,
4=yellow, 5=blue, 6=magenta, 7=cyan, others-ignored by hwlcd4)
Bytes 7-38: the 32 ASCII (7-bit) bytes for the text to display
Byte 39: 0xF7 – EOX (end of system exclusive message)
*/ /*
Hauptwerk 16-character string variable status message format:
Byte 1: 0xF0 – SyEx (system exclusive message start)
Byte 2: 0x7D - manufacturer ID (Milan Digital Audio)
Byte 3: 0x10 - message type code for Hauptwerk string variable status message
Byte 4: variable ID (see list below).
Bytes 6-20: 16 ASCII (7-bit) character codes
Byte 21: 0xF7 – EOX (end of system exclusive message) */

void myMidi::handleSysExData(unsigned char ExData) {
    if ( ExData == 0xF0 )
      sysexIndex=0;
    if ( sysexIndex < (sizeof( sysexBuf)+1) ) {
      sysexBuf[sysexIndex++] = ExData;
      sysexBuf[sysexIndex]=0;
    }
    if ( ExData == 0xF7 ) {
      profile.PEnd(PROFILE_MIDI_OUT_TO_SYSEX_IN);
      profile.PStart(PROFILE_SYSEX);
      handleSysExEnd();
      profile.PEnd  (PROFILE_SYSEX);
    }
  }
  
void myMidi::handleSysExEnd(void) {
    const int SysExStartFlag=0xF0;
    const int SysExExperimentalID=0x7D;
    const int SysExLCD32MessageIdentifier=0x01;
    const int SysExLCD16MessageIdentifier=0x19;
    //int startIndex;
    unsigned long time_ref = last_time;
    last_time = micros();
    unsigned long interval;
    interval = last_time - (unsigned long)time_ref ;
    // Accept GrandOrgue 16 or 32 char display messages
    if ( ( sysexBuf[ 0 ] == SysExStartFlag ) && ( sysexBuf[ 1 ] == SysExExperimentalID ) ) { 
      sysexBuf[sysexIndex-1] = 0; // Null terminate string
      int lcd_address = 0;
      //int lcd_colour = 0;
      if (  sysexBuf[ 2 ] == SysExLCD32MessageIdentifier ) {
        // lcd_colour  = sysexBuf[ 3 ]; // Unused feature
        lcd_address = sysexBuf[ 4 ];
        // lcd_address += sysexBuf[5]<<8;
        sysexIndex = 6; // Start of Text
      }
      if (  sysexBuf[ 2 ] == SysExLCD16MessageIdentifier ) {
        lcd_address = sysexBuf[ 3 ];
        sysexIndex = 4; // Start of Text
      }
      int i = 0 ;
      // Add Beats per bar and temp to metronome display if found with space
      while ( sysexBuf[sysexIndex + ++i ]  != 0 ) {
        if ( sysexBuf[sysexIndex + i - 3 ]=='B' ) {
          if ( sysexBuf[sysexIndex + i - 2 ]=='P' ) {
            if ( sysexBuf[sysexIndex + i - 1 ]=='M' ) {
               if ( ( sysexBuf[sysexIndex + i - 0 ] == ' ' ) && ( sysexBuf[sysexIndex + i + 1 ] == ' ' ) ) {
                 strncpy ( sysexBuf + sysexIndex + i - 3 ,"Tempo", 5  );
               }
               i-=5;
               while ( isdigit ( sysexBuf[sysexIndex+i] ) ) --i;
               while ( sysexBuf[sysexIndex+i]==' ' ) --i;
               if ( isdigit ( sysexBuf[sysexIndex+i] ) && sysexBuf[sysexIndex+i+1]==' ' && 
                    sysexBuf[sysexIndex+i+2]==' ' && sysexBuf[sysexIndex+i+3]==' ' && sysexBuf[sysexIndex+i+4]==' ' ) {
                 sysexBuf[sysexIndex+i+2]='B' ;
                 sysexBuf[sysexIndex+i+3]='P' ;
                 sysexBuf[sysexIndex+i+4]='B' ;
               }
               break;
             }
           }
        }
      }
      if ( lcd_address ) {
        if ( lcd_address == SEVEN_SEGMENT_ADDRESS ) {
          SEG7.displayPChar( sysexBuf + sysexIndex );         
        } else {
          //MultiLCD mlcd();
          mlcd.Write(lcd_address, sysexBuf + sysexIndex );
        }
      }
      if ( SUI.Cfg.Bits.hasEventLog ) {
        char buff[80];
        sprintf(buff,"%lu %5lu LCD[%d]='%s'\r\n",last_time,interval,lcd_address,sysexBuf + sysexIndex);
        CompositeSerial.write( buff ) ;
      }
    }
    sysexIndex=0;
    sysexBuf[sysexIndex]=0;
} // handleSysExEnd
