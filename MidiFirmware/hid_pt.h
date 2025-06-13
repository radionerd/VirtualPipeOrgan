#ifndef __HID_PT_H
#define __HID_PT_H


#ifdef __cplusplus
extern "C" {
#endif


//  HID.h 

//  Copyright (C)2025 richard (dot) jones (dot ) 1952 ( at ) gmail (dot ) com

// License MIT

class HID_PT {
  private:
    //const int KEY_HOME      = 0xD2;
    //const int KEY_PAGE_UP   = 0xD3;
    //const int KEY_END       = 0xD5;
    //const int KEY_PAGE_DOWN = 0xD6;
    const int ON  = 0;
    const int OFF = 1;
    const int LONG_PRESS_OFFSET = 3;
    const unsigned long LONG_PRESS_US = 1000000L;
    const int hid_commands[4] = {KEY_PAGE_DOWN,KEY_PAGE_UP,KEY_END,KEY_HOME};
    const char  * hid_text[4] = {    "PageDown",   "PageUp",  "Ctrl+End",  "Ctrl+Home"};
    const int INVALID = -1;

    int hid_pin;
    int last_state=OFF;
    int long_count;
    int short_count;
    unsigned long time_off;
    unsigned long time_on;
    unsigned long time_repeat;
  public:
   HID_PT ( int pin );
   int  getEnabled(void);
   void eventMonitor( int hid_code );
   void Print ( void );
   void service(void);
};
extern HIDKeyboard QKeyboard;

#ifdef __cplusplus
}
#endif


#endif /* __HID_PT_H */
