#include <Wire.h>
#include <USBComposite.h> // https://github.com/arpruss/USBComposite_stm32f1 Arduino version does not include Sysex support
#include "multilcd.h"
#include "pin.h"
#include "LiquidCrystal_PCF8574RJ.h"
#include "USBSerialUI.h"


LiquidCrystal_PCF8574 lcd(0x27);  // Address specified later

  int MultiLCD::getLCDIndex( unsigned int lcd_address ) {
    if ( lcd_address < 16 ) return lcd_address;
    return 16+(lcd_address&0xf); // 0x20-0x27 -> 0-7, 0x38-0x3f -> 7-15
  }

 
  MultiLCD::MultiLCD( void ) {
  }

  void MultiLCD::Begin ( char * mesg ) 
  {
   if ( SUI.Cfg.Bits.hasI2C ) {
    if (  I2cCheck() == 0 ) {
     for ( uint8_t i2cAddr = MIN_LCD_ADDR ; i2cAddr <= MAX_LCD_ADDR ; i2cAddr++ ) {
      if ( i2cAddr == 0x28 )
        i2cAddr = 0x38;
      Wire.begin(1);
      Wire.setClock(400000); // fast speed
      Wire.beginTransmission(i2cAddr);
      int error = Wire.endTransmission();
      if ( error == 0 ) {
        for ( int i = 0 ; i < 32 ; i++ )
          lcdData[i2cAddr&0xf].dmesg[i]=0; // clear our display image
        lcd.seti2cAddr(i2cAddr);
        lcd.begin(16, 2, Wire); // initialise lcd hardware
        char buff[80];
        if ( mesg[0] ) 
        {
          strncpy ( buff,mesg , LCD_SIZE );
        } else {
          sprintf(buff,"VirtualPipeOrgani2cAddr=%2d  0x%02X",i2cAddr,i2cAddr );
        }
        buff[ LCD_SIZE ] = 0;
        Write( i2cAddr,buff);
      } 
     }
    } 
   }
  }

 
int MultiLCD::Write(uint8_t lcd_address, char *buffer )
{
   int error = -1;
   int lcd_index = getLCDIndex(lcd_address);
   if ( SUI.Cfg.Bits.hasI2C ) {
      Wire.begin(1);      
      Wire.setClock(400000); // fast speed
      Wire.beginTransmission(lcd_address);
      error = Wire.endTransmission(false);
      if ( error == 0 ) {
        lcd.seti2cAddr( lcd_address ); // Richard Jones extended library required
        char line0[20]; // copy the line buffers so that we can search for changes
        char line1[20];
        strncpy (line0,buffer,17);
        line0[16]=0;
        strncpy (line1,buffer+16,17);
        line1[16]=0;
        // Write changes to lcd line 0
        int x0 = 0;
        while ( (x0 < 16) && ( line0[x0] == lcdData[lcd_index].dmesg[x0] ) )
          ++x0;
        int xe0 = 15 ;
        while ( (xe0 > x0 ) && ( line0[xe0] == lcdData[lcd_index].dmesg[xe0] ) ) {
          line0[xe0]=0; // truncate rh of repeat message
          --xe0;
        }
#define NOMDEBUG
#ifdef MDEBUG
        char buff[80];
        sprintf(buff,"i2c=%02x x0=%2d xe0=%2d line0=[%s] lcd=[%s]\r\n",lcd_address,x0,xe0,line0+x0, lcdData[lcd_index].dmesg);
        CompositeSerial.write(buff);
#endif
        if ( x0 < 16 ) {
          lcd.setCursor(x0, 0);
          lcd.print( line0+x0 );
        }
        // Write changes to lcd line 1
        int x1 = 0;
        while ( (x1 < 16) && ( line1[x1] == lcdData[lcd_index].dmesg[16+x1] ) )
          ++x1;
        int xe1 = 15 ;
        while ( (xe1 > x1 ) && ( line1[xe1] == lcdData[lcd_index].dmesg[16+xe1] ) ) {
          line1[xe1]=0; // truncate rh of repeat message
          --xe1;
        }
#ifdef MDEBUG
        sprintf(buff,"i2c=%02x x1=%2d xe1=%2d line1=[%s],lcd=[%s]\r\n",lcd_address,x1,xe1,line1+x1, lcdData[lcd_index].dmesg);
        CompositeSerial.write(buff);
#endif
        if ( x1 < 16 ) {
          lcd.setCursor(x1, 1);
          lcd.print( line1+x1 );
        }
        lcdData[lcd_index].responding = TRUE;
      }
  }
  saveDisplayText( lcd_address, buffer );
  //strncpy( &lcdData[lcd_index].dmesg[0] , buffer , LCD_SIZE ); // Record for print method
  //lcdData[lcd_index].i2cAddr=lcd_address;
  return error;
}
void MultiLCD::saveDisplayText( int lcd_address , char *text )
{
  int lcd_index = getLCDIndex( lcd_address );
  strncpy( &lcdData[lcd_index].dmesg[0] , text , LCD_SIZE ); // Record for print method
  lcdData[lcd_index].i2cAddr=lcd_address;
}

  // Return non-zero if i2c bus has slow rise time eg No pullup resistors
  int MultiLCD::I2cCheck( void ) {
    int port = PB6;
    int result;
    int error = 0;
    const int PB6_SCL_ID = 33;
    const int PB7_SDA_ID = 34;
    //Serial.print("Probing PB6 I2C Bus ");
    pinMode(port,OUTPUT);
    digitalWrite(port,0 );
    pinMode(port,INPUT);
    result = digitalRead(port);
    if ( result == 0 ) { // check for short rise time on port
      SUI.LiveConfigs[PB6_SCL_ID].fault = '!';
      SUI.setFault(PB6_SCL_ID,'!');
      error = port;
    }
    port = PB7;
    //Serial.print("Probing PB7 I2C Bus ");
    pinMode(port,OUTPUT);
    digitalWrite(port,0 );
    pinMode(port,INPUT);
    result = digitalRead(port);
    digitalWrite(port,1 );
    if ( result == 0 ) {
      SUI.setFault(PB7_SDA_ID,'!');
      SUI.LiveConfigs[PB7_SDA_ID].fault = '!';
      error = port;
    }
    return error;
  }
  
  void MultiLCD::Print( void ) {
    char buff[80];
    sprintf(buff,"LCD Display List\r\nDec  Hex Text\r\n");
    CompositeSerial.write(buff);
    for ( int index=0 ; index < NUM_LCD*2 ; index++ ) {
      if ( lcdData[index].dmesg[0] ) {
        uint8_t i2cAddr = lcdData[index].i2cAddr;
        char *active = (char *)"Not Responding";
        if ( lcdData[index].responding || (i2cAddr<=16))
          active = (char *) "";
        char line0[20];
        strncpy ( line0 , lcdData[index].dmesg, 16 );
        line0[16]=0;
        sprintf ( buff," %02d 0x%02X [%s][%s] %s\r\n",i2cAddr,i2cAddr,line0,lcdData[index].dmesg+16,active);
        CompositeSerial.write(buff);
      }
    }
    if ( ! SUI.Cfg.Bits.hasI2C ) {
       sprintf(buff,"\nI2C/LCD Not Enabled\r\n");
       CompositeSerial.write(buff);
    }
  }

  void MultiLCD::Test(void)
{
  char buff[80];

  if ( I2cCheck() )
    sprintf(buff,"I2C bus FAIL. %sNeeds 4k7 pullups?%s\r\n",ANSI_BLINK,ANSI_NO_BLINK );
  else
    sprintf(buff,"I2C Bus PASS\r\n");
  CompositeSerial.write(buff);
  static int test_count = 0;
  sprintf(buff, "LCD Check %6dABCDEFGHIKLMNOPQR\r\n", test_count++);
  CompositeSerial.write(buff);
  buff[32]=0;
  Begin(buff); // Scan i2c bus to discover any displays & write to them
}
