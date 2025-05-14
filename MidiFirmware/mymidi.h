#ifndef __MYMIDI_H
#define __MYMIDI_H

#ifdef __cplusplus
extern "C" {
#endif


class myMidi : public USBMIDI {
  private:
    char sysexBuf[80];
    unsigned int sysexIndex=0;
    unsigned long last_time=0;    
  public:
    virtual void handleNoteOff(unsigned int channel, unsigned int note, unsigned int velocity) ;
    virtual void handleNoteOn( unsigned int channel, unsigned int note, unsigned int velocity ) ;
    virtual void handleControlChange( unsigned int channel, unsigned int controller, unsigned int velocity ); 
//    int getChannelNoteIndex( unsigned int command, unsigned int channel, unsigned int note );
    virtual void handleSysExData(unsigned char ExData);
    virtual void handleSysExEnd(void);

};

extern myMidi midi;

#ifdef __cplusplus
}
#endif
#endif /* __MYMIDI_H */
