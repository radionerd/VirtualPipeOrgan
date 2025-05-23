#ifndef __PROFILE_H
#define __PROFILE_H


#ifdef __cplusplus
 extern "C" {
#endif

enum item { 
  PROFILE_KEYBOARD=0, 
  PROFILE_BUTTONS, 
  PROFILE_ADC, 
  PROFILE_WS2812, 
  PROFILE_MIDI_OUT_TO_SYSEX_IN,
  PROFILE_SYSEX,
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
    (const char *)"Keyboard",
    (const char *)"Buttons",
    (const char *)"ADC",
    (const char *)"WS2812 LED Strip",
    (const char *)"Midi Out to SysEx In",
    (const char *)"SysEx LCD or 7 Seg",
    (const char *)"Loop", 
    (const char *)"Print!",
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
