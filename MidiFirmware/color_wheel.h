#ifndef __COLOR_WHEEL_H
#define __COLOR_WHEEL_H


#ifdef __cplusplus
extern "C" {
#endif


void ColorWheel(void);
void LEDStripCtrl(int event );
const int LED_BUTTON_INVALID = -1;
const int LED_BUTTON_OFF = 0;
const int LED_BUTTON_ON  = 1;
const int LED_STRIP_SERVICE = 2;
//extern USBCompositeSerial CompositeSerial;

#ifdef __cplusplus
}
#endif

#endif /* __USB_SERIAL_UI_H */
