#include <Arduino.h>
#include <USBComposite.h>
#include "bluepill_ws2812.h"
#include "hid_pt.h"
#include "keyboardscan.h"
#include "led.h"
#include "mymidi.h"
#include "profile.h"
#include "USBSerialUI.h"
#include "ws2812ctrl.h"

extern myMidi midi;
uint32_t kb_input [NUM_KEYBOARD_INPUTS]; // 16 input bits per output line
uint32_t kb_image [NUM_KEYBOARD_INPUTS];
unsigned long kb_time[NUM_KEYBOARD_INPUTS*NUM_KEYBOARD_OUTPUTS]; // For velocity measurment
//extern HIDKeyboard QKeyboard;
extern HID_PT hid_pt;

KeyboardScan::KeyboardScan(){

/*  pinMode(PB13,INPUT_PULLDOWN); // data in 0x01
  pinMode(PB14,INPUT_PULLDOWN); // data in 0x02
  pinMode(PB15,INPUT_PULLDOWN); // data in 0x04
  pinMode(PA8 ,INPUT_PULLDOWN); // data in 0x08
  
  pinMode(PA9 ,INPUT_PULLDOWN); // data in 0x10
  pinMode(PA10,INPUT_PULLDOWN); // data in 0x20
  pinMode(PA15,INPUT_PULLDOWN); // data in 0x40
  pinMode(PB3 ,INPUT_PULLDOWN); // data in 0x80

  pinMode(PB1 ,INPUT_PULLDOWN); // data in 0x0100
  pinMode(PB0 ,INPUT_PULLDOWN); // data in 0x0200
  pinMode(PA7 ,INPUT_PULLDOWN); // data in 0x0400
  pinMode(PA6 ,INPUT_PULLDOWN); // data in 0x0800

  pinMode(PA5 ,INPUT_PULLDOWN); // data in 0x1000
  pinMode(PA4 ,INPUT_PULLDOWN); // data in 0x2000
  pinMode(PA3 ,INPUT_PULLDOWN); // data in 0x4000
  pinMode(PA2 ,INPUT_PULLDOWN); // data in 0x8000

  pinMode(PB4 ,OUTPUT);
  pinMode(PB5 ,OUTPUT);
  pinMode(PB8 ,OUTPUT);
  pinMode(PB9 ,OUTPUT);

  pinMode(PA1 ,OUTPUT);
  pinMode(PA0 ,OUTPUT);
  pinMode(PC15,OUTPUT);
  pinMode(PC14,OUTPUT); */ 
}
uint32_t KeyboardScan::get_input(bool pedalboard, bool hasKeyVelocity ) {
  const volatile uint32_t* IN_0X0001 = (uint32_t*) (BB + ( (PORTB+IDR)<<5 ) + ((uint32_t) 13 << 2 ));
  const volatile uint32_t* IN_0X0002 = (uint32_t*) (BB + ( (PORTB+IDR)<<5 ) + ((uint32_t) 14 << 2 ));
  const volatile uint32_t* IN_0X0004 = (uint32_t*) (BB + ( (PORTB+IDR)<<5 ) + ((uint32_t) 15 << 2 ));
  const volatile uint32_t* IN_0X0008 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  8 << 2 ));

  const volatile uint32_t* IN_0X0010 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  9 << 2 ));
  const volatile uint32_t* IN_0X0020 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t) 10 << 2 ));
  const volatile uint32_t* IN_0X0040 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t) 15 << 2 ));
  const volatile uint32_t* IN_0X0080 = (uint32_t*) (BB + ( (PORTB+IDR)<<5 ) + ((uint32_t)  3 << 2 ));

  const volatile uint32_t* IN_0X0100 = (uint32_t*) (BB + ( (PORTB+IDR)<<5 ) + ((uint32_t)  1 << 2 ));
  const volatile uint32_t* IN_0X0200 = (uint32_t*) (BB + ( (PORTB+IDR)<<5 ) + ((uint32_t)  0 << 2 ));
  const volatile uint32_t* IN_0X0400 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  7 << 2 ));
  const volatile uint32_t* IN_0X0800 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  6 << 2 ));

  const volatile uint32_t* IN_0X1000 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  5 << 2 ));
  const volatile uint32_t* IN_0X2000 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  4 << 2 ));
  const volatile uint32_t* IN_0X4000 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  3 << 2 ));
  const volatile uint32_t* IN_0X8000 = (uint32_t*) (BB + ( (PORTA+IDR)<<5 ) + ((uint32_t)  2 << 2 ));

  if ( ! pedalboard ) {
    if ( hasKeyVelocity ) {
      return ((*IN_0X0001) <<  0 ) +
             ((*IN_0X0002) <<  1 ) +
             ((*IN_0X0004) <<  2 ) +
             ((*IN_0X0008) <<  3 ) +
             ((*IN_0X0010) <<  4 ) +
             ((*IN_0X0020) <<  5 ) +
             ((*IN_0X0040) <<  6 ) +
             ((*IN_0X0080) <<  7 ) +
             ((*IN_0X0100) <<  8 ) +
             ((*IN_0X0200) <<  9 ) +
             ((*IN_0X0400) << 10 ) +
             ((*IN_0X0800) << 11 ) +
             ((*IN_0X1000) << 12 ) +
             ((*IN_0X2000) << 13 ) +
             ((*IN_0X4000) << 14 ) +
             ((*IN_0X8000) << 15 );
    } else {
      return ((*IN_0X0001) <<  0 ) +
            // ((*IN_0X0002) <<  1 ) +
             ((*IN_0X0004) <<  2 ) +
            // ((*IN_0X0008) <<  3 ) +
             ((*IN_0X0010) <<  4 ) +
            // ((*IN_0X0020) <<  5 ) +
             ((*IN_0X0040) <<  6 ) +
            // ((*IN_0X0080) <<  7 ) +
             ((*IN_0X0100) <<  8 ) +
            // ((*IN_0X0200) <<  9 ) +
             ((*IN_0X0400) << 10 ) +
            // ((*IN_0X0800) << 11 ) +
             ((*IN_0X1000) << 12 ) +
            // ((*IN_0X2000) << 13 ) +
             ((*IN_0X4000) << 14 ) ;
            // ((*IN_0X8000) << 15 );           
    }
  } else {
//    if ( hasKeyVelocity ) {
    return ((*IN_0X0001) <<  0 ) +
           ((*IN_0X0002) <<  1 ) +
           ((*IN_0X0004) <<  2 ) +
           ((*IN_0X0008) <<  3 ) +
           ((*IN_0X0010) <<  4 ) +
           ((*IN_0X0020) <<  5 ) +
           ((*IN_0X0040) <<  6 ) +
           ((*IN_0X0080) <<  7 ) ;
//    } else {
//    return ((*IN_0X0001) <<  0 ) +
           //((*IN_0X0002) <<  1 ) +
//           ((*IN_0X0004) <<  2 ) +
           //((*IN_0X0008) <<  3 ) +
//           ((*IN_0X0010) <<  4 ) +
           //((*IN_0X0020) <<  5 ) +
//           ((*IN_0X0040) <<  6 ) ;
           //((*IN_0X0080) <<  7 ) 
      
//    }
  }
}

// Measured 
// ~17us inactive inputs
// ~+24us to scan all inputs with active pull down
// Each scan line active for 1.84us
uint32_t KeyboardScan::FastHWScan(uint32_t *kb_input,uint32_t*kb_image, bool pedalboard )
{
//  static unsigned long lastTime=0;
//  unsigned long time_now = micros();
//  if ( time_now - lastTime < 1000 ) return 0; // 1ms scan intervals to reduce emi
//  lastTime=time_now;
  volatile uint32_t* OUT1 = (uint32_t*) (BB + ( (PORTB+ODR)<<5 ) + ((uint32_t)  4 << 2 ));
  volatile uint32_t* OUT2 = (uint32_t*) (BB + ( (PORTB+ODR)<<5 ) + ((uint32_t)  5 << 2 ));
  volatile uint32_t* OUT3 = (uint32_t*) (BB + ( (PORTB+ODR)<<5 ) + ((uint32_t)  8 << 2 ));
  volatile uint32_t* OUT4 = (uint32_t*) (BB + ( (PORTB+ODR)<<5 ) + ((uint32_t)  9 << 2 ));

  volatile uint32_t* OUT5 = (uint32_t*) (BB + ( (PORTA+ODR)<<5 ) + ((uint32_t)  1 << 2 ));
  volatile uint32_t* OUT6 = (uint32_t*) (BB + ( (PORTA+ODR)<<5 ) + ((uint32_t)  0 << 2 ));
  volatile uint32_t* OUT7 = (uint32_t*) (BB + ( (PORTC+ODR)<<5 ) + ((uint32_t) 15 << 2 ));
  volatile uint32_t* OUT8 = (uint32_t*) (BB + ( (PORTC+ODR)<<5 ) + ((uint32_t) 14 << 2 ));

  volatile uint32_t* Outputn[] = { OUT1,OUT2,OUT3,OUT4,OUT5,OUT6,OUT7,OUT8 };
  int change = 0;

  // DEBUG If active input re-initialise, caused by Test command?
  /*if ( get_input() )*/ {
    pinMode(PB13,INPUT_PULLDOWN); // data in 0x01
    pinMode(PB14,INPUT_PULLDOWN); // data in 0x02
    pinMode(PB15,INPUT_PULLDOWN); // data in 0x04
    pinMode(PA8 ,INPUT_PULLDOWN); // data in 0x08
  
    pinMode(PA9 ,INPUT_PULLDOWN); // data in 0x10
    pinMode(PA10,INPUT_PULLDOWN); // data in 0x20
    pinMode(PA15,INPUT_PULLDOWN); // data in 0x40
    pinMode(PB3 ,INPUT_PULLDOWN); // data in 0x80

    if ( ! pedalboard ) {
      pinMode(PB1 ,INPUT_PULLDOWN); // data in 0x0100
      pinMode(PA7 ,INPUT_PULLDOWN); // data in 0x0400

      pinMode(PA5 ,INPUT_PULLDOWN); // data in 0x1000
      pinMode(PA3 ,INPUT_PULLDOWN); // data in 0x4000
     if ( SUI.Cfg.Bits.hasKeyVelocity ) {
       pinMode(PB0 ,INPUT_PULLDOWN); // data in 0x0200
       pinMode(PA6 ,INPUT_PULLDOWN); // data in 0x0800
       pinMode(PA4 ,INPUT_PULLDOWN); // data in 0x2000
       pinMode(PA2 ,INPUT_PULLDOWN); // data in 0x8000
     }
    }

    pinMode(PB4 ,OUTPUT);
    pinMode(PB5 ,OUTPUT);
    pinMode(PB8 ,OUTPUT);
    pinMode(PB9 ,OUTPUT);

    pinMode(PA1 ,OUTPUT);
    pinMode(PA0 ,OUTPUT);
    pinMode(PC15,OUTPUT);
    pinMode(PC14,OUTPUT);  
   
  }
  bool hasKeyVelocity = SUI.Cfg.Bits.hasKeyVelocity;
  for ( int scan_line = 0 ; scan_line < NUM_KEYBOARD_OUTPUTS ; scan_line++ ) {
    *Outputn[scan_line] = 1; // raise output scan line
    volatile uint32_t input = get_input(pedalboard,hasKeyVelocity); // read 16 input lines
    *Outputn[scan_line] = 0 ; // lower output scan line
    kb_input[scan_line] = input;
    if ( kb_input[scan_line] != kb_image[scan_line] ) {
      change++;
    }
    if ( input ) {
      activePulldown(pedalboard,hasKeyVelocity); // Pull down all used inputs
      activePulldown(pedalboard,hasKeyVelocity); // Restore all used inputs
    }
  }
  return change;
}

// Code generator for fast discharge of active input wires
void generateOption(bool hasPedalBoard,bool hasKeyVelocity)
{
      char buff[120];      
      //bool hasPedalBoard  = SUI.Cfg.Bits.hasPedalBoard;
      //bool hasKeyVelocity = SUI.Cfg.Bits.hasKeyVelocity;

      int info = 0;
      const int input_pin [] ={PB13,PB14,PB15,PA8,PA9,PA10,PA15,PB3,PB1,PB0,PA7,PA6,PA5,PA4,PA3,PA2 };
      //                          0    1    2   3   4    5    6   7   8   9   a   b   c   d   e   f
      int nip = 16 ;
      if ( hasPedalBoard ) nip = 8 ;
      for (int ip = 0 ; ip < nip ; ip++ ) {
        if ( ( hasKeyVelocity == 1 ) || ( (ip&1)==0 ) )
        pinMode( input_pin[ip],INPUT_PULLDOWN);
      }
      if ( info ) {
        sprintf(buff,"Code Generator for fast active input line pull down\r\n(Saves pull down resistors and time\r\n"); 
        CompositeSerial.write(buff);      
        //sprintf(buff,"pedalboard=%d\r\n",hasPedalBoard);
        //CompositeSerial.write(buff);
        sprintf(buff,"&GPIOA->regs->CRL = %08lx \r\n", (unsigned long)&GPIOA->regs->CRL);
        CompositeSerial.write(buff);
        sprintf(buff," GPIOA->regs->CRL = %08lx \r\n", GPIOA->regs->CRL);
        CompositeSerial.write(buff);
        sprintf(buff,"&GPIOA->regs->CRH = %08lx \r\n", (unsigned long)&GPIOA->regs->CRH);
        CompositeSerial.write(buff);
        sprintf(buff," GPIOA->regs->CRH = %08lx \r\n", GPIOA->regs->CRH);
        CompositeSerial.write(buff);
        sprintf(buff,"&GPIOB->regs->CRL = %08lx \r\n", (unsigned long)&GPIOB->regs->CRL);
        CompositeSerial.write(buff);
        sprintf(buff," GPIOB->regs->CRL = %08lx \r\n", GPIOB->regs->CRL);
        CompositeSerial.write(buff);
        sprintf(buff,"&GPIOB->regs->CRH = %08lx \r\n", (unsigned long)&GPIOB->regs->CRH);
        CompositeSerial.write(buff);
        sprintf(buff," GPIOB->regs->CRH = %08lx \r\n\n", GPIOB->regs->CRH);
        CompositeSerial.write(buff);
      }

      for (int ip = 0 ; ip < nip ; ip++ ) {
        if ( ( hasKeyVelocity == 1 ) || ( (ip&1)==0 ) )
        pinMode( input_pin[ip],OUTPUT);
      }
      unsigned long acrl = GPIOA->regs->CRL;
      unsigned long acrh = GPIOA->regs->CRH;
      unsigned long bcrl = GPIOB->regs->CRL;
      unsigned long bcrh = GPIOB->regs->CRH;
      if ( info ) {
        sprintf(buff,"&GPIOA->regs->CRL = %08lx \r\n", (unsigned long)&GPIOA->regs->CRL);
        CompositeSerial.write(buff);
        sprintf(buff," GPIOA->regs->CRL = %08lx \r\n", acrl=GPIOA->regs->CRL);
        CompositeSerial.write(buff);
        sprintf(buff,"&GPIOA->regs->CRH = %08lx \r\n", (unsigned long)&GPIOA->regs->CRH);
        CompositeSerial.write(buff);
        sprintf(buff," GPIOA->regs->CRH = %08lx \r\n", acrh=GPIOA->regs->CRH);
        CompositeSerial.write(buff);
        sprintf(buff,"&GPIOB->regs->CRL = %08lx \r\n", (unsigned long)&GPIOB->regs->CRL);
        CompositeSerial.write(buff);
        sprintf(buff," GPIOB->regs->CRL = %08lx \r\n", bcrl=GPIOB->regs->CRL);
        CompositeSerial.write(buff);
        sprintf(buff,"&GPIOB->regs->CRH = %08lx \r\n", (unsigned long)&GPIOB->regs->CRH);
        CompositeSerial.write(buff);
        sprintf(buff," GPIOB->regs->CRH = %08lx \r\n\n", bcrh=GPIOB->regs->CRH);
        CompositeSerial.write(buff);
      }
      for (int ip = 0 ; ip < nip ; ip++ ) {
        if ( ( hasKeyVelocity == 1 ) || ( (ip&1)==0 ) )
          pinMode( input_pin[ip],INPUT_PULLDOWN);
      }
      // Code generator
      sprintf(buff,"  if ( ( hasPedalBoard == %d ) && ( hasKeyVelocity == %d ) )\r\n  { \r\n", hasPedalBoard,hasKeyVelocity);
      CompositeSerial.write(buff);
      sprintf(buff,"    GPIOA->regs->CRL ^= 0x%08lx ;\r\n", (unsigned long)(GPIOA->regs->CRL^acrl) );
      CompositeSerial.write(buff);
      sprintf(buff,"    GPIOA->regs->CRH ^= 0x%08lx ;\r\n", (unsigned long)GPIOA->regs->CRH^acrh);
      CompositeSerial.write(buff);
      sprintf(buff,"    GPIOB->regs->CRL ^= 0x%08lx ;\r\n", (unsigned long)GPIOB->regs->CRL^bcrl);
      CompositeSerial.write(buff);
      sprintf(buff,"    GPIOB->regs->CRH ^= 0x%08lx ;\r\n  }\r\n", (unsigned long)GPIOB->regs->CRH^bcrh);
      CompositeSerial.write(buff);
   
}
// Code generator for fast discharge of active input wires
void KeyboardScan::PCodeGenerator(void)
{      
      char buff[120];      
      sprintf(buff,"Code Generator for fast active input line pull down\r\nWhich saves pull down resistors and execution time\r\n"); 
      CompositeSerial.write(buff);      

      generateOption(0,0);
      generateOption(0,1);
      generateOption(1,0);
      generateOption(1,1);
}

// Call this method twice to pull down and release active input scan lines
void KeyboardScan::activePulldown(bool hasPedalBoard, bool hasKeyVelocity ) {
  if ( ( hasPedalBoard == 0 ) && ( hasKeyVelocity == 0 ) ) { 
    GPIOA->regs->CRL ^= 0xb0b0b000 ;
    GPIOA->regs->CRH ^= 0xb00000b0 ;
    GPIOB->regs->CRL ^= 0x000000b0 ;
    GPIOB->regs->CRH ^= 0xb0b00000 ;
  }
  if ( ( hasPedalBoard == 0 ) && ( hasKeyVelocity == 1 ) ) { 
    GPIOA->regs->CRL ^= 0xbbbbbb00 ;
    GPIOA->regs->CRH ^= 0xb0000bbb ;
    GPIOB->regs->CRL ^= 0x0000b0bb ;
    GPIOB->regs->CRH ^= 0xbbb00000 ;
  }
  if ( hasPedalBoard == 1 ) {
//  if ( ( hasPedalBoard == 1 ) && ( hasKeyVelocity == 0 ) ) { 
//    GPIOA->regs->CRL ^= 0x00000000 ;
//    GPIOA->regs->CRH ^= 0xb00000b0 ;
//    GPIOB->regs->CRL ^= 0x00000000 ;
//    GPIOB->regs->CRH ^= 0xb0b00000 ;
//  }
//  if ( ( hasPedalBoard == 1 ) && ( hasKeyVelocity == 1 ) ) { 
    GPIOA->regs->CRL ^= 0x00000000 ;
    GPIOA->regs->CRH ^= 0xb0000bbb ;
    GPIOB->regs->CRL ^= 0x0000b000 ;
    GPIOB->regs->CRH ^= 0xbbb00000 ;
  }
}
/*
void KeyboardScan::originalactivePulldown(bool pedalboard ) {
    // To avoid long discharge times use pull down resistors 4k7 or less
    // OR active pull down by setting input pin briefly to output low
    // Tested with 330pF loading on input lines
    if ( pedalboard == 1 ) 
    { 
      // Set 8 input lines to output 0 @ 50MHz fall time
      GPIOA->regs->CRL ^= 0x00000000 ; // Redundant, but stays for constant discharge timing
      GPIOA->regs->CRH ^= 0xb0000bbb ;
      GPIOB->regs->CRL ^= 0x0000b000 ;
      GPIOB->regs->CRH ^= 0xbbb00000 ;
      // Revert 16 lines to input pull down
      GPIOA->regs->CRL ^= 0x00000000 ;
      GPIOA->regs->CRH ^= 0xb0000bbb ;
      GPIOB->regs->CRL ^= 0x0000b000 ;
      GPIOB->regs->CRH ^= 0xbbb00000 ;
    } else {
      // Set 16 input lines to output 0 @ 50MHz fall time
      GPIOA->regs->CRL ^= 0xbbbbbb00 ;
      GPIOA->regs->CRH ^= 0xb0000bbb ;
      GPIOB->regs->CRL ^= 0x0000b0bb ;
      GPIOB->regs->CRH ^= 0xbbb00000 ;
      // Revert 16 lines to input pull down
      GPIOA->regs->CRL ^= 0xbbbbbb00 ;
      GPIOA->regs->CRH ^= 0xb0000bbb ;
      GPIOB->regs->CRL ^= 0x0000b0bb ;
      GPIOB->regs->CRH ^= 0xbbb00000 ;
    }
    // Alternative time out for input line discharge
    //int count = 0;
    //while ( ( ++count < 10 ) && input ) { // wait for active inputs to decay to inactive
    //  input = get_input(pedalboard);
    //}
    //count *= 2 ; // wait for about double the same time again to guard against noise
    //while ( --count > 0 ) { 
    //  input = get_input(pedalboard);
    //} if ( pedalboard == 0 ) 
    
}*/

void KeyboardScan::Print( void ) {
// const char *NoteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
  char buf[80];
  const char * op_names[] = {
    "CN13 PIN4",
    "CN13 PIN3",
    "CN13 PIN2",
    "CN13 PIN1",
    "CN14 PIN4",
    "CN14 PIN3",
    "CN14 PIN2",
    "CN14 PIN1"
  };
 // char *note_name[] = {
 //   "C","C♯","D","D♯","E","F","F♯","G","G♯","A","A♯","B"
 // };
  int note_id=-1;
  //CompositeSerial.write(VT100_CLEAR);
  sprintf(buf, "Keyboard Scan Result MIDI Channel %d\r\n", midi.getKeyboardChannel()+1 );
  SUI.DisplayTitle( buf );

  for ( int i = 0 ; i < NUM_KEYBOARD_OUTPUTS ; i++ ) {
    for ( int j = 0 ; j < NUM_KEYBOARD_INPUTS; j++ ) {
      buf[NUM_KEYBOARD_INPUTS-1-j] = '0';
      if ( kb_input[i] & ( 1 << j ) ) {
        buf[NUM_KEYBOARD_INPUTS-1-j] = '1';
        // display lowest note
        int new_note  = (((j/2)*8 )+i);
        if ( note_id == -1 ) {
          note_id = new_note;
        }
        if ( note_id > new_note ) {
          note_id = new_note;
        }
      }
    }
    sprintf(buf+NUM_KEYBOARD_INPUTS," OP%d  %s\r\n",i,op_names[i]);
    CompositeSerial.write(buf);
  }
  CompositeSerial.write("........******** IP *=CN13 input pins 5-6,7=nc,8-13\r\n");
  CompositeSerial.write("********........ IP *=CN14 input pins 5-12\r\n");
}
void KeyboardScan::MusicKeyboardScan( bool pedalboard ) {
  //char buf[120];
  static unsigned long lastActivity;
//  const char *note_name[12] = {
//    "C","C♯","D","D♯","E","F","F♯","G","G♯","A","A♯","B"
//  };
//  const int OCTAVE = 12;
//  int note_id=-1;
  //profile.PStart(PROFILE_KEYBOARD);
  int change = FastHWScan(kb_input,kb_image, pedalboard );
  if ( change ) {
    // process MusicKeyboard changes
    //sprintf(buf,"Keyboard change = %d\r\n",change);
    //CompositeSerial.write(buf);    
    for ( int i = 0 ; i < NUM_KEYBOARD_OUTPUTS ; i++ ) {
      int difference = kb_input [i] ^ kb_image[i];
      //if ( pedalboard) difference &= 0x55 ; //  ignore top 8 and first contact inputs on pedalboard
      if ( difference ) {
        SUI.RequestDisplayUpdate();
        int inputs = NUM_KEYBOARD_INPUTS;
        if ( pedalboard ) inputs-=8;
        for ( int j = 0 ; j < inputs; j++ ) {
          if ( difference & ( 1 << j ) ) {
            int index = i*NUM_KEYBOARD_INPUTS + j ;
            long int key_travel_time;
            kb_time[index] = micros();
            if (  j & 1 ) { // top of key travel?
              key_travel_time = kb_time[index] - kb_time[index-1];
            } else {
              key_travel_time = kb_time[index] - kb_time[index+1];              
            }
            //sprintf(buf,"Key travel time = %ldus\r\n",key_travel_time);
            //CompositeSerial.write(buf);
            // Send to midi
            int midi_note = 36 + i + ( (j>>1) * 8 );
           
            int midi_velocity = 1 ;
            if ( key_travel_time < 100000 ) {
              midi_velocity = (100000-key_travel_time)/787;
            }
            //buf[0]=0;
            int midi_channel = midi.getKeyboardChannel() ;
            int on_off=-1;
            if ( SUI.Cfg.Bits.hasKeyVelocity ) {
              if ( kb_input [i] & ( 1 << j ) ) { // contact just closed?
                if ( ( j & 1 ) == 0 ) { // lower contact?
                  led.on();
                  on_off = 1;
                  // Ardour with fluidsynth expects on and off counts to agree
                  // Different behaviours are found for different manufacturers partial key activation
                  // Kawai ES920 sends n note off messages after n note on messages
                  // M-AudioKS61Mk3 ignores repeated on 
                  if ( ! note_on[midi_note] ) { // suppress repeat on messages
                    note_on[midi_note]++;
                    midi.sendNoteOn ( midi_channel, midi_note, midi_velocity );
                    SUI.monitorNoteOn   ( midi_channel, midi_note, midi_velocity, SUI.DEV_KEYBOARD  );
                  }
                }
              } else { // contact just opened
                if ( j & 1 ) { // upper contact?
                  led.off();
                  on_off = 0;
                  if ( note_on[midi_note] ) { // suppress repeat note off messages
                    note_on[midi_note] = 0;
                    midi.sendNoteOff ( midi_channel, midi_note, midi_velocity );
                    SUI.monitorNoteOff   ( midi_channel, midi_note, midi_velocity, SUI.DEV_KEYBOARD  );
                  }
                }
              }
            } else {
              midi_velocity = 64; // single contact set medium velocity
              if ( kb_input [i] & ( 1 << j ) ) { // contact just closed?
                if ( ( j & 1 ) == 0 ) { // lower contact closing?
                  led.on();
                  on_off = 1;
                  midi.sendNoteOn ( midi_channel, midi_note, midi_velocity );
                  SUI.monitorNoteOn   ( midi_channel, midi_note, midi_velocity, SUI.DEV_KEYBOARD  );
                }
              } else {
                if ( ( j & 1 ) == 0 ) { // lower contact opening
                  led.off();
                  on_off = 0;
                  midi.sendNoteOff ( midi_channel, midi_note, midi_velocity );
                  SUI.monitorNoteOff   ( midi_channel, midi_note, midi_velocity, SUI.DEV_KEYBOARD  );
                }
              }            
            }
            /*
            const int HID_KEY_OFFSET = 96;
            if ( (on_off != -1 ) && (midi_note >=HID_KEY_OFFSET )  && ( midi_note <=HID_KEY_OFFSET+3 ) ) {
              const unsigned control[] = {
                KEY_PAGE_DOWN,
                KEY_PAGE_UP,
                KEY_HOME,
                KEY_END};
              if ( on_off )
                QKeyboard.press( control[midi_note-HID_KEY_OFFSET] );
              else
                QKeyboard.release( control[midi_note-HID_KEY_OFFSET] );
              char HIDBuf[80];
              sprintf( HIDBuf,"Sending HID %02X on_off=%d\r\n", control[midi_note-HID_KEY_OFFSET] ,on_off);
              CompositeSerial.write(HIDBuf);
            }*/
            // Use Csharp/Db with no other activity for 5 seconds to control RGB LED Strip
            if ( on_off >= 0 ) { 
              if ( midi_note == 37 ) { // Lowest C#
                if ( ( micros()-lastActivity ) > 5000000L ) {
                   ws2812Ctrl.service(on_off);
                   //sprintf(buf,"LED RGB=%d\r\n",on_off);
                }
              } else {
                lastActivity = micros();
              }
             }
            //if ( buf[0]) {
            //  CompositeSerial.write(buf);    
            //}
          }
        }
      }
      kb_image[i] = kb_input[i]; // inform driver
    }
  }
  //profile.PEnd(PROFILE_KEYBOARD);
}
