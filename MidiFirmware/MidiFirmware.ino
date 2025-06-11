#include <Arduino.h>
#include <USBComposite.h> // https://github.com/arpruss/USBComposite_stm32f1 Arduino version does not include Sysex support
#include <flash_stm32.h>
#include "adc.h"
#include "buttonscan.h"
#include "color_wheel.h"
#include "hid.h"
#include "keyboardscan.h"
#include "led.h"
#include "multilcd.h"
#include "mymidi.h"
#include "pin.h"
#include "profile.h"
#include "TM1637.h" // Must include RJ enhancments for displayPChar(char *) https://github.com/RobTillaart/TM1637_RT
#include "USBSerialUI.h"

/*
   MidiFirmware

   Copyright (C)2022,2023 richard (dot) jones (dot ) 1952 ( at ) gmail (dot ) com

   See README.md for helpful details

   License: MIT
*/

LED led; // or led(PC13); choose your own LED gpio pin
TM1637 SEG7; // 7 Segment display driver
USBHID HID;
HIDKeyboard QKeyboard(HID);
USBCompositeSerial CompositeSerial;
myMidi midi;
MultiLCD mlcd;
ButtonScan Button;
KeyboardScan kbd;
ADC adc;
PROFILE profile;
HID_PT hid_pt(PA14);

const char *VPOConsoleMsg[16]= {
  "VPO Console Midi Ch1, 9",
  "VPO Console Midi Ch2, 10",
  "VPO Console Midi Ch3, 11",
  "VPO Console Midi Ch4, 12",
  "VPO Console Midi Ch5, 13",
  "VPO Console Midi Ch6, 14",
  "VPO Console Midi Ch7, 15",
  "VPO Console Midi Ch8, 16",
  "VPO Console Midi Ch9, 1",
  "VPO Console Midi Ch10, 2",
  "VPO Console Midi Ch11, 3",
  "VPO Console Midi Ch12, 4",
  "VPO Console Midi Ch13, 5",
  "VPO Console Midi Ch14, 6",
  "VPO Console Midi Ch15, 7",
  "VPO Console Midi Ch16, 8",
};

void setup() {
  int i = 0;
  pinMode ( LED_BUILTIN, OUTPUT );
  SEG7.begin(PB10,PB7,6); // begin(uint8_t clockPin, uint8_t dataPin, uint8_t digits)
  SEG7.setBrightness(1);
  char buf[80];
  sprintf(buf,"PC ???");
  SEG7.displayPChar(buf); // Display ??? until we get PC response
  mlcd.Begin((char *)"PC/USB Comms ???"); // Search for and connected LCDs
  USBComposite.setProductId(0x0031);
  //USBComposite.setProductId(0x0003); // spoof DFU device
  USBComposite.setManufacturerString((const char *)"New Zealand");
  USBComposite.setProductString( VPOConsoleMsg[ midi.getKeyboardChannel() ] );
  USBComposite.setSerialString((const char *) "00001");
// Trim USBComposite Buffer to below 320 byte limit
  int buf_size = 32;
  HID.setTXPacketSize(buf_size);
  CompositeSerial.setRXPacketSize(buf_size);
  CompositeSerial.setTXPacketSize(buf_size);
  midi.registerComponent(); // Composite default midi buffer 128 bytes  
  HID.registerComponent();  // Composite default HID buffer 64 Bytes
  HID.setReportDescriptor(HID_KEYBOARD);
  //HID.begin(); // Not required according to Rogers instructions
  CompositeSerial.registerComponent(); // Composite serial buffer 128 bytes has to be reduced
  if ( USBComposite.begin() == false ) { // Fast flash indicate error forever
    while (1) {
      led.on();
      delay(50);
      led.off();
      delay(50);
    }
  }
  while (!USBComposite) digitalWrite(LED_BUILTIN, ++i & 1); // Super fast flash while waiting for USB to register
  int midi_display_channel = midi.getKeyboardChannel()+1;
  sprintf(buf,"Add=%-2d",midi_display_channel );
  SEG7.displayPChar(buf); // Display sysex ID
  mlcd.saveDisplayText( midi_display_channel,buf );

  mlcd.Begin((char *)""); // Display VPO & ADDr Info
  //LEDStripCtrl(1);
  delay(2000); // for reliable CompositeSerial.write()
//if ( SUI.Cfg.Bits.hasEventLog ) {
  //  CompositeSerial.println( VPOConsoleMsg[ SUI.midiKeyboardChannel() ] );
  // }
}



USBSerialUI SUI;
/*
enum item { 
  PROFILE_KEYBOARD=0,
  PROFILE_SUI,
  PROFILE_ADC, 
  PROFILE_WS2812, 
  PROFILE_BUTTONS,
  PROFILE_MIDI_POLL,
  PROFILE_SYSEX,
  PROFILE_MIDI_OUT_TO_SYSEX_IN,
  PROFILE_LOOP,
  PROFILE_PPRINT,
  PROFILE_SPARE
  } ; */


// the loop function runs over and over again forever
void loop() {
  static int pollCount;
  static unsigned long last_time;

  unsigned long time_now = micros();
  if ( ( time_now - last_time ) > PID_TIME_OFFSET_US ) { // Call at 1ms intervals
    last_time += PID_TIME_OFFSET_US;
    profile.PStart( PROFILE_LOOP );
    led.service();
    {
      profile.PStart( PROFILE_KEYBOARD );
      kbd.MusicKeyboardScan(SUI.Cfg.Bits.hasPedalBoard);
      profile.PEnd( PROFILE_KEYBOARD );
    }
    if ( pollCount <= PROFILE_MIDI_POLL )
      profile.PStart( pollCount);
    switch ( pollCount  ) {
      case PROFILE_SUI :        
        SUI.poll(); // Check for keyboard input, display menus & responses, scan io
      break;
      case PROFILE_ADC :
         adc.Scan();
      break;
      case PROFILE_WS2812 :
         LEDStripCtrl( LED_STRIP_SERVICE );
      break;
      case PROFILE_BUTTONS :
         Button.Scan();
      break;
      case PROFILE_MIDI_POLL :
        midi.poll(); // check for midi input
      break;
      case PROFILE_HID:
        hid_pt.service();
    }
    if ( pollCount <= PROFILE_MIDI_POLL )
      profile.PEnd( pollCount);
    if ( ++pollCount >= 11 )
      pollCount = 1;
  }
  profile.PEnd( PROFILE_LOOP );
}
