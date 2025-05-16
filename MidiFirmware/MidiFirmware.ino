#include <Arduino.h>
#include <USBComposite.h> // https://github.com/arpruss/USBComposite_stm32f1 Arduino version does not include Sysex support
#include <flash_stm32.h>
#include "led.h"
#include "lcd.h"
#include "mymidi.h"
#include "pin.h"
//#include "profile.h"
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
//USBHID HID;
//HIDKeyboard Keyboard(HID);
USBCompositeSerial CompositeSerial;
myMidi midi;
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
  SEG7.displayPChar((char *)"Add=32"); // Briefly Display sysex ID
  USBComposite.setProductId(0x0031);
  //USBComposite.setProductId(0x0003); // spoof DFU device
  USBComposite.setManufacturerString((const char *)"Richard Jones");
  // USBComposite.setProductString((const char *)"VPOrgan-Midi+HID+Serial");
  USBComposite.setProductString( VPOConsoleMsg[ SUI.midiKeyboardChannel() ] );
  USBComposite.setSerialString((const char *) "00001");
// Trim USBComposite Buffer to below 320 byte limit
//  int buf_size = 32;
//  HID.setTXPacketSize(buf_size);
//  CompositeSerial.setRXPacketSize(buf_size);
//  CompositeSerial.setTXPacketSize(buf_size);
  midi.registerComponent(); // Composite default midi buffer 128 bytes  
//  HID.registerComponent();  // Composite default HID buffer 64 Bytes
//  HID.setReportDescriptor(HID_KEYBOARD);
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
  delay(2000); // for reliable CompositeSerial.write()
  LCD_N_C32 lcd; // Scan i2c bus to discover any displays & initialise them
  SEG7.displayPChar( (char *) "ready ");
  if ( SUI.Cfg.Bits.hasEventLog ) {
    CompositeSerial.println( VPOConsoleMsg[ SUI.midiKeyboardChannel() ] );
  }
}



USBSerialUI SUI;

// the loop function runs over and over again forever
void loop() {
  led.service();
  SUI.poll(); // Check for keyboard input, display menus & responses, scan io
  midi.poll(); // check for midi input
}
