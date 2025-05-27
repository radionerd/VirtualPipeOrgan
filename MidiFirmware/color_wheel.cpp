//--------------------------------------------------------------
// Blue Pill STM32F103C8 WS2812 Neopixel driver example
// Color Wheel display
// Connect WS2812 LED string to pin PC13 (use level up-shifter)
// https://github.com/FearlessNight/bluepill_ws2812/blob/master/bluepill_ws2812.cpp

#include <Arduino.h>
#include <USBComposite.h>
#include "bluepill_ws2812.h"
#include "color_wheel.h"
#include "hsvtorgb.h"
#include "profile.h"

bluepill_neopixel PIX;       // a string of pixels
#define NUM_PIXELS 120     //   number of pixels in the string
pixel string[NUM_PIXELS]; //   rgb data buffer
#define string_port GPIOA //   pin string is connected to
#define string_pin  13


// Ref: https://en.wikipedia.org/wiki/HSL_and_HSV Fig24
void rwheel(pixel *p , uint8_t rot ) 
{
  int rot6 = (int)rot*6; // create 3 bit segment + 8 bit gradient from 8 bit rotation 
  int grad = rot6&0xff;
  switch ( rot6 & 0x700 ) {
    case 0x000 :
      p->rgb.r = 255;
      p->rgb.g = grad;
      p->rgb.b = 0;
    break;
    case 0x100 :
      p->rgb.r = 255-grad;
      p->rgb.g = 255;
      p->rgb.b = 0;
    break;
    case 0x200 :
      p->rgb.r = 0;
      p->rgb.g = 255;
      p->rgb.b = grad;
    break;
    case 0x300 :
      p->rgb.r = 0;
      p->rgb.g = 255-grad;
      p->rgb.b = 255;
    break;
    case 0x400 :
      p->rgb.r = grad;
      p->rgb.g = 0;
      p->rgb.b = 255;
    break;
    case 0x500 :
      p->rgb.r = 255;
      p->rgb.g = 0;
      p->rgb.b = 255-grad;
    break;
    default : // ERROR
      p->rgb.r = 255;
      p->rgb.g = 255;
      p->rgb.b = 255;
      
  }
}
/*
// RGB color wheel
void oldwheel( pixel *p, uint8_t w )
{
  if(w < 85)
  { p->rgb.r = 255 - w * 3;
    p->rgb.g = 0;
    p->rgb.b = w * 3;
  }
  else if(w < 170)
  { w -= 85;
    p->rgb.r = 0;
    p->rgb.g = w * 3;
    p->rgb.b = 255 - w * 3;
  }
  else
  { w -= 170;
    p->rgb.r = w * 3;
    p->rgb.g = 255 - w * 3;
    p->rgb.b = 0;
  }
}*/

// dim pixel down
void attenuate( pixel *p, uint8_t factor )
{
  p->rgb.r /= factor;
  p->rgb.g /= factor;
  p->rgb.b /= factor;
}

void Wsetup() 
{
  PIX.begin(string_port, string_pin); // set pin to output
}

void Wloop()
{
  static uint8_t wheel_index = 0; // color wheel

  // fill pixel buffer
  for (uint8_t i=0; i < NUM_PIXELS; i++)
  { rwheel( &string[i], wheel_index + i*(256/NUM_PIXELS) );  
    attenuate( &string[i], 8 );
  } 
  
  // output pixel buffer to string  
  PIX.paint( string[0].bytes, NUM_PIXELS, string_port, string_pin );

  // loop
  wheel_index++;
  delay(50);
}

void ColorWheel(void)
{
  static uint8_t wheel_index = 0; // color wheel
  static uint8_t setup_done=0;

  static unsigned long last_time=0;
  
  unsigned long time_now = micros();
  if ( ( time_now - last_time ) < 50000 ) return;
    last_time += 50000;

  if ( ! setup_done ) {
    PIX.begin(string_port, string_pin); // set pin to output
    ++setup_done;
  }
  // fill pixel buffer
  for (uint8_t i=0; i < NUM_PIXELS; i++)
  { rwheel( &string[i], wheel_index + i*(256/NUM_PIXELS) );  
    attenuate( &string[i], 8 );
  } 
  // output pixel buffer to string  
  PIX.paint( string[0].bytes, NUM_PIXELS, string_port, string_pin );

  // loop
  wheel_index++;
  //delay(50);
}


// LED Lighting strip control
// From a single button Short presses:
//   1 Turn on or off
//   2 show colour wheel
//   3 show static colour
// Long Press:
// Increment/decrement value selected by n short presses followed by long press
//   0   long cycle Brightness
//   1 + long cycle Saturation
//   2 + long cycle Hue
//enum { BRIGHTNESS, HUE, SATURATION } bhs;
int bsh[3]={120,120,0}; // not too bright white (hue:0-255,sat:0-255,brightness:0-255)

void LEDStripCtrl(int event ) {
//  const int LED_STRIP_RESET = 0;
  const int LED_STRIP_OFF = 0 ;
  const int LED_STRIP_ON  = 1 ;
  const int LED_STRIP_WHEEL = 2 ;
  static int LEDStripState=LED_STRIP_OFF; // OFF / ON / WHEEL
  static unsigned long buttonOnTime=0; // Time when button was pressed/Released
  //static unsigned long buttonOffTime=0; // Time when button was pressed/Released
  static int buttonState=LED_BUTTON_INVALID;// or LED_STRIP_ON
  static int buttonCount=2; // Wraps to zero on first button press
  pixel pix;  
  static int LEDStripUpdate=2; // Update twice at startup to avoid residual led being lit
  //char buff[180];
  // log scale derived from logscale[n] = 10^(log(255)-n*log(255)/32)
  const uint8_t log_scale[32]={
    255,214,180,152,128,107,90,76,64,54,45,38,32,
    27,23,19,16,13,11,9,8,7,6,5,4,3,3,2,2,1,1,1};

  static unsigned long last_time=0; // Last time this function was called
  unsigned long time_now = micros();
  switch ( event ) {
    case LED_BUTTON_ON :
      buttonOnTime = time_now;
      buttonState = event;
      ++buttonCount;
      //sprintf(buff,"LED_BUTTON_ON Exit buttonCount=%d\r\n",buttonCount);
      //CompositeSerial.write(buff);
    break;
    case LED_BUTTON_OFF:
      //buttonOffTime = time_now;
      buttonState = event;
    break;
    case LED_STRIP_SERVICE :
    {
      if ( time_now-last_time < 50000 ) return; // 50ms service interval
      profile.PStart(PROFILE_WS2812);
      if ( last_time == 0 ) {
        last_time = PROFILE_WS2812*PID_TIME_OFFSET_US;
      }
      last_time = time_now;
      if ( ( time_now - buttonOnTime ) > 1000000 ) { // long interval since button pressed / released
        // long button on time?
        if ( buttonState == LED_BUTTON_ON )  {
          if ( --buttonCount > 2 )
            buttonCount = 2;
          //sprintf(buff,"LongTime BUTTON_ON Start bsh[%d]=%d\r\n", buttonCount,bsh[buttonCount]);
          //CompositeSerial.write(buff);
          LEDStripState = LED_STRIP_ON;
          LEDStripUpdate++;
          bsh[buttonCount]++ ; // increment one of hue, saturation or brightness
          bsh[buttonCount]&=0x1ff;
          //int updown=bsh[buttonCount];
          //if ( updown > 255 ) updown = 511-updown;
          //sprintf(buff,"LongTime BUTTON_ON Exit bsh[%d]=%d (%d)\r\n", buttonCount,bsh[buttonCount],updown);
          //CompositeSerial.write(buff);
          ++buttonCount;
        }
        if ( buttonState == LED_BUTTON_OFF ) {
          //sprintf(buff,"Long time: LED_BUTTON_OFF Start LED StripState=%d buttonState=%d buttonCount=%d\r\n",LEDStripState,buttonState,buttonCount );
          //CompositeSerial.write(buff);
          switch ( --buttonCount ) {
            case 1:
              LEDStripState = LED_STRIP_WHEEL;
            break;
            default :
              if ( LEDStripState == LED_STRIP_OFF ) {
                LEDStripState = LED_STRIP_ON; 
              } else {
                LEDStripState = LED_STRIP_OFF; 
              }
              LEDStripUpdate++;
          }
          if ( ( time_now - buttonOnTime ) > 1500000 ) 
            LEDStripState = LED_STRIP_ON;
          buttonState = LED_BUTTON_INVALID; // Inhibit timer calling again for OFF
          buttonCount=0;  
          //sprintf(buff,"Long time: LED_BUTTON_OFF Exit LED  StripState=%d buttonState=%d buttonCount=%d\r\n",LEDStripState,buttonState,buttonCount );
          //CompositeSerial.write(buff);
        }
      }        
      if ( LEDStripState == LED_STRIP_WHEEL ) {
          ColorWheel();
      }
      if ( LEDStripUpdate ) {
        --LEDStripUpdate;
        pix.rgb.r=0;
        pix.rgb.g=0;
        pix.rgb.b=0;
        if ( LEDStripState == LED_STRIP_ON ) {
          // Load with hsb conversion 
          int h = bsh[2];
          if ( h > 255 ) h = 511-h;
          
          int s = bsh[1]&0x1ff;
          if ( s > 255 ) s = 511-s;
          
          int b = bsh[0]&0x1ff;
          if ( b > 255 ) b = 511-b;
          //sprintf(buff,"Bri=%3d Sat=%3d Hue=%3d\r\n",log_scale[b>>3],log_scale[s>>3],h);
          //CompositeSerial.write(buff);
          RGBColor rgb = hsv2rgb(h,log_scale[s>>3],log_scale[b>>3]);
          pix.rgb.r=rgb.r; // DEBUG
          pix.rgb.g=rgb.g;
          pix.rgb.b=rgb.b;
        }
        // fill pixel buffer
        for (uint8_t i=0; i < NUM_PIXELS; i++) { 
           string[i].rgb.r  = pix.rgb.r;
           string[i].rgb.g  = pix.rgb.g;
           string[i].rgb.b  = pix.rgb.b;
        }
        PIX.begin(string_port, string_pin); // set pin to output
        PIX.paint( string[0].bytes, NUM_PIXELS, string_port, string_pin );
      }
      profile.PEnd(PROFILE_WS2812);
    }
    break;
  }
}
