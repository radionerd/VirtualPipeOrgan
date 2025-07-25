#include <Arduino.h>
#include <USBComposite.h>
#include <flash_stm32.h>
#include <stdio.h>
#include "adc.h"
#include "bluepill_ws2812.h"
#include "buttonscan.h"
#include "hid_pt.h"
#include "keyboardscan.h"
#include "led.h"
#include "multilcd.h"
#include "mymidi.h"
#include "pin.h"
#include "profile.h"
#include "TM1637.h"
#include "USBSerialUI.h"
#include "ws2812ctrl.h"
// Copyright (C)2023 Richard Jones. MIT License applies
extern ADC adc;
extern ButtonScan Button;
extern HID_PT hid_pt;
extern KeyboardScan kbd;
extern LED led; // or led(PC13); choose your own LED gpio pin
extern MultiLCD mlcd;
extern TM1637 SEG7; // 7 Segment display driver
extern WS2812Ctrl ws2812Ctrl;
extern void * __text_start__; // Declare the _start function

USBSerialUI::USBSerialUI(void) {
  RestoreConfigFromFlash();
  TestIO();
  adc.Begin();
}
void USBSerialUI::poll(void) {

  char buf[80];
  while (CompositeSerial.available()) {
    char c = CompositeSerial.read();
    if ( false ) {
      // if ( true ) show incoming codes 0x00-0xFF for HID investigation
      char view = c&0x7f;
      if ( view < ' ' )
        view = ' '; 
      sprintf(buf,"Keyboard Char=%02X %c\r\n",c,view);
      CompositeSerial.write(buf); return;
    }
    CommandCharDecode(c);
    LastCommand = c;
  }
  if ( DisplayUpdate ) { // Refresh monitoring screens
    switch ( LastCommand ) {
      case 'k' :
      case 'o' :
      case 'w' :
      case 'x' :
      case 'u' :
      case 'z' :
        CommandCharDecode(LastCommand);
      break;
    }
    DisplayUpdate = 0 ;
  }
}

void USBSerialUI::handleNoteOn( unsigned int midi_channel, unsigned int midi_note, unsigned int midi_velocity ) {
  if ( midi_channel == midi.getButtonChannel() ) {
    led.on(); // Turn on LED briefly to indicate addressed USB input
    monitorNoteOn   ( midi_channel, midi_note, midi_velocity , DEV_LED );
    Button.SetLED(midi_note,TRUE);
  }
  AppOnlineTime = micros();
}
void USBSerialUI::handleNoteOff( unsigned int midi_channel, unsigned int midi_note, unsigned int midi_velocity ) {
  if ( midi_channel == midi.getButtonChannel() ) {
    led.off(); // Turn off LED briefly to indicate addressed USB input
    monitorNoteOff   ( midi_channel, midi_note, midi_velocity, DEV_LED );
    Button.SetLED(midi_note,FALSE);
  }
  AppOnlineTime = micros();
}

void  USBSerialUI::handleControlChange( unsigned int midi_channel, unsigned int controller, unsigned int velocity ) {
  char buf[80];
  if ( midi_channel == midi.getKeyboardChannel() ) {
    led.toggle(); // Flash LED briefly to indicate addressed USB input
    if ( Cfg.Bits.hasEventLog) {
      sprintf(buf,"Unhandled Control Change channel=%d controller=%d velocity=%d\r\n",midi_channel+1,controller,velocity);
      CompositeSerial.write(buf);
    }
  }
}


void USBSerialUI::CommandCharDecode( char c )
{
    CompositeSerial.write("\r\n");
    switch ( tolower(c) ) {
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
          ConfigValue [0] = Cfg.Bits.midiChannel;
          ConfigValue [1] = Cfg.Bits.hasPedalBoard;
          ConfigValue [2] = Cfg.Bits.numberADCInputs;
          ConfigValue [3] = Cfg.Bits.hasKeyVelocity  ;
          ConfigValue [4] = Cfg.Bits.hasI2C       ;
          ConfigValue [5] = Cfg.Bits.hasTM1637    ;
          ConfigValue [6] = Cfg.Bits.hasButtonLEDMode ;
          ConfigValue [7] = Cfg.Bits.hasPageTurning;
          ConfigValue [8] = Cfg.Bits.hasWS2812LEDStrip;
          ConfigValue [9] = Cfg.Bits.hasEventLog  ; // K
          int i = 0;
          if ( c != ' ' ) {
            // Adjust configured value by index for display
            i = ( ( c - 'a' ) & 0xf ) ;
            if ( c == tolower(c) ) { // increase or decrease count?
              ConfigValue[i] ++ ;
              if ( ConfigValue[i] > ConfigItems[i].maxval )
                ConfigValue[i] = 0;
            } else {
              ConfigValue[i]--;
              if (ConfigValue[i] < 0 )
                ConfigValue[i] = ConfigItems[i].maxval;
            }
            // Special case for > 4 ADC with Keyboard
            if ( ( Cfg.Bits.hasPedalBoard==0 ) && ( Cfg.Bits.hasKeyVelocity==0 ) && ( ConfigValue[2] > 4 ) )
               ConfigValue[2]=0 ;
            Cfg.Bits.midiChannel       = ConfigValue [0];
            Cfg.Bits.hasPedalBoard     = ConfigValue [1];
            Cfg.Bits.numberADCInputs   = ConfigValue [2];
            Cfg.Bits.hasKeyVelocity    = ConfigValue [3];
            Cfg.Bits.hasI2C            = ConfigValue [4];
            Cfg.Bits.hasTM1637         = ConfigValue [5];
            Cfg.Bits.hasButtonLEDMode  = ConfigValue [6];
            Cfg.Bits.hasPageTurning    = ConfigValue [7];
            Cfg.Bits.hasWS2812LEDStrip = ConfigValue [8]; // I
            Cfg.Bits.hasEventLog       = ConfigValue [9]; // J
          }
          DisplayFunctionPinOut();
          DisplayConfigurationMenu();
        }
        break;
      case 'k' :
       if ( c =='K' )
        kbd.PCodeGenerator();
       else
        kbd.Print() ;
        break;
      case 'l' : // LCD
        if ( c =='L' )
          mlcd.Test();
        mlcd.Print();
        break;
      case 'm' :
        DisplayFlashSummary() ;
        break;
      case 'o' :
        Button.Print();
        break;
      case 'p' :
        if ( c == 'P' )
          profile.PReset();
        profile.PStart(PROFILE_PPRINT);
        profile.PPrint();
        profile.PEnd  (PROFILE_PPRINT);
        break;
      case 'r' :// restore config from flash
        if ( c == 'R' ) {
          CompositeSerial.write("Restart...\r\n");        
          nvic_sys_reset();
        } else {
          RestoreConfigFromFlash();
          DisplayFunctionPinOut();
          DisplayConfigurationMenu();
          CompositeSerial.write("Press 'R' to restart\r\n");        
        }
        break;
      case 'y' :
      case 's' :
        SaveConfigToFlash(c);
        //RestoreConfigFromFlash(); // Update LiveConfigs 
        break;
      case 't' :
        TestIO();
        break;
      case 'u' :
        hid_pt.Print();
      break;
      case 'v' :
        {
          char buff[80];
          //int i = strlen(__FILE__);
          //while ( --i ) {
          //  if ( __FILE__[i - 1] == '/' ) break; // strip lengthy file path
          //}
          sprintf ( buff , "%s %s %s\r\n","V1.0.2" , __DATE__, __TIME__);
          CompositeSerial.write(buff);
          unsigned int text_start_address =(unsigned int )&__text_start__;
          sprintf(buff,"Start of text section: 0x%X\r\n\n", text_start_address);
          CompositeSerial.write(buff);
          CompositeSerial.println("0x8000000 Bootloader None");
          CompositeSerial.println("0x8001000 Bootloader HID");
          CompositeSerial.println("0x8002000 Bootloader STM32duino");
        }
        break;
      case 'w' :
        ws2812Ctrl.Print();
        break;
      case 'x' :
        adc.Print();
        break;
      case 'z' :
        DisplayStatus();
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
                (unsigned int)&address[0],
                address[0x0], address[0x1], address[0x2], address[0x3],
                address[0x4], address[0x5], address[0x6], address[0x7] );
      CompositeSerial.write(buff);
      address += 8;
      sprintf ( buff , "%08X %04X %04X %04X %04X %04X %04X %04X %04X\r\n",
                (unsigned int)&address[0],
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
      sprintf( buff, "Flash Write %08X=%04X\r\n", (unsigned int )FlashPage + i * 2, Cfg.Word );
      CompositeSerial.write(buff);
      pstatus = FLASH_ProgramHalfWord( (uint32)FlashPage + i * 2, Cfg.Word );
      sprintf( buff, "Flash Program Status=%d\r\n", pstatus );
      CompositeSerial.write(buff);
    } else {
      CompositeSerial.write("Same Value, not written\r\n");
    }
  } else {
    sprintf( buff, "MIDI Flash Signature Not Found\r\n" );
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
//  int oldCfg = Cfg.Word;
  Cfg.Word = 0;
  if ( FlashPage[0] == FlashSignature ) {
    while ( FlashPage[++i] != 0xFFFF ) ;
    Cfg.Word = FlashPage[i - 1];
  }
  Cfg.Bits.hasEventLog = 0; // Force off: To avoid lockup when terminal not emptying log buffer
  fillCfgPinData( Cfg.Word , &LiveConfigs[0] );
}

void USBSerialUI::DisplayTitle( const char *title ) {
  char buff[80];
  sprintf(buff, "%s%s%s%s\r\n", ANSI_UNDERLINE, title, ANSI_NO_UNDERLINE, ANSI_CLR_EOL );
  CompositeSerial.write( buff );
}

void USBSerialUI::DisplayPrompt(const char *prompt ) {
  CompositeSerial.write(prompt);
  CompositeSerial.write(ANSI_ERASE_DOWN );
}
void USBSerialUI::DisplayConfigurationMenu() {
  int i;
  char buff[80];

  ConfigValue [0] = Cfg.Bits.midiChannel;
  ConfigValue [1] = Cfg.Bits.hasPedalBoard;
  ConfigValue [2] = Cfg.Bits.numberADCInputs;
  ConfigValue [3] = Cfg.Bits.hasKeyVelocity  ;
  ConfigValue [4] = Cfg.Bits.hasI2C       ;
  ConfigValue [5] = Cfg.Bits.hasTM1637    ;
  ConfigValue [6] = Cfg.Bits.hasButtonLEDMode ;
  ConfigValue [7] = Cfg.Bits.hasPageTurning;
  ConfigValue [8] = Cfg.Bits.hasWS2812LEDStrip;
  ConfigValue [9] = Cfg.Bits.hasEventLog  ; // K

  DisplayTitle("Configuration Menu");
  i = 0;
  while ( ConfigItems[i].maxval != 0 ) {
    char valtext[20];
    if ( ConfigItems[i].maxval > 1 ) {
      int dvalue = ConfigValue[i];
      if (  i == 2 ) { // Number of ADCs inputs?
        if ( Cfg.Bits.hasPedalBoard == 0 ) { // Force display to zero while preserving value
          if ( Cfg.Bits.hasKeyVelocity ) {
            dvalue = 0 ;
          } else {
           if ( dvalue > 4 ) {
              dvalue = 0;
              Cfg.Bits.numberADCInputs = dvalue;
           }
          }
        }
      }
      sprintf ( valtext, "%d", dvalue );
    } else {
      strncpy(valtext, "✓", 5);
      if ( ( ConfigValue[i] ) == 0 ) {
        valtext[0] = ' ' ; // '✓';
        valtext[1] = 0 ;
      }
    }
    if ( i == 0 )
      sprintf(buff, "%c [%d] %s=%d, Button channel=%d%s\r\n", 'A' + i, midi.getKeyboardChannel()+1, ConfigItems[i].text, midi.getKeyboardChannel()+1, midi.getButtonChannel()+1, ANSI_CLR_EOL );
    else
      if ( i == 6 )
        sprintf(buff, "%c [%s] %s%s%s\r\n", 'A' + i, valtext, ConfigItems[i].text, ButtonModeText[Cfg.Bits.hasButtonLEDMode], ANSI_CLR_EOL );    
      else  
        sprintf(buff, "%c [%s] %s%s\r\n", 'A' + i, valtext, ConfigItems[i].text, ANSI_CLR_EOL );    
    CompositeSerial.write(buff);
    i++;
  }
  sprintf(buff, "Cfg.Word=%04X%s\r\n", Cfg.Word, ANSI_CLR_EOL);
  CompositeSerial.write(buff);
  DisplayPrompt((const char *)"Enter A-L,a-l To adjust cfg value, S to Save, ? - Menu:");
}

void USBSerialUI::DisplayFunctionPinOut(void) {
  int i;
  char buff[120];

  fillCfgPinData( Cfg.Word , &MenuConfigs[0] );
  if ( Cfg.Bits.hasI2C )
    mlcd.I2cCheck();
  sprintf(buff, "%s%sSTM32 Blue Pill Assigned Pin Functions%s%s\r\n", ANSI_CURSOR_00, ANSI_UNDERLINE, ANSI_NO_UNDERLINE, ANSI_CLR_EOL );
  CompositeSerial.write( buff ); // ANSI Cursor position 0,0
  char function_text1[80];
  char function_text2[80];

  for ( i = 0 ; i < 20 ; i++ )
  {
    getFunctionText( &MenuConfigs[ i + 20], &function_text1[0], 20 );
    getFunctionText( &MenuConfigs[19 -  i], &function_text2[0], 20 );
    sprintf(buff, "%-13s%s%c%s%4s         %-4s%s%c%s  %-s%s\r\n", 
      function_text1, ANSI_BLINK,LiveConfigs[i+20].fault,ANSI_NO_BLINK, pinCfg[i + 20].description, pinCfg[19 - i].description,ANSI_BLINK,LiveConfigs[19-i].fault,ANSI_NO_BLINK, function_text2, ANSI_CLR_EOL );
    if ( i == 0 )
      strncpy( &buff[25+4], "USB", 3);
    if ( i == 19 )
      strncpy( &buff[19+4*2], "| | | |", 7);
    CompositeSerial.write(buff);
  }
  function_text1[0]=0;
  function_text2[0]=0;
  for ( i = 0 ; i < NUM_GPIO_PINS ; i ++ )
  {
    if ( LiveConfigs[i].fault == '*' ) {
      sprintf(function_text1,"%s* = Fault%s",ANSI_BLINK,ANSI_NO_BLINK);
    }
    if ( LiveConfigs[i].fault == '!' ) {
      sprintf(function_text2,"%s        ! = Add 4k7 Pullup to +5V%s",ANSI_BLINK,ANSI_NO_BLINK);    
    }
  }
  i = NUM_GPIO_PINS-3;
  sprintf(buff, "%-19s| | | |%s%s\r\n",function_text1,function_text2, ANSI_CLR_EOL);
  CompositeSerial.write(buff);
  getFunctionText(&MenuConfigs[i], function_text1, 20 );
  getFunctionText(&MenuConfigs[i+1], function_text2, 20 );
  sprintf(buff, "%-13s%c%-4s---+ +---%s%c  %s%s\r\n", function_text1,LiveConfigs[i].fault, pinCfg[i].description, pinCfg[i+1].description,LiveConfigs[i+1].fault,function_text2, ANSI_CLR_EOL );
  CompositeSerial.write(buff);
  getFunctionText(&MenuConfigs[i+2], function_text1, 20 );
  sprintf(buff, "%-13s%c%-4s (on Jumper via 100K resistor)%s\r\n%s\r\n", function_text1,LiveConfigs[i+2].fault, pinCfg[i+2].description, ANSI_CLR_EOL,ANSI_CLR_EOL );
  CompositeSerial.write(buff);
}

char * USBSerialUI::getFunctionText(GPIOPinConfig *pinCfg, char *buff, int n ) {
  int pin_function = pinCfg->function;
  int count = pinCfg->count;

  strncpy( buff, function_text[pin_function], n );
  switch ( pin_function ) {
    case IP_ADC :
    case IP_SCAN :
    case OP_SCAN :
      sprintf( buff + strlen(buff), " %x", count);
  } 
  return buff;
}

void USBSerialUI::fillCfgPinData( unsigned ConfigWord, GPIOPinConfig * newCfgs ) {
  int op_count  = 0;
  int keyboard  = 0;

  Config tcfg;
  tcfg.Word = ConfigWord;
  for ( int gpio_index = 0 ; gpio_index < NUM_GPIO_PINS ; gpio_index ++ ) {
    newCfgs[gpio_index].function = pinCfg[gpio_index].function;
    newCfgs[gpio_index].count = pinCfg[gpio_index].ccCommand;
    newCfgs[gpio_index].keyboard = 0;    
    newCfgs[gpio_index].error = 0;
    newCfgs[gpio_index].fault = ' ';
    switch ( pinCfg[gpio_index].function ) {
      case OP_LED :
      break;
      case OP_SCAN :
        newCfgs[gpio_index].function = OP_SCAN;
        newCfgs[gpio_index].count    = op_count++;
        newCfgs[gpio_index].keyboard = keyboard;            
      break;
      case IP_SCAN :
        // Enable ADCs and WS2812 with peddleboard
        if ( tcfg.Bits.hasPedalBoard ) {
          if ( newCfgs[gpio_index].count > 7 ) {
            newCfgs[gpio_index].function = IP_ADC ;
            newCfgs[gpio_index].count = 15-newCfgs[gpio_index].count;
            if ( newCfgs[gpio_index].count >= tcfg.Bits.numberADCInputs ) {
              newCfgs[gpio_index].function = IO_SPARE ;
            }          
            //if ( tcfg.Bits.hasWS2812 && ( newCfgs[gpio_index].count == 7 ) )
            //  newCfgs[gpio_index].function = IO_WS2812 ;
          } else {
            if ( ( tcfg.Bits.hasKeyVelocity == 0 ) && ( newCfgs[gpio_index].count & 1 ) )
              newCfgs[gpio_index].function = IO_SPARE ;
          }
        } else {
          // Keyboard without velocity sense may have up to four adc's on unused scan inputs
          if ( tcfg.Bits.hasKeyVelocity == 0 ) {
            int count = newCfgs[gpio_index].count;
            if ( count & 1 ) { // only odd inputs unused
              if ( count > 7 ) { // unused input range
                count = 15-newCfgs[gpio_index].count; // Adjust count to ADC
                newCfgs[gpio_index].count = count;
                if ( newCfgs[gpio_index].count/2 >= tcfg.Bits.numberADCInputs ) {
                   newCfgs[gpio_index].function = IO_SPARE ;
                } else {
                   newCfgs[gpio_index].function = IP_ADC ;
                }
              } else {
                   newCfgs[gpio_index].function = IO_SPARE ;  
              }
            }
          }
        }
      break;
/*      case IP_ADC :
        if ( adc_count >  tcfg.Bits.numberADCInputs ) {
          newCfgs[gpio_index].function = OP_SCAN;
          newCfgs[gpio_index].count    = op_count++;
          newCfgs[gpio_index].keyboard = keyboard;            
        } else {
          newCfgs[gpio_index].count    = adc_count++;
        }
        break;*/
      case I2C_SCL :
        if ( tcfg.Bits.hasI2C == 0 ) {
          newCfgs[gpio_index].function = IO_SPARE;
        }
      break;
      case TM1637_CK :
        if ( tcfg.Bits.hasTM1637 == 0 ) {
          newCfgs[gpio_index].function = IO_SPARE;
        }
        break;
      case IP_SR_DATA :
      case OP_SR_CLOCK :
        //if ( tcfg.Bits.hasShiftRegs == 0 ) {
        //  newCfgs[gpio_index].function = IO_SPARE;
        //}
        break;
      case I2C_SDA :
      case COMMON_DATA :
        if ( ( Cfg.Bits.hasI2C | Cfg.Bits.hasTM1637 ) == 0 ) {
          newCfgs[gpio_index].function = IO_SPARE;
        }
        break;
      case BOOT1 :
      break;
      case SWCLK :
        if ( Cfg.Bits.hasPageTurning )
          newCfgs[gpio_index].function = IO_HID;
      break;
      case SWDIO :
        if ( Cfg.Bits.hasWS2812LEDStrip ) {
          newCfgs[gpio_index].function = IO_WS2812;
        }
        break;
    }
  }
}


void USBSerialUI::DisplayMenu(void) {
  CompositeSerial.write(ANSI_CLEAR);
  DisplayTitle("MIDI Interface Menu" );
  CompositeSerial.write("@ - or spacebar to View Configuration\r\n");
  CompositeSerial.write("A-J Adjust configuration value\r\n");
  CompositeSerial.write("K - Keyboard Contacts\r\n");
  CompositeSerial.write("L - LCD 'l'=view, 'L'=load\r\n");
  CompositeSerial.write("M - Flash memory summary\r\n");
  CompositeSerial.write("O - Shift Register IO\r\n");
  CompositeSerial.write("P - Profile\r\n"); // show max execution time
  CompositeSerial.write("R - Restore Configuration\r\n");
  CompositeSerial.write("S - Save New Configuration\r\n");
  CompositeSerial.write("T - Test IO Pins\r\n");
  CompositeSerial.write("U - USB HID Page Turn Input\r\n");
  CompositeSerial.write("V - Version Info\r\n");
  CompositeSerial.write("W - WS2812 LED Strip\r\n");
  CompositeSerial.write("X - eXpression Pedal / ADC results\r\n");
  CompositeSerial.write("Z - Pin Status\r\n");
}



void USBSerialUI::TestIO()
{
  char fail = '*';
  char pass = ' ';
  char result;
  int failures=0;
  char buff[90];
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
  sprintf(buff,"Test Result %d failures\r\n",failures);
  CompositeSerial.write(buff);
  pinMode(LED_BUILTIN , OUTPUT);
}

/*
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
}*/

void USBSerialUI::DisplayStatus(void) {
  char buff[80];
  char function_text[21];
  CompositeSerial.write(ANSI_CLEAR);
  DisplayTitle( (const char *) "USB MIDI Interface Status" );
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
      if ( device > DEV_LED )
        device = DEV_UNKNOWN;
    if ( ( device==DEV_KEYBOARD ) && ( Cfg.Bits.hasPedalBoard ) )
      device = DEV_PEDALBOARD;
    // C(60) == C4 is debateable but agrees with MidiSnoop
    unsigned int octave = ( note - OCTAVE ) / OCTAVE; // C(60) == C4 is debateable but agrees with MidiSnoop
    if ( note < OCTAVE ) octave=0;
    sprintf(buff,"%10luus MIDINoteOn  Ch=%-2d  note=%-2d/%-2s%d vel=%-3d %s\r\n",micros(),channel+1,note,note_name[note%OCTAVE],octave, velocity, device_names[device]);
    CompositeSerial.write(buff);
  }
  RequestDisplayUpdate();
}

void USBSerialUI::monitorNoteOff( unsigned int channel, unsigned int note, unsigned int velocity , unsigned int device ){
  char buff[80];
  if ( Cfg.Bits.hasEventLog ) {
    if ( device > DEV_LED )
      device = DEV_UNKNOWN;
    if ( ( device==DEV_KEYBOARD ) && ( Cfg.Bits.hasPedalBoard ) )
      device = DEV_PEDALBOARD;
    unsigned octave = ( note - OCTAVE ) / OCTAVE; // C(60) == C4 is debateable but agrees with MidiSnoop
    if ( note < OCTAVE ) octave=0;
    sprintf(buff,"%10luus MIDINoteOff Ch=%-2d  note=%-2d/%-2s%d vel=%-3d %s\r\n",micros(),channel+1,note,note_name[note%OCTAVE],octave,velocity,device_names[device]);
    CompositeSerial.write(buff);
  }
  RequestDisplayUpdate();
}

void USBSerialUI::RequestDisplayUpdate(void) 
{ 
  DisplayUpdate = 1; 
}
