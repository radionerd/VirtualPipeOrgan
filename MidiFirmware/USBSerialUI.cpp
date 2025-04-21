/*
#include <BridgeSSLClient.h>
#include <BridgeUdp.h>
#include <HttpClient.h>
#include <Process.h>
#include <Console.h>
#include <YunServer.h>
#include <FileIO.h>
#include <Bridge.h>
#include <BridgeClient.h>
#include <BridgeServer.h>
#include <YunClient.h>
#include <Mailbox.h> */

#include <Arduino.h>
#include <USBComposite.h>
#include <flash_stm32.h>
#include <stdio.h>
#include "lcd.h"
#include "led.h"
#include "mymidi.h"
#include "pin.h"
//#include "profile.h"
#include "buttonscan.h"
#include "keyboardscan.h"
#include "USBSerialUI.h"
// Copyright (C)2023 Richard Jones. MIT License applies
ButtonScan Button;

KeyboardScan MusicKeyboard;

uint32_t kb_input [NUM_KEYBOARD_INPUTS]; // 16 input bits per output line
uint32_t kb_image [NUM_KEYBOARD_INPUTS];
unsigned long kb_time[NUM_KEYBOARD_INPUTS*NUM_KEYBOARD_OUTPUTS]; // For velocity measurment

uint32_t sr_input_list[NUM_SHIFT_REG_OUTPUTS+1];
uint32_t sr_outputs   [NUM_SHIFT_REG_OUTPUTS+1];

USBSerialUI::USBSerialUI(void) {
  RestoreConfigFromFlash();
  TestIO();
}
void USBSerialUI::poll(void) {
  MusicKeyboardScan();
  ButtonScan();
  //GPIOScan();
  char buff[80];
  while (CompositeSerial.available()) {
    char c = CompositeSerial.read();
    CommandCharDecode(c);
    LastCommand = c;
  }
  if ( DisplayUpdate ) { // Refresh monitoring screens
    DisplayUpdate -- ;
    switch ( tolower(LastCommand) ) {
      case 'k' :
      case 'o' :
      case 'z' :
        CommandCharDecode(LastCommand);
      break;
    }
  }
}
int USBSerialUI::midiButtonChannel(void) {
  const int MIDI_BUTTON_OFFSET = 8;
  return ( (Cfg.Bits.midiChannelOffset + MIDI_BUTTON_OFFSET) & 0xF );
}
int USBSerialUI::midiKeyboardChannel(void) {
  return Cfg.Bits.midiChannelOffset;
}

void USBSerialUI::handleNoteOn( unsigned int midi_channel, unsigned int midi_note, unsigned int midi_velocity ) {
  if ( midi_channel == midiButtonChannel() ) {
    led.on(); // Turn on LED briefly to indicate addressed USB input
    monitorNoteOn   ( midi_channel, midi_note, midi_velocity , DEV_LED );
    if ( midi_note < NUM_SHIFT_REG_OUTPUTS ) {
      sr_outputs[midi_note] = 1;
    }
    led.off();
  }
  AppOnlineTime = micros();
}
void USBSerialUI::handleNoteOff( unsigned int midi_channel, unsigned int midi_note, unsigned int midi_velocity ) {
  if ( midi_channel == midiButtonChannel() ) {
    led.on(); // Turn on LED briefly to indicate addressed USB input
    monitorNoteOff   ( midi_channel, midi_note, midi_velocity, DEV_LED );
    if ( midi_note < NUM_SHIFT_REG_OUTPUTS ) {
      sr_outputs[midi_note] = 0;
    }
    led.on(); // Turn on LED briefly to indicate addressed USB input
  }
  AppOnlineTime = micros();
}

void  USBSerialUI::handleControlChange( unsigned int midi_channel, unsigned int controller, unsigned int velocity ) {
  char buf[80];
  if ( midi_channel == midiKeyboardChannel() ) {
    led.on(); // Turn on LED briefly to indicate addressed USB input
    sprintf(buf,"Unhandled Control Change channel=%d controller=%d velocity=%d\r\n",midi_channel,controller,velocity);
    CompositeSerial.write(buf);
    led.off();
  }
}


void USBSerialUI::ButtonScan(void) {

  static unsigned long last_time=0;
  unsigned long time_now = micros();
  if ( time_now - last_time < 20000 ) return;
  last_time += 20000; // 20ms 50 times per second update rate
  const int midi_velocity = 64;
  const unsigned long AUTO_REPEAT_DELAY_US = 2000000; // 2 seconds
  const unsigned long AUTO_REPEAT_PERIOD_US = 333000; // 0.333 seconds

  static int active_buttons=0;
  int result = Button.Scan( sr_input_list,sr_outputs );
  // NB The strange sr_input_list interface yields a fast inactive scan result
  if ( ( active_buttons != result ) || active_buttons ) { 
    active_buttons = result;
//    if ( active_buttons ) { // removed for auto repeat
      for ( int j = 0 ; j < active_buttons ; j++ ) {
        // Flag all new active buttons
        if ( ShiftRegImage[sr_input_list[j] ].Input == 0 ) {
          ShiftRegImage[ sr_input_list[j] ].Input = 2; // Flag just becoming active
          ShiftRegImage[ sr_input_list[j] ].Time = time_now; // record time for auto repeat
          if ( AppOnlineTime == 0 ) {
            sr_outputs[  sr_input_list[j] ] = 1; // Button test when not online
          }
          //ShiftRegImage[ sr_input_list[j] ].Output = ! ShiftRegImage[ sr_input_list[j] ].Output; // DEBUG Toggle outputs
          int midi_note = sr_input_list[j];
          if( ShiftRegImage[ 0 ].Input && ( midi_note != 0 ) ) { // Shifted key?
              midi_note += NUM_SHIFT_REG_OUTPUTS ;
          }
          int midi_channel = midiButtonChannel() ;
          midi.sendNoteOn ( midi_channel, midi_note, midi_velocity );
          monitorNoteOn   ( midi_channel, midi_note, midi_velocity, DEV_BUTTON  );
          RequestDisplayUpdate();
        } else {
          ShiftRegImage[ sr_input_list[j] ].Input = 2; // Flag is active
          unsigned long elapsed_time_us = time_now - ShiftRegImage[ sr_input_list[j] ].Time;
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
            int midi_channel = midiButtonChannel() ;
            midi.sendNoteOn ( midi_channel, midi_note, midi_velocity );
            monitorNoteOn   ( midi_channel, midi_note, midi_velocity, DEV_BUTTON  );
            RequestDisplayUpdate();
          }
        }
        // sr_outputs[sr_input_list[j]] = ShiftRegImage[sr_input_list[j]].Output; // DEBUG without GrandOrgue Update LED displays
      }
//    }
    for ( int i = 0 ; i < NUM_SHIFT_REG_OUTPUTS ; i++ ) {
      if ( ShiftRegImage[ i ].Input ) {
        if ( -- ShiftRegImage[ i ].Input == 0 ) { // Detect just going inactive
          // Send OFF to MIDI
          int midi_channel = midiButtonChannel() ;
          int midi_note = i ;
          if ( AppOnlineTime == 0 ) {
            sr_outputs[ midi_note ] = 0; // Button test when not online
          }
          if( ShiftRegImage[ 0 ].Input && ( midi_note != 0 ) ) { // Shifted key?
              midi_note += NUM_SHIFT_REG_OUTPUTS ;
          }
          midi.sendNoteOff ( midi_channel, midi_note, midi_velocity );
          monitorNoteOff   ( midi_channel, midi_note, midi_velocity, DEV_BUTTON  );
          RequestDisplayUpdate();

        }
      }
    }
  }
}

void USBSerialUI::MusicKeyboardScan(void) {
  char buf[120];
  const char *note_name[12] = {
    "C","C♯","D","D♯","E","F","F♯","G","G♯","A","A♯","B"
  };
  const int OCTAVE = 12;
  int note_id=-1;
  int change = MusicKeyboard.Scan(kb_input,kb_image);
  if ( change ) {
    // process MusicKeyboard changes
    //sprintf(buf,"Keyboard change = %d\r\n",change);
    //CompositeSerial.write(buf);    
    for ( int i = 0 ; i < NUM_KEYBOARD_OUTPUTS ; i++ ) {
      int difference = kb_input [i] ^ kb_image[i];
      if ( difference ) {
        RequestDisplayUpdate();
        for ( int j = 0 ; j < NUM_KEYBOARD_INPUTS; j++ ) {
          if ( difference & ( 1 << j ) ) {
            int index = i*NUM_KEYBOARD_INPUTS + j ;
            long int key_travel_time;
            kb_time[index] = micros();
            if (  j & 1 ) { // top of key travel?
              key_travel_time = kb_time[index] - kb_time[index-1];
            } else {
              key_travel_time = kb_time[index] - kb_time[index+1];              
            }
            //sprintf(buf,"Key travel time = %ldus\r\n",key_travel_time);
            //CompositeSerial.write(buf);
            // Send to midi
            int midi_note = 36 + i + ( (j>>1) * 8 );
           
            int midi_velocity = 1 ;
            if ( key_travel_time < 100000 ) {
              midi_velocity = (100000-key_travel_time)/787;
            }
            buf[0]=0;
            int midi_channel = Cfg.Bits.midiChannelOffset ;
            if ( kb_input [i] & ( 1 << j ) ) { // contact just closed?
              if ( ( j & 1 ) == 0 ) { // lower contact?
                midi.sendNoteOn ( midi_channel, midi_note, midi_velocity );
                monitorNoteOn   ( midi_channel, midi_note, midi_velocity, DEV_KEYBOARD  );
                // sprintf(buf,"Midi Note =%d ON  Velocity=%d Name=%2s i=%d j=%d diff=%02X time=%ld\r\n",note,velocity,note_name[note%OCTAVE],i,j,difference,key_travel_time);
              }
            } else { // contact just opened
              if ( j & 1 ) { // upper contact?
                midi.sendNoteOff ( midi_channel, midi_note, midi_velocity );
                monitorNoteOff   ( midi_channel, midi_note, midi_velocity, DEV_KEYBOARD  );
                // sprintf(buf,"Midi Note =%d OFF Velocity=%d Name=%2s i=%d j=%d diff=%02X time=%ld\r\n",note,velocity,note_name[note%OCTAVE],i,j,difference,key_travel_time);
              }
            }
            if ( buf[0]) {
              CompositeSerial.write(buf);    
            }
          }
        }
      }
      kb_image[i] = kb_input[i]; // inform driver
    }
  }
}



void USBSerialUI::CommandCharDecode( char c )
{
    CompositeSerial.write("\r\n");
    switch ( tolower(c) ) {
      case 'y' :
      case 's' :
        SaveConfigToFlash(c);
        RestoreConfigFromFlash(); // Update LiveConfigs 
        break;
      case 'r' :// restore config from flash
        RestoreConfigFromFlash();
        DisplayFunctionPinOut();
        DisplayConfigurationMenu();
        break;
      case 't' :
        TestIO();
        break;  
      case 'a' :
      case 'b' :
      case 'c' : // Configuration
      case 'd' :
      case 'e' :
      case 'f' :
      case 'g' :
      case 'h' :
      case 'i' :
      case 'j' :
      case ' ' :
        {
          ConfigValue [0] = Cfg.Bits.midiChannelOffset;
          ConfigValue [1] = Cfg.Bits.numberADCInputs;
          // Cfg.Bits.numberOfKeyboards = ConfigValue [2];
          ConfigValue [2] = Cfg.Bits.hasPedalBoard;
          ConfigValue [3] = Cfg.Bits.hasI2C       ;
          ConfigValue [4] = Cfg.Bits.hasTM1637    ;
          ConfigValue [5] = Cfg.Bits.hasShiftRegs ;
          ConfigValue [6] = Cfg.Bits.hasShiftRegLEDInvert;
          ConfigValue [7] = Cfg.Bits.hasPC13Scan  ; // I
          ConfigValue [8] = Cfg.Bits.hasPB2PA13PA14Scan;
          ConfigValue [9] = Cfg.Bits.hasEventLog  ; // K
          int i = 0;
          if ( c != ' ' ) {
            // Adjust configured value by index for display
            i = ( ( c - 'a' ) & 0xf ) ;
            if ( c == tolower(c) ) {
              ConfigValue[i] ++ ;
              if ( ConfigValue[i] > ConfigItems[i].maxval )
                ConfigValue[i] = 0;
            } else {
              ConfigValue[i]--;
              if (ConfigValue[i] < 0 )
                ConfigValue[i] = ConfigItems[i].maxval;
            }
            Cfg.Bits.midiChannelOffset = ConfigValue [0];
            Cfg.Bits.numberADCInputs   = ConfigValue [1];
            Cfg.Bits.hasPedalBoard     = ConfigValue [2];
            Cfg.Bits.hasI2C            = ConfigValue [3];
            Cfg.Bits.hasTM1637         = ConfigValue [4];
            Cfg.Bits.hasShiftRegs      = ConfigValue [5];
            Cfg.Bits.hasShiftRegLEDInvert = ConfigValue[6]; // G
            Cfg.Bits.hasPC13Scan       = ConfigValue [7]; // H
            Cfg.Bits.hasPB2PA13PA14Scan= ConfigValue [8]; // I
            Cfg.Bits.hasEventLog       = ConfigValue [9]; // J
          }
          DisplayFunctionPinOut();
          DisplayConfigurationMenu();
        }
        break;
      case 'k' :
        DisplayKeyboardContacts() ;
        break;
      case 'l' : // LCD test
        LCDTest();
        break;
      case 'm' :
        DisplayFlashSummary() ;
        break;
      case 'o' :
        DisplayShiftRegisterIO();
        break;
      case 'v' :
        {
          char buff[80];
          int i = strlen(__FILE__);
          while ( --i ) {
            if ( __FILE__[i - 1] == '/' ) break; // strip lengthy file path
          }
          sprintf ( buff , "%s %s %s %s\r\n", &__FILE__[i] , "V1.0.0" , __DATE__, __TIME__);
          CompositeSerial.write(buff);
        }
        break;
      //case 'p' :
      //  sprintf(buff,"Profile max duration=%ldus '%s'",profile.GetMaxDuration(),profile.GetMaxMessage() );
      //  CompositeSerial.println(buff);
      //  break;
      case 'z' :
        DisplayStatus();/*
        for ( int i = 0 ; i < 8 ; i++ ) {
          sprintf(buff, "ADC[%d] = 0x%03X\r\n", i, analogRead( PA0 + i ) ); // DEBUG Fix for up to ten inputs, read Port pin from GPIO array
          CompositeSerial.write(buff);
        }*/
        break;

      default:
        DisplayMenu();
    }
}


void USBSerialUI::DisplayFlashSummary(void) {
  int lastfs = 0;
  for ( uint16* address = (uint16*)0x8000000 ; address < (uint16*)0x801FFE0; address += 8 ) {
    int allfs = 0;
    for ( int i = 8 ; i < 16 ; i++ )
      if ( address[i] == 0xffff )
        allfs ++;
    if ( ( allfs == 8 && lastfs != 8 ) || ( allfs != 8 && lastfs == 8 ) ) {
      char buff[80] ;
      sprintf ( buff , "%08X %04X %04X %04X %04X %04X %04X %04X %04X\r\n",
                &address[0],
                address[0x0], address[0x1], address[0x2], address[0x3],
                address[0x4], address[0x5], address[0x6], address[0x7] );
      CompositeSerial.write(buff);
      address += 8;
      sprintf ( buff , "%08X %04X %04X %04X %04X %04X %04X %04X %04X\r\n",
                &address[0],
                address[0x0], address[0x1], address[0x2], address[0x3],
                address[0x4], address[0x5], address[0x6], address[0x7] );
      CompositeSerial.write(buff);
      address -= 8;
      CompositeSerial.write("...\r\n");
    }
    lastfs = allfs;
  }
}

const uint16_t *FlashPage = (uint16_t *)0x0801F800;
const uint16_t FlashSignature = 0xdead;
const int FlashPageSize = 0x200;
void USBSerialUI::SaveConfigToFlash( char c ) {
  char buff[80];
  int estatus;
  int pstatus;
  FLASH_Unlock();
  if ( ( FlashPage[0] != FlashSignature ) || ( FlashPage[ FlashPageSize - 1] != 0xFFFF ) ) {
    if ( tolower(c) == 'y' ) {
      //EraseFlashPage( FlashPage );
      estatus = FLASH_ErasePage((uint32)FlashPage);
      //WriteFlash( &FlashPage[0], FlashSignature );
      pstatus = FLASH_ProgramHalfWord( (uint32)FlashPage, FlashSignature );
      sprintf( buff, "\r\nEraseStatus=%d ProgramStatus=%d\r\n", estatus, pstatus );
      CompositeSerial.write(buff );
    } else {
      DisplayFlashSummary();
      CompositeSerial.write("Need to erase flash OK?: Y/n" );
      return;
    }
  }
  if ( FlashPage[0] == FlashSignature ) {
    int i = 0;
    while ( FlashPage[++i] != 0xFFFF ) ;
    if ( FlashPage[i - 1] != Cfg.Word  ) {
      sprintf( buff, "Flash Write %08X=%04X\r\n", (uint32)FlashPage + i * 2, Cfg.Word );
      CompositeSerial.write(buff);
      pstatus = FLASH_ProgramHalfWord( (uint32)FlashPage + i * 2, Cfg.Word );
      sprintf( buff, "Flash Program Status=%d\r\n", pstatus );
      CompositeSerial.write(buff);
    } else {
      CompositeSerial.write("Same Value, not written\r\n");
    }
  } else {
    sprintf( buff, "Midi Flash Signature Not Found\r\n" );
    CompositeSerial.write(buff);
  }
}

unsigned USBSerialUI::getFlashConfig(void) {
  int i = 0;
  Cfg.Word = 0;
  if ( FlashPage[0] == FlashSignature ) {
    while ( FlashPage[++i] != 0xFFFF ) ;
    Cfg.Word = FlashPage[i - 1];
  }
  return Cfg.Word;
}
// Set config defaults
// Flash signature present read latest config
void USBSerialUI::RestoreConfigFromFlash() {
  int i = 0;
  int oldCfg = Cfg.Word;
  Cfg.Word = 0;
  if ( FlashPage[0] == FlashSignature ) {
    while ( FlashPage[++i] != 0xFFFF ) ;
    Cfg.Word = FlashPage[i - 1];
  }
  fillCfgPinData( Cfg.Word , &LiveConfigs[0] );
}

void USBSerialUI::DisplayTitle( char *title ) {
  char buff[80];
  sprintf(buff, "%s%s%s%s\r\n", VT100_UNDERLINE, title, VT100_NO_UNDERLINE, VT100_CLR_EOL );
  CompositeSerial.write( buff );
}

void USBSerialUI::DisplayPrompt(char *prompt ) {
  CompositeSerial.write(prompt);
  CompositeSerial.write(VT100_ERASE_DOWN );
}
void USBSerialUI::DisplayConfigurationMenu() {
  int i;
  char buff[80];

  ConfigValue [0] = Cfg.Bits.midiChannelOffset;
  ConfigValue [1] = Cfg.Bits.numberADCInputs;
  ConfigValue [2] = Cfg.Bits.hasPedalBoard;
  ConfigValue [3] = Cfg.Bits.hasI2C       ;
  ConfigValue [4] = Cfg.Bits.hasTM1637    ;
  ConfigValue [5] = Cfg.Bits.hasShiftRegs ;
  ConfigValue [6] = Cfg.Bits.hasShiftRegLEDInvert;
  ConfigValue [7] = Cfg.Bits.hasPC13Scan  ; // I
  ConfigValue [8] = Cfg.Bits.hasPB2PA13PA14Scan;
  ConfigValue [9] = Cfg.Bits.hasEventLog  ; // K

  DisplayTitle("Configuration Menu");
  i = 0;
  while ( ConfigItems[i].maxval != 0 ) {
    char valtext[20];
    if ( ConfigItems[i].maxval > 1 ) {
      sprintf ( valtext, "%d", ConfigValue[i]);
    } else {
      strncpy(valtext, "✓", 5);
      if ( ( ConfigValue[i] ) == 0 ) {
        valtext[0] = ' ' ; // '✓';
        valtext[1] = 0 ;
      }
    }
    if ( i == 0 )
      sprintf(buff, "%c [%d] %s, Button channel=%d%s\r\n", 'A' + i,midiKeyboardChannel() + 1, ConfigItems[i].text,midiButtonChannel() + 1, VT100_CLR_EOL );
    else
      sprintf(buff, "%c [%s]=%s%s\r\n", 'A' + i, valtext, ConfigItems[i].text, VT100_CLR_EOL );    
    CompositeSerial.write(buff);
    i++;
  }
  sprintf(buff, "Cfg.Word=%04X%s\r\n", Cfg.Word, VT100_CLR_EOL);
  CompositeSerial.write(buff);
  DisplayPrompt("Enter A-L,a-l To adjust configured value. S to Save:");
}

void USBSerialUI::DisplayFunctionPinOut(void) {
  int i;
  char buff[80];

  fillCfgPinData( Cfg.Word , &MenuConfigs[0] );
  sprintf(buff, "%s%sSTM32 Blue Pill Assigned Pin Functions%s%s\r\n", VT100_CURSOR_00, VT100_UNDERLINE, VT100_NO_UNDERLINE, VT100_CLR_EOL );
  CompositeSerial.write( buff ); // VT100 Cursor position 0,0
  char function_text1[40];
  char function_text2[40];

  for ( i = 0 ; i < 20 ; i++ )
  {
    getFunctionText( &MenuConfigs[ i + 20], &function_text1[0], 20 );
    getFunctionText( &MenuConfigs[19 -  i], &function_text2[0], 20 );
    sprintf(buff, "%-13s%c%4s         %-4s%c  %-s%s\r\n", function_text1, LiveConfigs[i+20].fault, pinCfg[i + 20].description, pinCfg[19 - i].description,LiveConfigs[19-i].fault, function_text2, VT100_CLR_EOL );
    if ( i == 0 )
      strncpy( &buff[21], "USB", 3);
    if ( i == 19 )
      strncpy( &buff[19], "| | | |", 7);
    CompositeSerial.write(buff);
  }
  function_text1[0]=0;
  function_text2[0]=0;
  for ( i = 0 ; i < NUM_GPIO_PINS ; i ++ )
  {
    if ( LiveConfigs[i].fault == '*' ) {
      sprintf(function_text1,"* = Fault");
    }
    if ( LiveConfigs[i].fault == '!' ) {
      sprintf(function_text2,"        ! = Add 4k7 Pullup");    
    }
  }
  i = NUM_GPIO_PINS-3;
  sprintf(buff, "%-19s| | | |%s%s\r\n",function_text1,function_text2, VT100_CLR_EOL);
  CompositeSerial.write(buff);
  getFunctionText(&MenuConfigs[i], function_text1, 20 );
  getFunctionText(&MenuConfigs[i+1], function_text2, 20 );
  sprintf(buff, "%-13s%c%-4s---+ +---%s%c  %s%s\r\n", function_text1,LiveConfigs[i].fault, pinCfg[i].description, pinCfg[i+1].description,LiveConfigs[i+1].fault,function_text2, VT100_CLR_EOL );
  CompositeSerial.write(buff);
  getFunctionText(&MenuConfigs[i+2], function_text1, 20 );
  sprintf(buff, "%-13s%c%-4s (on Jumper via 100K resistor)%s\r\n", function_text1,LiveConfigs[i+2].fault, pinCfg[i+2].description, VT100_CLR_EOL );
  CompositeSerial.write(buff);
}

char * USBSerialUI::getFunctionText(GPIOPinConfig *pinCfg, char *buff, int n ) {
  int i = 0 ;
  int pin_function = pinCfg->function;
  int count = pinCfg->count;
  int kbd = pinCfg->keyboard;
  
  strncpy( buff, function_text[pin_function], n );
  if ( pin_function == IP_ADC || pin_function == IP_SCAN ) {
    sprintf( buff + strlen(buff), "%2d", count);
  } 
  if ( pin_function == OP_SCAN ) {
    if ( kbd == 0 ) {
      sprintf ( buff , "Pedalboard-%c", 'a' + count );
    } else {
      sprintf ( buff , "Keyboard%d-%c", kbd, 'a' + count );
    }
  }
  return buff;
}

void USBSerialUI::fillCfgPinData( unsigned ConfigWord, GPIOPinConfig * newCfgs ) {
  const int maxopcount[5] = { 3,5,5,5,5 } ; // max number of scan outputs per pedalboard or keyboard
  int adc_count = 1; // Offset by 1 for display
  int op_count  = 0;
  int ip_count  = 0;
  int keyboard  = 0;
  typedef union {
    unsigned Word;

    struct {
      unsigned midiChannelOffset : 4; // A  4
      unsigned numberADCInputs : 4;   // B  8
      unsigned hasPedalBoard : 1;     // C  9
      unsigned hasI2C: 1;             // D 10
      unsigned hasTM1637 : 1;         // E 11
      unsigned hasShiftRegs: 1;       // F 12
      unsigned hasShiftRegLEDInvert: 1; // G 13
      unsigned hasPC13Scan: 1;        // H 14
      unsigned hasPB2PA13PA14Scan: 1; // I 15
      unsigned hasEventLog: 1;        // J 16
    } Bits;
  } Config1; // Redefined here to work around 'not declared in this scope' error
  Config1 tcfg;
  tcfg.Word = ConfigWord;
  for ( int gpio_index = 0 ; gpio_index < NUM_GPIO_PINS ; gpio_index ++ ) {
    newCfgs[gpio_index].function = pinCfg[gpio_index].function;
    newCfgs[gpio_index].count = 0;
    newCfgs[gpio_index].keyboard = 0;    
    newCfgs[gpio_index].error = 0;
    newCfgs[gpio_index].fault = ' ';
    if ( op_count >= maxopcount [ keyboard ] ) {
      op_count = 0;
      keyboard+=1;
    }
    switch ( pinCfg[gpio_index].function ) {
      case OP_LED :
        if ( tcfg.Bits.hasPC13Scan == 0 ) {
          break;
        }
      case OP_SCAN :
        newCfgs[gpio_index].function = OP_SCAN;
        if ( ( keyboard == 0 ) && ( op_count == 0 ) ) { // Adjust count for pedalboard ( or not)
          if (tcfg.Bits.hasPedalBoard == 0 ) {
            op_count = 2; // skip first two pedalboard octaves, use final part octave for top note of each keyboard
          }
        }
        newCfgs[gpio_index].count    = op_count++;
        newCfgs[gpio_index].keyboard = keyboard;            
      break;
      case IP_SCAN :
        newCfgs[gpio_index].count    = ip_count++;
        break;
      case IP_ADC :
        if ( adc_count >  tcfg.Bits.numberADCInputs ) {
          newCfgs[gpio_index].function = OP_SCAN;
          newCfgs[gpio_index].count    = op_count++;
          newCfgs[gpio_index].keyboard = keyboard;            
        } else {
          newCfgs[gpio_index].count    = adc_count++;
        }
        break;
      case I2C_SCL :
        if ( tcfg.Bits.hasI2C == 0 ) {
          newCfgs[gpio_index].function = OP_SCAN;
          newCfgs[gpio_index].count    = op_count++;
          newCfgs[gpio_index].keyboard = keyboard;            
        }
        break;
      case TM1637_CK :
        if ( tcfg.Bits.hasTM1637 == 0 ) {
          newCfgs[gpio_index].function = OP_SCAN;
          newCfgs[gpio_index].count    = op_count++;
          newCfgs[gpio_index].keyboard = keyboard;            
        }
        break;
      case IP_SR_DATA :
      case OP_SR_CLOCK :
        if ( tcfg.Bits.hasShiftRegs == 0 ) {
          newCfgs[gpio_index].function = OP_SCAN;
          newCfgs[gpio_index].count    = op_count++;
          newCfgs[gpio_index].keyboard = keyboard;            
        }
        break;
      case I2C_SDA :
      case COMMON_DATA :
        if ( ( Cfg.Bits.hasI2C | Cfg.Bits.hasTM1637 | Cfg.Bits.hasShiftRegs ) == 0 ) {
          newCfgs[gpio_index].function = OP_SCAN;
          newCfgs[gpio_index].count    = op_count++;
          newCfgs[gpio_index].keyboard = keyboard;            
        }
        break;
      case BOOT1 :
      case SWCLK :
      case SWDIO :
        if ( Cfg.Bits.hasPB2PA13PA14Scan ) {
          newCfgs[gpio_index].function = OP_SCAN;
          newCfgs[gpio_index].count    = op_count++;
          newCfgs[gpio_index].keyboard = keyboard;            
        }
        break;
    }
  }
}


const char *NoteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
void USBSerialUI::DisplayKeyboardContacts() {
  char buf[80];
  char * op_names[] = {
    "CN13 PIN4",
    "CN13 PIN3",
    "CN13 PIN2",
    "CN13 PIN1",
    "CN14 PIN4",
    "CN14 PIN3",
    "CN14 PIN2",
    "CN14 PIN1"
  };
  char *note_name[] = {
    "C","C♯","D","D♯","E","F","F♯","G","G♯","A","A♯","B"
  };
  int note_id=-1;
  //CompositeSerial.write(VT100_CLEAR);
  sprintf(buf, "Keyboard Scan Result Midi Channel %d\r\n", 1 + ((5 + Cfg.Bits.midiChannelOffset) & 0xF) );
  DisplayTitle( buf );

  for ( int i = 0 ; i < NUM_KEYBOARD_OUTPUTS ; i++ ) {
    for ( int j = 0 ; j < NUM_KEYBOARD_INPUTS; j++ ) {
      buf[NUM_KEYBOARD_INPUTS-1-j] = '0';
      if ( kb_input[i] & ( 1 << j ) ) {
        buf[NUM_KEYBOARD_INPUTS-1-j] = '1';
        // display lowest note
        int new_note  = (((j/2)*8 )+i);
        if ( note_id == -1 ) {
          note_id = new_note;
        }
        if ( note_id > new_note ) {
          note_id = new_note;
        }
      }
    }
    sprintf(buf+NUM_KEYBOARD_INPUTS," OP%d  %s\r\n",i,op_names[i]);
    CompositeSerial.write(buf);
  }
  CompositeSerial.write("........******** IP *=CN13 input pins 5-6,7=nc,8-13\r\n");
  CompositeSerial.write("********........ IP *=CN14 input pins 5-12\r\n");
}
void USBSerialUI::DisplayMenu(void) {
  CompositeSerial.write(VT100_CLEAR);
  DisplayTitle("Midi Interface Menu" );
  CompositeSerial.write("A-J Adjust config value\r\n");
  CompositeSerial.write("K - Keyboard Contacts\r\n");
  CompositeSerial.write("L - LCD check\r\n");
  CompositeSerial.write("M - Flash memory summary\r\n");
  CompositeSerial.write("O - Shift Register IO\r\n");
  CompositeSerial.write("P - Profile\r\n"); // show max execution time
  CompositeSerial.write("S - Save New Configuration\r\n");
  CompositeSerial.write("T - Test IO Pins\r\n");
  CompositeSerial.write("R - Restore Configuration\r\n");
  CompositeSerial.write("V - Version Info\r\n");
  CompositeSerial.write("Z - Display Status\r\n");
}

void USBSerialUI::DisplayShiftRegisterIO(void) {
  char buff[80];
  
  CompositeSerial.write(VT100_CLEAR);
  sprintf(buff, "Shift Register Midi Channel %d Note Numbers", 1+midiButtonChannel() );
  DisplayTitle( buff );
  for ( int sr = 0 ; sr < NUM_SHIFT_REGS ; sr++ ) {
    sprintf(buff, "\r\nSR%2d  ", 1 + sr );
    for ( int i = 0 ; i < SHIFT_REG_SIZE ; i++ ) {
      char ip = ' ';
      char op = ' ';
      if ( ShiftRegImage[sr*8+i].Input ) ip='i';
      // if ( ShiftRegImage[sr*8+i].Output) op='o';
      if ( sr_outputs[sr*8+i] ) op='o';
      sprintf(buff + strlen(buff), "%c%02d%c ", ip, sr * 8 + i, op );
    }
    CompositeSerial.write(buff);
  }
  CompositeSerial.write("\r\n\ni = input on, o = output on");
}

void USBSerialUI::LCDTest()
{
  char buff[80];
  static int test_count = 0;
  sprintf(buff, "LCD Check %6d    \r\n", test_count++);
  CompositeSerial.write(buff);
  //delay(1000);
  //LCD_N_C32 lcd(""); // Scan i2c bus to discover any displays & write to them
  //               "0123456789abcdef0123456789abcdef
  //lcd.write(0x3f,"Address0x3FLine1Line2 NoScanTest" );
  LCD_N_C32 lcd1(buff); // Scan i2c bus to discover any displays & write to them
  //CompositeSerial.write("LCD Check Return\r\n");
}

void USBSerialUI::TestIO()
{
  char fail = '*';
  char pass = ' ';
  char result;
  int failures=0;
  for ( int i = 0 ; i < NUM_GPIO_PINS ; i++ ) {
    result = pass;
    if (pinCfg[i].description[0] == 'P' && function_text[pinCfg[i].function][0] !='U' ) { // test port pins excluding USB
      pinMode(pinCfg[i].portPin, OUTPUT );
      digitalWrite(pinCfg[i].portPin,0);
      if ( digitalRead (pinCfg[i].portPin) != 0 ) {
        result = fail;
        failures++;
      }
      digitalWrite(pinCfg[i].portPin,1);
      if ( digitalRead (pinCfg[i].portPin) == 0 ) {
        result = fail;
        failures++;
      }
      LiveConfigs[i].fault = result;
      pinMode(pinCfg[i].portPin, INPUT );
    }
  }
  pinMode(LED_BUILTIN , OUTPUT);
}

void USBSerialUI::GPIOScan(void) {
  static int ip_scan_min = 0;
  static int ip_scan_max = 0;
  static int op_sr_clock = 0;
  static int op_sr_data  = 0;
  static int num_sr_clocks = 0;

  for ( int op_id = 0 ; op_id < NUM_GPIO_PINS ; op_id ++ ) {
    switch ( LiveConfigs[op_id].function ) {
      case OP_LED : break;
      case OP_SCAN : // 32 note Pedalboard and 61 note keyboards
        if (ip_scan_min && ip_scan_max ) {
          digitalWrite( pinCfg[op_id].portPin , HIGH );
          pinMode     ( pinCfg[op_id].portPin , OUTPUT);
          int active_ip_id=0;
          int mask = 1;
          for ( int ip_id = ip_scan_min ; ip_id <=ip_scan_max ; ip_id++ ) {
            if ( LiveConfigs[ip_id].function == IP_SCAN ) {
              int input = 0;
              if ( digitalRead( pinCfg[ip_id].portPin ) ) {
                input = mask ;
                active_ip_id = ip_id;             
                LiveConfigs[ip_id].input |= ( 1 << LiveConfigs[op_id].count );
              } else {
                LiveConfigs[ip_id].input &= ( ( 1 << LiveConfigs[op_id].count ) ^ 0xffff );
              }
              int last_input = LiveConfigs[op_id].input & mask ; // 12 inputs recorded on output scanline
              if ( input != last_input ) {
                LiveConfigs[op_id].input ^= mask;
                int midi_note = 24 + LiveConfigs[ip_id].count + LiveConfigs[op_id].count * Octave ;
                int midi_channel = LiveConfigs[ip_id].keyboard + Cfg.Bits.midiChannelOffset ; // FIXME should read flash cfg
                if ( ( LiveConfigs[ip_id].keyboard == 0 ) && ( midi_note > 55 ) ) { // Map last 4 pedalboard contacts to keyboards 1-4 note 84
                  midi_channel += midi_note - 55 ;
                  midi_note = 84;
                }
                led.on();
                if ( input ) {
                   midi.sendNoteOn ( midi_channel, midi_note, 64 );
                   monitorNoteOn   ( midi_channel, midi_note, 64 , DEV_KEYBOARD );
                } else {
                   midi.sendNoteOff ( midi_channel, midi_note,  0 );
                   monitorNoteOff   ( midi_channel, midi_note,  0 , DEV_KEYBOARD);
                }
                char buff[80];
                sprintf(buff,"op=%d,ip=%2d,val[%2d]=%d\r\n",
                  pinCfg[op_id].ccCommand,
                  pinCfg[ip_id].ccCommand,
                  pinCfg[op_id].ccCommand + ( pinCfg[ip_id].ccCommand&0xFC ),
                  pinCfg[ip_id].ccCommand&0x3
                  );
                CompositeSerial.write(buff);
                RequestDisplayUpdate();
                led.off();
              }
              mask = mask << 1;
            }
          }
          digitalWrite(pinCfg[op_id].portPin , LOW );
          if ( active_ip_id ) // Delay next output until input line has decayed to zero
            for ( int delay = 0 ; delay < Octave ; delay++ )
              if( digitalRead( pinCfg[active_ip_id].portPin )==0 )
                break;
        }
      break;
      case IP_SCAN :
        pinMode( pinCfg[op_id].portPin , INPUT_PULLDOWN );
        if ( ip_scan_min == 0 )
          ip_scan_min = op_id;
        if ( ip_scan_max < op_id )
          ip_scan_max = op_id;
      break;
      case IP_ADC :
      break;
      case  OP_SR_CLOCK :
        op_sr_clock   = pinCfg[op_id].portPin;
        num_sr_clocks = pinCfg[op_id].ccCommand;        
        pinMode(op_sr_clock,OUTPUT);
        break;
      case OP_SR_DATA :
      case COMMON_DATA :
        op_sr_data = pinCfg[op_id].portPin;
        pinMode(op_sr_data,OUTPUT);
        break;
      case IP_SR_DATA : // Scan button inputs and load LED outputs
        volatile uint32_t* PB12_BB = (volatile uint32_t*) (0x42000000 + (0x10c0C<<5) + (12<<2) ); // STM32 Cortex M3 Bit Band address for Port B bit 12
        volatile uint32_t* PB11_BB = (volatile uint32_t*) (0x42000000 + (0x10c0C<<5) + (11<<2) ); // STM32 Cortex M3 Bit Band address for Port B bit 11
        volatile uint32_t* PB07_BB = (volatile uint32_t*) (0x42000000 + (0x10c0C<<5) + ( 7<<2) ); // STM32 Cortex M3 Bit Band address for Port B bit 07
        static int sr_skip = 0;
        int scanId=0; // Used with multiple shift regisiter blocks
        if ( sr_skip++ >= 100 ) { // Perform shift registerscan at modest rate
          sr_skip = 0;
          // Triple scan of 96 shift register outputs measured as 740us without any optimisation
          int ip_sr_data = pinCfg[op_id].portPin;
          pinMode(ip_sr_data,INPUT_PULLDOWN);
          if ( ip_sr_data && op_sr_data && op_sr_clock ) { // All 3 connections present?
            // clear all shift register outputs
            //digitalWrite( op_sr_data, LOW );
            int i = NUM_SHIFT_REGISTER_BITS;
            *PB07_BB = 0;
            for ( int i = 0 ; i <  NUM_SHIFT_REGISTER_BITS/8 ; i++ ) { // NB: for loop adds 200us, 1.64us clock period, 700us clock hi
              //digitalWrite( op_sr_clock, HIGH );
              //digitalWrite( op_sr_clock, LOW );
              *PB11_BB = 1;
              *PB11_BB = 0;
              *PB11_BB = 1;
              *PB11_BB = 0;
              *PB11_BB = 1;
              *PB11_BB = 0;
              *PB11_BB = 1;
              *PB11_BB = 0;
              *PB11_BB = 1;
              *PB11_BB = 0;
              *PB11_BB = 1;
              *PB11_BB = 0;
              *PB11_BB = 1;
              *PB11_BB = 0;
              *PB11_BB = 1;
              *PB11_BB = 0;
            }
            // search for changes in button status & update
            int change = -1;
            int input;
            // digitalWrite( op_sr_data, HIGH );
            *PB07_BB = 1;
            for ( int i = 0 ; i <  NUM_SHIFT_REGISTER_BITS ; i++ ) {
              //digitalWrite( op_sr_clock, HIGH );
              //digitalWrite( op_sr_clock, LOW );
              *PB11_BB = 1;
              *PB11_BB = 0;
              digitalWrite( op_sr_data , LOW ); // only required after 1st clock
              *PB07_BB = 0;
              input = digitalRead( ip_sr_data );
              //input = *PB12_BB;
              if ( ShiftRegImage[scanId + i].Input != input ) {
                change = scanId + i; // record change for later update to avoid SR LEDs blinking out during USB update
                if ( input )
                  ShiftRegImage[scanId + i].Output = !ShiftRegImage[scanId + i].Output; // Toggle output for DEBUG
                ShiftRegImage[scanId + i].Input  = input; // DEBUG
                break;
              }
            }
            // load shift registers with LED status
            int invert = 0;
            //if (Cfg.Bits.hasShiftRegLEDInvert) invert = 1;
            digitalWrite( op_sr_data, ShiftRegImage[ scanId + num_sr_clocks - 1 ].Output ^ invert ); // improve data delay before rising clock
            for ( int i = 0 ; i <  NUM_SHIFT_REGISTER_BITS ; i++ ) {
              digitalWrite( op_sr_clock, HIGH );
              digitalWrite( op_sr_clock, LOW );
              digitalWrite( op_sr_data, ShiftRegImage[ scanId + num_sr_clocks - i - 2 ].Output ^ invert );
              // digitalWrite( op_sr_data, i&1);//ShiftRegImage[ scanId + num_sr_clocks - i - 2 ].Output ^ invert ); DEBUG
            }
            if ( change >=0 ) {
                char buff[80];
                sprintf(buff,"ShiftReg[%d]=%d\r\n",change,input);
                CompositeSerial.write( buff );
              //InputChange( change , input );
                if ( LastCommand == 'o' ) 
                  DisplayShiftRegisterIO();
            }
          }
          digitalWrite(op_sr_data,HIGH); // set shared data line high
        }
        scanId += num_sr_clocks;
        break;
    }
  }
}

void USBSerialUI::DisplayStatus(void) {
  char buff[80];
  char function_text[21];
  CompositeSerial.write(VT100_CLEAR);
  DisplayTitle("USB Midi Interface Status" );
  sprintf(buff,"id  port   function     kbd  count input error fault\n\r");
  CompositeSerial.write(buff);
  for ( int pin_id = 0 ; pin_id < NUM_GPIO_PINS ; pin_id ++ ) {
    sprintf(buff,"%2d  %4s %12s%5x %5x %5x %5x    '%c' \r\n",
      pin_id , 
      pinCfg[pin_id].description, 
      getFunctionText(&LiveConfigs[pin_id], function_text, 20 ),
      LiveConfigs[pin_id].keyboard ,
      LiveConfigs[pin_id].count ,
      LiveConfigs[pin_id].input ,
      LiveConfigs[pin_id].error,
      LiveConfigs[pin_id].fault ); 
    CompositeSerial.write(buff);
  }
}
void USBSerialUI::setFault( int pin_id , char fault ){
  LiveConfigs[pin_id].fault = fault;
}

void USBSerialUI::monitorNoteOn ( unsigned int channel, unsigned int note, unsigned int velocity , unsigned int device ){
  char buff[80];

  if ( Cfg.Bits.hasEventLog ) {
    sprintf(buff,"MidiNoteOn  Ch=%-2d  note=%-3d vel=%-3d %2s%d  %s\r\n",channel+1,note,velocity,note_name[note%OCTAVE],note/OCTAVE, device_names[device&3]);
    CompositeSerial.write(buff);
  }
  RequestDisplayUpdate();
}

void USBSerialUI::monitorNoteOff( unsigned int channel, unsigned int note, unsigned int velocity , unsigned int device ){
  char buff[80];
  if ( Cfg.Bits.hasEventLog ) {
    sprintf(buff,"MidiNoteOff Ch=%-2d  note=%-3d vel=%-3d %2s%d  %s\r\n",channel+1,note,velocity,note_name[note%OCTAVE],note/OCTAVE,device_names[device&3]);
    CompositeSerial.write(buff);
  }
  RequestDisplayUpdate();
}
