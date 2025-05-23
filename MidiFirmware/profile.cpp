#include <Arduino.h>
#include <USBComposite.h>
#include "profile.h"
 
 void PROFILE::PReset ( void ) {
    char buf[80];
    sprintf(buf,"Profile Reset\r\n");
    CompositeSerial.write(buf);
    for ( int i = 0 ; i <= PROFILE_SPARE ; i++ ) {
        times[i].count = 0;
        times[i].pperiod = 0;
        times[i].paverage = 0;
        times[i].ppeak = 0;
    }
    times[PROFILE_PPRINT].count = -1 ;
  }
  void PROFILE::PPrint ( void ) 
  { 
    char buf[80];
    sprintf(buf,"\nExecution Times in microseconds\r\n\n   Count Interval AvgTime MaxTime  Feature\r\n");
    //                                                  123456781234567891234567812345678
    CompositeSerial.write(buf);
    for ( int i = 0 ; i <= PROFILE_SPARE ; i++ ) {
      if (times[i].count > 9999999L ) times[i].count=0;
      sprintf(buf,"%8ld%9ld%8ld%8ld  %s\r\n",
        times[i].count,
        times[i].pperiod,
        times[i].paverage,
        times[i].ppeak,
        itemText[i]);
      CompositeSerial.write(buf);
    }
  }

  void PROFILE::PStart ( int index ) {
    times[index].count++;
    times[index].pstart = micros();
  }

  // Note the end time and calculate average period and durations
  void PROFILE::PEnd ( int index ) { // Require a new start. GO may send multiple sysex
   if ( times[index].pstart ) {
    unsigned long time_now = micros();
    long duration = time_now - times[index].pstart;
    if ( times[index].paverage == 0 ) {
         //times[index].pperiod = times[index].pend; ; // seed interval time
         times[index].paverage = duration ; // seed average time
    }
    long period = time_now-times[index].pend;
    if ( period > 9999999L ) 
         period = 9999999L;
    //times[index].pperiod  = (times[index].pperiod*31L/32L) + (period/32L); // Note integer rounding errors
    //times[index].paverage = (times[index].paverage*31/32) + (duration/32);
    times[index].pperiod  = period; // Instantaneous values seem to give a better idea of what is going on
    times[index].paverage = duration;
    if ( times[index].ppeak < duration ) {
        times[index].ppeak = duration;
    }
    times[index].pend = time_now;
    times[index].pstart = 0; 
   }
  }
