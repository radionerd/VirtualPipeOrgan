#ifndef __USB_SERIAL_UI_H
#define __USB_SERIAL_UI_H


#ifdef __cplusplus
extern "C" {
#endif

#include "pin.h"

const char VT100_CLEAR[] = "\x1B[2J";
const char VT100_ERASE_DOWN[] = "\x1B[J";
const char VT100_CLR_EOL[] = "\x1B[0K";
const char VT100_CURSOR_00[] = "\x1B[0;0H";
const char VT100_UNDERLINE[] = "\x1B[4m";
const char VT100_NO_UNDERLINE[] = "\x1B[0m";

class USBSerialUI {
    unsigned long AppOnlineTime=0;
    int ConfigValue[20];
    int DisplayUpdate;
    char LastCommand;
//    const unsigned int Octave = 12; // half steps
    const int MAX_15 = 15;
    const int MAX_8  =  8;
    const int MAX_1  =  1;

    struct ConfigItem {
      int maxval;
      const char *text;
    } ;
   

    ConfigItem const ConfigItems[11] = {
      { MAX_15,(const char *)"Key/Pedal Midi Channel"},  // A
      { MAX_1, (const char *)"Pedalboard"} ,//B
      { MAX_8, (const char *)"  Number of ADC Inputs PA2-PB1"}, // C
      { MAX_1, (const char *)"Keyboard/Pedalboard Velocity Reporting"} ,//D     
      { MAX_1, (const char *)"PCF8574 I2C LCDs 16x2" }, //E
      { MAX_1, (const char *)"TM1637 7 segment, 6 digit display" }, // F
      { MAX_1, (const char *)"74HC164 Button LED backlight enabled"}, // G
      { MAX_1, (const char *)"74HC164 Button LED inverted", },  // H
      { MAX_1, (const char *)"Use Debug Connector for WS2812 RGB LEDs PA13,PA14"}, // I
      { MAX_1, (const char *)"Event Log to USB Serial (may slow response time)"}, // J
      {     0, (const char *)"" }
    };
    const char *function_text[28] = {
      "Spare",
      "", // Reserved
      "OP_LED",
      "Scan Out",
      "Scan In ",
      "IP_ADC ",
      "IP_ADC_LED_DIM",
      "OP_SR_CLOCK",
      "OP_SR_DATA",
      "OP_SR_ENA",
      "IP_SR_DATA",
      "TM1637_CK",
      "TM1637_DA",
      "CommonData",
      "IP_CONTACT",
      "ShareI2C_SDA",
      "LCD  I2C_SCL",
      "USB-",
      "USB+",
      "V5",
      "VBAT",
      "V3_3",
      "GND",
      "SWCLK",
      "SWDIO",
      "BOOT1",
      "NRST",
      "WS2812 LEDs"
    };

    const char * device_names[5] = {"Unknown", "Pedalboard","Keyboard","Button","Button LED"};
  const char *note_name[12] = {
    "C","C♯","D","D♯","E","F","F♯","G","G♯","A","A♯","B"
  };
  const unsigned int OCTAVE = 12;


  public:
    enum  { DEV_UNKNOWN,DEV_PEDALBOARD,DEV_KEYBOARD,DEV_BUTTON,DEV_LED } devices;
    typedef struct // GPIOPinConfig 
    {
      int function;
      int keyboard;
      int count;
      int input;
      int error;
      char fault;
    } GPIOPinConfig;
#define MenuConfigs LiveConfigs
    //GPIOPinConfig MenuConfigs [NUM_GPIO_PINS];
    GPIOPinConfig LiveConfigs [NUM_GPIO_PINS];
    //PROFILE profile;

    typedef union {
      unsigned Word;

      struct {
        unsigned midiChannel : 4;       // A  4
        unsigned hasPedalBoard : 1;     // B  5
        unsigned numberADCInputs : 4;   // C  9
        unsigned hasKeyVelocity: 1;     // D 10
        unsigned hasI2C: 1;             // E 11
        unsigned hasTM1637 : 1;         // F 12
        unsigned hasButtonLEDBacklit: 1;// G 13
        unsigned hasButtonLEDInvert:1;  // H 14
        unsigned hasPB2PA13PA14Scan: 1; // I 15
        unsigned hasEventLog: 1;        // J 16
      } Bits;
    } Config;
    Config Cfg;

    USBSerialUI(void);
    unsigned long GetAppOnlineTime(void) { return AppOnlineTime; };
    void poll(void);
    void setFault( int pin_id , char fault );
    void handleNoteOff( unsigned int channel, unsigned int note, unsigned int velocity );
    void handleNoteOn ( unsigned int channel, unsigned int note, unsigned int velocity );
    void handleControlChange( unsigned int channel, unsigned int controller, unsigned int velocity );
    int  hasLCD(void){ return Cfg.Bits.hasTM1637; };
    void RequestDisplayUpdate(void) ;
    void monitorNoteOn ( unsigned int channel, unsigned int note, unsigned int velocity, unsigned int device );
    void monitorNoteOff( unsigned int channel, unsigned int note, unsigned int velocity, unsigned int device );
    void DisplayTitle( const char *title );
    

  private:
    void ADCBegin(void);
    void ADCScan(void);
    int  AppOnline(void);
    void ButtonScan(void);
    void CommandCharDecode(char command);
    void DisplayConfigurationMenu(void);
    void DisplayFlashSummary(void);
    void DisplayFunctionPinOut(void);
    void DisplayKeyboardContacts(void);
    void DisplayMenu(void);
    void DisplayPrompt(const char *prompt );
    void DisplayShiftRegisterIO(void);
    void DisplayStatus(void);
    void fillCfgPinData( unsigned ConfigWord , GPIOPinConfig *newCfgs );
//    void OldgetFunctionText(int gpio_index, char *buff, int n ) ;
    unsigned getFlashConfig(void);
    char * getFunctionText(GPIOPinConfig *pincfg, char *buff, int n ) ;
    void GPIOScan(void);
    void LCDTest(void);
    void MusicKeyboardScan(bool pedalboard);
    void RestoreConfigFromFlash();
    void SaveConfigToFlash( char c );
    void TestIO(void);
};

extern USBSerialUI SUI;
//extern GPIOPinConfig LiveConfigs [NUM_GPIO_PINS];


#ifdef __cplusplus
}
#endif


#endif /* __USB_SERIAL_UI_H */
