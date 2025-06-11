#include <Arduino.h>
#include <USBComposite.h>
#include "hid.h"
#include "led.h"
#include "USBSerialUI.h"
   HID_PT::HID_PT ( int pin )
   {
     hid_pin = pin;
   }

   int HID_PT::getEnabled(void) 
   { 
     return SUI.Cfg.Bits.hasPageTurning; 
   };
   
   void HID_PT::Print ( void ) {
     if ( getEnabled() ) {
       const char *on_off[] = { "ON","OFF" };
       char buf[80];
       CompositeSerial.write(ANSI_CLEAR);
       sprintf (buf,"Page Turning input = %s",on_off[last_state&1] );
       CompositeSerial.println (buf);
     } else {
       CompositeSerial.println ("Page Turning Not Enabled");
     }
   }

   // Contribute to event log if enabled
   void HID_PT::monitor( int hid_index ) {
     if ( SUI.Cfg.Bits.hasEventLog ) {
       char buf[120];
       sprintf( buf,"%10luus USB HID %s",micros(), hid_text[hid_index]);
       CompositeSerial.println(buf);
     }
   }

   // Count page turn button presses. 
   // Send HID codes as QWERTY Keyboard commands
   // 1 Press PAGE DOWN 
   // 2 Press PAGE UP
   // 1 short + Long Press Ctrl+HOME, PAGE_DOWN ...repeat
   // 2 short + Long Press Ctrl+END. Page_UP ...repeat
   void HID_PT::service(void) {
     const int DEBOUNCE_50ms = 50000; // ignore new input for debounce time following switch closure
     if ( getEnabled() ) {
      afio_cfg_debug_ports(AFIO_DEBUG_NONE); // Allow use of PB3, PA14
      pinMode(hid_pin,INPUT_PULLUP);
      unsigned long time_now = micros();
      if ( time_now - time_on > DEBOUNCE_50ms ) {
       int new_state = digitalRead(hid_pin); // Active on pull down
       if ( last_state != new_state ) {
         last_state = new_state;
         if ( last_state )
           led.off();
         else
           led.on();
         SUI.RequestDisplayUpdate();
         if ( new_state == ON ) {
           time_on  = time_now ;
           time_repeat = time_now;
         } else {
           if ( ( time_now - time_on ) < LONG_PRESS_US ) { // short press?
             if ( ( time_now - time_off ) > LONG_PRESS_US ) { // Long time since last released?
               short_count=0;
               //CompositeSerial.println("short_count reset");
             } else {
               if ( short_count < 1 ) { // Maybe more counts coming?
                 //CompositeSerial.println("++short_count");
                 ++short_count;
               }
             }
           }
           time_off = time_now ;
           //char buf[120];
           //sprintf( buf,"%luus Duration=%luus HID:NewOFF short_count=%d long_count=%d new_state=%d",time_now,time_now-time_on,short_count,long_count,new_state);
           //CompositeSerial.println(buf);
           if ( long_count == 0 ) { // Don't trigger on long release
             switch ( short_count &= 3 ) {
               case 0 :
                 QKeyboard.press   ( KEY_PAGE_DOWN );
                 QKeyboard.release ( KEY_PAGE_DOWN );
               break;
               case 1 :
                 QKeyboard.press   ( KEY_PAGE_UP );
                 QKeyboard.release ( KEY_PAGE_UP );
                 QKeyboard.press   ( KEY_PAGE_UP );
                 QKeyboard.release ( KEY_PAGE_UP );
               break;
             }
             monitor   ( short_count & 3 );
           }
         }
         long_count=0;
       }
       if ( last_state == ON ) {
         if ( (time_now - time_repeat) > LONG_PRESS_US ) { // Long press auto repeat
           //char buf[120];
           //sprintf( buf,"%luus %luus HID:ON Long short_count=%d long_count=%d new_state=%d",time_now,time_now-time_on,short_count,long_count,new_state);
           //CompositeSerial.println(buf);

            time_repeat += LONG_PRESS_US;
            led.toggle();
            if ( ++long_count==1 ) {  
              if ( short_count == 0 ) {
                 QKeyboard.press   ( KEY_LEFT_CTRL );
                 QKeyboard.press   ( KEY_HOME );
                 QKeyboard.release ( KEY_HOME );
                 QKeyboard.release ( KEY_LEFT_CTRL );
                 monitor   ( 3 );
              } else {
                 QKeyboard.press   ( KEY_LEFT_CTRL );
                 QKeyboard.press   ( KEY_END );
                 QKeyboard.release ( KEY_END );
                 QKeyboard.release ( KEY_LEFT_CTRL );
                 monitor   ( 2 );               
              }
            } else {
              if ( short_count == 0 ) {
                 QKeyboard.press   ( KEY_PAGE_DOWN );
                 QKeyboard.release ( KEY_PAGE_DOWN );
                 monitor   ( 0 );
              } else {
                 QKeyboard.press   ( KEY_PAGE_UP );
                 QKeyboard.release ( KEY_PAGE_UP );
                 monitor   ( 1 );                   
              }
            }
         }
       }
     }
    }
   }
