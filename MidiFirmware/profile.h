#ifndef __PROFILE_H
#define __PROFILE_H


#ifdef __cplusplus
 extern "C" {
#endif

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
  } ;

class PROFILE {
  private:

  struct { 
    unsigned long int count ; 
    unsigned long pstart; 
    unsigned long pend;
    unsigned long pperiod; 
    unsigned long paverage; 
    unsigned long ppeak ; 
  } times[PROFILE_SPARE+1];
 
  const char *itemText[PROFILE_SPARE+1] = 
  {
    (const char *)"Music Keyboard",
    (const char *)"SerialUI",
    (const char *)"ADC",
    (const char *)"WS2812 LED Strip",
    (const char *)"Buttons",
    (const char *)"Midi Poll",
    (const char *)"Sysex",
    (const char *)"Midi Out to SysEx In",
    (const char *)"Loop", 
    (const char *)"This Print",
    (const char *)"Spare"
  };
  
  public:
  void PReset ( void );
  void PPrint ( void );
  void PStart ( int index );
  void PEnd   ( int index );

};

const int PID_TIME_OFFSET_US = 1000;

extern PROFILE profile;

#ifdef __cplusplus
}
#endif 


#endif /* __PROFILE_H */
