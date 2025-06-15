#include <Arduino.h>
#include <USBComposite.h>
#include "bluepill_ws2812.h"
#include "hsvtorgb.h"
#include "profile.h"
#include "USBSerialUI.h"
#include "ws2812ctrl.h"

WS2812Ctrl::WS2812Ctrl(void)
{
  PIX.begin(led_strip_port, led_strip_pin); // set pin to output
}

// Ref: https://en.wikipedia.org/wiki/HSL_and_HSV Fig24
void WS2812Ctrl::rwheel(pixel *p, uint8_t rot)
{
  int rot6 = (int)rot * 6; // create 3 bit segment + 8 bit gradient from 8 bit rotation
  int grad = rot6 & 0xff;
  switch (rot6 & 0x700)
  {
  case 0x000:
    p->rgb.r = 255;
    p->rgb.g = grad;
    p->rgb.b = 0;
    break;
  case 0x100:
    p->rgb.r = 255 - grad;
    p->rgb.g = 255;
    p->rgb.b = 0;
    break;
  case 0x200:
    p->rgb.r = 0;
    p->rgb.g = 255;
    p->rgb.b = grad;
    break;
  case 0x300:
    p->rgb.r = 0;
    p->rgb.g = 255 - grad;
    p->rgb.b = 255;
    break;
  case 0x400:
    p->rgb.r = grad;
    p->rgb.g = 0;
    p->rgb.b = 255;
    break;
  case 0x500:
    p->rgb.r = 255;
    p->rgb.g = 0;
    p->rgb.b = 255 - grad;
    break;
  default: // ERROR
    p->rgb.r = 255;
    p->rgb.g = 255;
    p->rgb.b = 255;
  }
}
// dim pixel down
void WS2812Ctrl::attenuate(pixel *p, uint8_t factor)
{
  p->rgb.r /= factor;
  p->rgb.g /= factor;
  p->rgb.b /= factor;
}

int WS2812Ctrl::getEnabled(void)
{
  return SUI.Cfg.Bits.hasWS2812LEDStrip;
};

// Contribute to event log if enabled
void WS2812Ctrl::eventMonitor(int ws2812_index)
{
  if (SUI.Cfg.Bits.hasEventLog)
  {
    const char * state_names[] = {"OFF","ON","Rainbow","Unknown"};
    char buf[120];
    sprintf(buf, "%10luus WS2812 LED Strip %s\r\n", micros(),state_names[ws2812_index&3]);
    CompositeSerial.write(buf);
  }
}

void WS2812Ctrl::colorWheel(void)
{
  static uint8_t wheel_index = 0; // color wheel
  static uint8_t setup_done = 0;

  static unsigned long last_time = 0;

  unsigned long time_now = micros();
  if ((time_now - last_time) < 50000)
    return;
  last_time += 50000;

  if (!setup_done)
  {
    PIX.begin(led_strip_port, led_strip_pin); // set pin to output
    ++setup_done;
  }
  // fill pixel buffer
  hsb[2]=wheel_index;
  hsb[1]=0;
  hsb[0]=255/8;
  for (uint8_t i = 0; i < NUM_PIXELS; i++)
  {
    rwheel(&led_strip[i], wheel_index + i * (256 / NUM_PIXELS));
    attenuate(&led_strip[i], 8);
  }
  // output pixel buffer to string
  PIX.paint(led_strip[0].bytes, NUM_PIXELS, led_strip_port, led_strip_pin);
  SUI.RequestDisplayUpdate();

  // loop
  wheel_index++;
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
// enum { BRIGHTNESS, HUE, SATURATION } bhs;

void WS2812Ctrl::service(int event)
{
  //  const int LED_STRIP_RESET = 0;
  const int LED_STRIP_OFF = 0;
  const int LED_STRIP_ON = 1;
  const int LED_STRIP_WHEEL = 2;
  //static int led_strip_state = LED_STRIP_OFF; // OFF / ON / WHEEL
  static unsigned long buttonOnTime = 0;    // Time when button was pressed/Released
  // static unsigned long buttonOffTime=0; // Time when button was pressed/Released
  static int buttonState = WS2812_BUTTON_INVALID; // or LED_STRIP_ON
  static int buttonCount = 2;                        // Wraps to zero on first button press
  pixel pix;
  static int LEDStripUpdate = 2; // Update twice at startup to avoid residual led being lit
  // char buff[180];
  //  log scale derived from logscale[n] = 10^(log(255)-n*log(255)/32)
//  const uint8_t rev_log_scale[32] = {
//      255, 214, 180, 152, 128, 107, 90, 76, 64, 54, 45, 38, 32,
//      27, 23, 19, 16, 13, 11, 9, 8, 7, 6, 5, 4, 3, 3, 2, 2, 1, 1, 1};
  const uint8_t log_scale[32] = {
    1,1,1,2,2,3,3,4,5,6,7,8,9,11,13,16,19,23,27,
    32,38,45,54,64,76,90,107,128,152,180,214,255 };
  static unsigned long last_time = 0; // Last time this function was called
  if (getEnabled())
  {
//    const int WS2812_BUTTON_INVALID = -1;
    const int WS2812_BUTTON_OFF = 0;
    const int WS2812_BUTTON_ON = 1;
    const int WS2812_SERVICE = 2;

    unsigned long time_now = micros();
    int entry_led_strip_state = led_strip_state;
    switch (event)
    {
    case WS2812_BUTTON_ON:
      buttonOnTime = time_now;
      buttonState = event;
      ++buttonCount;
      // sprintf(buff,"LED_BUTTON_ON Exit buttonCount=%d\r\n",buttonCount);
      // CompositeSerial.write(buff);
      break;
    case WS2812_BUTTON_OFF:
      // buttonOffTime = time_now;
      buttonState = event;
      break;
    case WS2812_SERVICE:
    {
      if (time_now - last_time < 50000)
        return; // 50ms service interval
      last_time = time_now;
      if ((time_now - buttonOnTime) > 1000000)
      { // long interval since button pressed / released
        // long button on time?
        if (buttonState == WS2812_BUTTON_ON)
        {
          if (--buttonCount > 2)
            buttonCount = 2;
          // sprintf(buff,"LongTime BUTTON_ON Start bsh[%d]=%d\r\n", buttonCount,bsh[buttonCount]);
          // CompositeSerial.write(buff);
          led_strip_state = LED_STRIP_ON;
          LEDStripUpdate++;
          hsb[buttonCount]++; // increment one of hue, saturation or brightness
          hsb[buttonCount] &= 0x1ff;
          // int updown=bsh[buttonCount];
          // if ( updown > 255 ) updown = 511-updown;
          // sprintf(buff,"LongTime BUTTON_ON Exit bsh[%d]=%d (%d)\r\n", buttonCount,bsh[buttonCount],updown);
          // CompositeSerial.write(buff);
          ++buttonCount;
          SUI.RequestDisplayUpdate();
        }
        if (buttonState == WS2812_BUTTON_OFF)
        {
          // sprintf(buff,"Long time: LED_BUTTON_OFF Start LED StripState=%d buttonState=%d buttonCount=%d\r\n",led_strip_state,buttonState,buttonCount );
          // CompositeSerial.write(buff);
          switch (--buttonCount)
          {
          case 1:
            led_strip_state = LED_STRIP_WHEEL;
            break;
          default:
            if (led_strip_state == LED_STRIP_OFF)
            {
              led_strip_state = LED_STRIP_ON;
            }
            else
            {
              led_strip_state = LED_STRIP_OFF;
            }
            LEDStripUpdate++;
          }
          if ((time_now - buttonOnTime) > 1500000)
            led_strip_state = LED_STRIP_ON;
          buttonState = LED_STRIP_BUTTON_INVALID; // Inhibit timer calling again for OFF
          buttonCount = 0;
          // sprintf(buff,"Long time: LED_BUTTON_OFF Exit LED  StripState=%d buttonState=%d buttonCount=%d\r\n",led_strip_state,buttonState,buttonCount );
          // CompositeSerial.write(buff);
        }
      }
      if (led_strip_state == LED_STRIP_WHEEL)
      {
        colorWheel();
        SUI.RequestDisplayUpdate();
      }
      if (LEDStripUpdate)
      {
        SUI.RequestDisplayUpdate();
        --LEDStripUpdate;
        pix.rgb.r = 0;
        pix.rgb.g = 0;
        pix.rgb.b = 0;
        if (led_strip_state == LED_STRIP_ON)
        {
          // Load with hsb conversion
          int h = hsb[2];
          if (h > 255)
            h = 511 - h;

          int s = hsb[1] & 0x1ff;
          if (s > 255)
            s = 511 - s;

          int b = hsb[0] & 0x1ff;
          if (b > 255)
            b = 511 - b;
          // sprintf(buff,"Bri=%3d Sat=%3d Hue=%3d\r\n",log_scale[b>>3],log_scale[s>>3],h);
          // CompositeSerial.write(buff);
          RGBColor rgb = hsv2rgb(h, s, log_scale[b >> 3]);
          pix.rgb.r = rgb.r; // DEBUG
          pix.rgb.g = rgb.g;
          pix.rgb.b = rgb.b;
        }
        // fill pixel buffer
        for (uint8_t i = 0; i < NUM_PIXELS; i++)
        {
          led_strip[i].rgb.r = pix.rgb.r;
          led_strip[i].rgb.g = pix.rgb.g;
          led_strip[i].rgb.b = pix.rgb.b;
        }
        PIX.begin(led_strip_port, led_strip_pin); // set pin to output
        PIX.paint(led_strip[0].bytes, NUM_PIXELS, led_strip_port, led_strip_pin);
      }
    }
    break;
    }
    if ( entry_led_strip_state != led_strip_state )
      eventMonitor ( led_strip_state );
  }
}

void WS2812Ctrl::Print(void)
{
  if (getEnabled())
  {
    const char *state_names[] = {"OFF", "ON","Rainbow","Invalid"};
    char buf[120];
    CompositeSerial.write(ANSI_CLEAR);
    sprintf(buf, "WS22812 LED Strip = %s\r\n\r\n", state_names[led_strip_state&3]);
    CompositeSerial.write(buf);
             int h = hsb[2];
          if (h > 255)
            h = 511 - h;

          int s = hsb[1] & 0x1ff;
          if (s > 255)
            s = 511 - s;

          int b = hsb[0] & 0x1ff;
          if (b > 255)
            b = 511 - b;

    sprintf(buf,"%3d=Red    %3d=Hue\r\n%3d=Green  %3d=Saturation\r\n%3d=Blue   %3d=Brightness\r\n",
      led_strip[0].rgb.r,
      h,    
      led_strip[0].rgb.g,
      s,    
      led_strip[0].rgb.b,
      b);
    CompositeSerial.write(buf);  }
  else
  {
    CompositeSerial.write("WS2812 LED String Not Enabled\r\n");
  }
}
