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
  PROFILE_SYSEX,
  PROFILE_PPRINT,
  PROFILE_SPARE } ;

class PROFILE {
  private:
  struct { int count ; unsigned long pstart; unsigned long paverage; unsigned long ppeak ; } times[PROFILE_SPARE+1];
  const char *itemText[PROFILE_SPARE+1] = 
  {
    (const char *)"Keyboard",
    (const char *)"Buttons",
    (const char *)"ADC",
    (const char *)"WS2812 LED Strip",
    (const char *)"SysEx LCD & 7 Seg",
    (const char *)"Profile Print!",
    (const char *)"Spare"
  };
  public:
  void PReset ( void ) {
    char buf[80];
    sprintf(buf,"Profile Reset\r\n");
    CompositeSerial.write(buf);
    for ( int i = 0 ; i <= PROFILE_SPARE ; i++ ) {
        times[i].count = 0;
        times[i].paverage = 0;
        times[i].ppeak = 0;
    }
    times[PROFILE_PPRINT].count = -1 ;
  }
  void PPrint ( void ) 
  { 
    char buf[80];
    sprintf(buf,"Execution Times in micro seconds\r\nCount Mean   Max  Feature\r\n");
    CompositeSerial.write(buf);
    for ( int i = 0 ; i <= PROFILE_SPARE ; i++ ) {
      if (times[i].count > 99999 ) times[i].count=0;
      sprintf(buf,"%5d %5ld %5ld  %s\r\n",times[i].count,times[i].paverage,times[i].ppeak, itemText[i]);
      CompositeSerial.write(buf);
    }
  }

  void PStart ( int index ) {
    times[index].count++;
    times[index].pstart = micros();
  }
  void PEnd ( int index ) {
    unsigned long duration = micros() - times[index].pstart;
    if ( times[index].paverage == 0 ) {
         times[index].paverage = duration ; // seed average time
    } else {
      times[index].paverage = times[index].paverage*31/32 + duration/32;
      if ( times[index].ppeak < duration ) {
        times[index].ppeak = duration;
      }
    }
  }
};

#ifdef __cplusplus
}
#endif 


#endif /* __PROFILE_H */
