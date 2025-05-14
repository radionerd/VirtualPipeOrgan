#include <Arduino.h>
#include <USBComposite.h> // https://github.com/arpruss/USBComposite_stm32f1 Arduino version does not include Sysex support
#include "led.h"
#include "lcd.h"
#include "mymidi.h"
#include "TM1637.h"
#include "USBSerialUI.h"

extern TM1637 SEG7; // 7 Segment display driver


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
    if ( ExData == 0xF7 )
      handleSysExEnd();
//    char buff[80];
//    if ( ExData < ' ' )
//      ExData = ' ';
//    if ( ExData < 0x80 ) {
//      sprintf(buff,"%c",ExData);
//    } else {
//      sprintf(buff,".\n");  
//    }
//    CompositeSerial.write( buff ) ;
  }
  
void myMidi::handleSysExEnd(void) {
    const int SysExStartFlag=0xF0;
    const int SysExManufacturerID=0x7D;
    const int SysExLCD32MessageIdentifier=0x01;
    const int SysExLCD16MessageIdentifier=0x19;
//  const int SysExLCDNumberMSB=0;
//  const int SysExLCDColourIndex=3; // 1 = White Unused
    const int SysExLCD16TextStartIndex=4;
    const int SysExLCD32TextStartIndex=6;

    unsigned long time_ref = last_time;
    last_time = micros();
    unsigned long interval;
    interval = last_time - (unsigned long)time_ref ;
    if ( sysexBuf[ 0 ] == SysExStartFlag && sysexBuf[ 1 ] == SysExManufacturerID && sysexBuf[ 2 ] == SysExLCD32MessageIdentifier ) { // && sysexBuf[ 4 ] == SysExLCDNumberMSB GrandOrgue work around  ) {
      sysexBuf[sysexIndex-1] = 0; // Null terminate
      sysexIndex=0;
      uint8_t LCDNumberLSB = sysexBuf[3] | sysexBuf[4]; // GrandOrgue & Hauptwerk seem to disagree about MSB and LSB order, so assume MSB==0 for now
      if ( LCDNumberLSB == 0x20 ) {
          SEG7.displayPChar( sysexBuf + SysExLCD32TextStartIndex );
      } else {
        LCD_N_C32 lcd("");
        lcd.write(LCDNumberLSB, sysexBuf + SysExLCD32TextStartIndex );
      }
      if ( SUI.Cfg.Bits.hasEventLog ) {
        char buff[80];
        sprintf(buff,"%lu %5lu LCD[%d]='%s'\r\n",last_time,interval,LCDNumberLSB,sysexBuf + SysExLCD32TextStartIndex);
        CompositeSerial.write( buff ) ;
      }
    }
    if ( sysexBuf[ 0 ] == SysExStartFlag && sysexBuf[ 1 ] == SysExManufacturerID && sysexBuf[ 2 ] == SysExLCD16MessageIdentifier ) { // && sysexBuf[ 4 ] == SysExLCDNumberMSB GrandOrgue work around  ) {
      sysexBuf[sysexIndex-1] = 0; // Null terminate
      sysexIndex=0;
      uint8_t LCDNumberLSB = sysexBuf[3] ;
      if ( LCDNumberLSB == 0x20 ) {
          SEG7.displayPChar( sysexBuf + SysExLCD16TextStartIndex );         
      } else {
        LCD_N_C32 lcd((const char *)"");
//        if ( strncmp ( sysexBuf + SysExLCD16TextStartIndex + 3 , "BPM",3 ) == 0 || strncmp ( sysexBuf + SysExLCD16TextStartIndex + 4 , "BPM",3 )==0 ) {
//          strncpy ( sysexBuf + SysExLCD16TextStartIndex + 12 , "BPB",3 ); // Flag beats per bar / Measure
//        }
        if ( strncmp ( sysexBuf + SysExLCD16TextStartIndex + 10 , "BPM",3 ) == 0 || strncmp ( sysexBuf + SysExLCD16TextStartIndex + 11 , "BPM",3 )==0 ) {
          if ( sysexBuf[ SysExLCD16TextStartIndex+1 ] == ' ' )
            strncpy ( sysexBuf + SysExLCD16TextStartIndex + 2 , "BPB",3 ); // Flag beats per bar
          else
            strncpy ( sysexBuf + SysExLCD16TextStartIndex + 3 , "BPB",3 ); // Flag beats per bar
          if ( sysexBuf[ SysExLCD16TextStartIndex+9 ] == ' ' )          
            strncpy ( sysexBuf + SysExLCD16TextStartIndex + 10 , "Tempo ",6 ); // beats per minnute
          else
            strncpy ( sysexBuf + SysExLCD16TextStartIndex + 11 , "Tempo",5 ); // beats per minnute
          
        }
        lcd.write(LCDNumberLSB, sysexBuf + SysExLCD16TextStartIndex );
      }
      if ( SUI.Cfg.Bits.hasEventLog ) {
        char buff[80];
        sprintf(buff,"%lu %5lu LCD[%d]='%s'\r\n",last_time,interval,LCDNumberLSB,sysexBuf + SysExLCD16TextStartIndex);
        CompositeSerial.write( buff ) ;
      }
    }
    if ( sysexIndex ) {
     if ( SUI.Cfg.Bits.hasEventLog ) {
      if ( sysexBuf[0]==0xF0 ) {
       char buff[80];
       sprintf(buff,"%lu %5lu SysEx: ",last_time , interval );
       CompositeSerial.write( buff );
       //sysexBuf[sysexIndex-1] = 0; // Null terminate
       for ( unsigned int i = 0 ; i < sysexIndex-1 ; i++ ) {
        char c = sysexBuf[i];
        if ( i > 3 && c >= ' ' ) {
          buff[0] = c;
          buff[1]=0;
        } else {
          sprintf( buff, "%02X ",sysexBuf[i]);
        }
        CompositeSerial.write(buff);
       }
       CompositeSerial.write(" Ignored\r\n");    
      }
     }
    }
    sysexIndex=0;
    sysexBuf[sysexIndex]=0;
  } // handleSysExEnd
