#ifndef __WS2812_H
#define __WS2812_H
#include "bluepill_ws2812.h"
#ifdef __cplusplus
extern "C"
{
#endif

  //  WS2812Ctrl.h

  //  Copyright (C)2025 richard (dot) jones (dot ) 1952 ( at ) gmail (dot ) com

  // License MIT

const int WS2812_BUTTON_INVALID = -1;
const int WS2812_BUTTON_OFF = 0;
const int WS2812_BUTTON_ON = 1;

const int LED_STRIP_BUTTON_INVALID = -1;
const int LED_STRIP_BUTTON_OFF = 0;
const int LED_STRIP_BUTTON_ON  = 1;
const int LED_STRIP_SERVICE = 2;


  class WS2812Ctrl
  {
  private:
    int hsb[3] = {120, 120, 0}; // not too bright white (hue:0-255,sat:0-255,brightness:0-255)

    int led_strip_state;
    bluepill_neopixel PIX; // a strip of pixels
    // 65 max pixels without disrupting micros() figures
    static const int NUM_PIXELS = 120;       //   number of pixels in the string max 65 for all timer interrupts
    pixel led_strip[NUM_PIXELS];      //   rgb data buffer
    const int led_strip_port = GPIOA; // port string is connected to
    const int led_strip_pin = 13;     // gpio pin string is connected to
    void attenuate(pixel *p, uint8_t factor);
    int getEnabled(void);
    void rwheel(pixel *p, uint8_t rot);
    void colorWheel(void);

  public:
    const int WS2812_BUTTON_INVALID = -1;
    const int WS2812_BUTTON_OFF = 0;
    const int WS2812_BUTTON_ON = 1;
    const int WS2812_STRIP_SERVICE = 2;
    WS2812Ctrl(void);
    void eventMonitor(int event);
    void Print(void);
    void service(int event);
  };

extern WS2812Ctrl ws2812Ctrl;

#ifdef __cplusplus
}
#endif

#endif /* __WS2812Ctrl_H */
