#ifndef __USB_SERIAL_UI_H
#define __USB_SERIAL_UI_H


#ifdef __cplusplus
extern "C" {
#endif

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
    struct Image {
      int Input;
      unsigned long Time;
      int Output;
      int Error;
    };
    Image ShiftRegImage[128];
    const int Octave = 12; // half steps


    struct ConfigItem {
      int maxval;
      char *text;
    } ;


    ConfigItem const ConfigItems[11] = {
      { 15, "Keyboard Midi Channel"},  // A
      { 10, "Number of ADC Inputs"}, // B
      {  1, "Pedal Board"} ,         // C
      {  1, "PCF8574 LCDs 16x2 I2C" }, // D
      {  1, "TM1637 6 Digit,7 Segment Display" }, // E
      {  1, "Shift Register Based Buttons & LEDs"}, // F
      {  1, "Shift Register LED invert", },  // G
      {  1, "Use PC13 as scan output and LED"},  // H
      {  1, "Use Boot 1, and Debug Connector as Outputs PB2,PA13,PA14"}, // I
      {  1, "Event Log to USB Serial (may slow response time)"}, // J
      {  0, "" }
    };
    const char *function_text[27] = {
      "Spare",
      "", // Reserved
      "OP_LED",
      "OP_SCAN",
      "ScanInput",
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
      "I2C_SDA", // I2C_SDA"
      "I2C_SCL",
      "USB-",
      "USB+",
      "V5",
      "VBAT",
      "V3_3",
      "GND",
      "SWCLK",
      "SWDIO",
      "BOOT1",
      "NRST"
    };

    typedef struct GPIOPinConfig {
      int function;
      int keyboard;
      int count;
      int input;
      int error;
      char fault;
    } GPIOPinConfig;

    GPIOPinConfig MenuConfigs [NUM_GPIO_PINS];
    GPIOPinConfig LiveConfigs [NUM_GPIO_PINS];
    enum  { DEV_UNKNOWN,DEV_KEYBOARD,DEV_BUTTON,DEV_LED } devices;
    const char * device_names[4] = {"Unknown", "Keyboard","Button","LED"};
  const char *note_name[12] = {
    "C","C♯","D","D♯","E","F","F♯","G","G♯","A","A♯","B"
  };
  const int OCTAVE = 12;


  public:
      typedef union {
      unsigned Word;

      struct {
        unsigned midiChannelOffset : 4; // A  4
        unsigned numberADCInputs : 4;   // B  8
        unsigned hasPedalBoard : 1;     // C  9
        unsigned hasI2C: 1;             // D 10
        unsigned hasTM1637 : 1;         // E 11
        unsigned hasShiftRegs: 1;       // F 12
        unsigned hasShiftRegLEDInvert:1;// G 13
        unsigned hasPC13Scan: 1;        // H 14
        unsigned hasPB2PA13PA14Scan: 1; // I 15
        unsigned hasEventLog: 1;        // J 16
      } Bits;
    } Config;

    Config Cfg;

    USBSerialUI(void);
    void poll(void);
    void setFault( int pin_id , char fault );
    void handleNoteOff( unsigned int channel, unsigned int note, unsigned int velocity );
    void handleNoteOn ( unsigned int channel, unsigned int note, unsigned int velocity );
    void handleControlChange( unsigned int channel, unsigned int controller, unsigned int velocity );
    int  hasLCD(void){ return Cfg.Bits.hasTM1637; };
    int  midiKeyboardChannel(void);
    int  midiButtonChannel(void);
    

  private:
    int  AppOnline(void);
    void ButtonScan(void);
    void CommandCharDecode(char command);
    void DisplayConfigurationMenu(void);
    void DisplayFlashSummary(void);
    void DisplayFunctionPinOut(void);
    void DisplayKeyboardContacts(void);
    void DisplayMenu(void);
    void DisplayPrompt(char *prompt );
    void DisplayShiftRegisterIO(void);
    void DisplayStatus(void);
    void DisplayTitle( char *title );
    void fillCfgPinData( unsigned ConfigWord , GPIOPinConfig *newCfgs );
//    void OldgetFunctionText(int gpio_index, char *buff, int n ) ;
    unsigned getFlashConfig(void);
    char * getFunctionText(GPIOPinConfig *pincfg, char *buff, int n ) ;
    void GPIOScan(void);
    void LCDTest(void);
    void monitorNoteOn ( unsigned int channel, unsigned int note, unsigned int velocity, unsigned int device );
    void monitorNoteOff( unsigned int channel, unsigned int note, unsigned int velocity, unsigned int device );
    void MusicKeyboardScan(void);
    void RestoreConfigFromFlash();
    void RequestDisplayUpdate(void) { DisplayUpdate = 1; } 
    void SaveConfigToFlash( char c );
    void TestIO(void);
};

extern USBSerialUI SUI;

#ifdef __cplusplus
}
#endif


#endif /* __USB_SERIAL_UI_H */
