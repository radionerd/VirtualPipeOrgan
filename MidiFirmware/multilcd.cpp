#include <Wire.h>
#include <USBComposite.h> // https://github.com/arpruss/USBComposite_stm32f1 Arduino version does not include Sysex support
#include "multilcd.h"
#include "pin.h"
#include "LiquidCrystal_PCF8574RJ.h"
#include "USBSerialUI.h"


LiquidCrystal_PCF8574 lcd(0x27);  // Address specified later

  int MultiLCD::getLCDIndex( unsigned int lcd_address ) {
    int index = -1;
    if ( lcd_address >= 0x20 && lcd_address <= 0x27 )
      index = lcd_address & 7;
    if ( lcd_address >= 0x38 && lcd_address <= 0x3f )
      index = lcd_address & 0xf;
    return index;
  }

 
  MultiLCD::MultiLCD( void ) {
//    if ( I2cCheck() !=0 ) {
//      CompositeSerial.println("LCD I2C Bus check fail");
//    } else {
//      Begin ((char*)"");
//    }
  }

  void MultiLCD::Begin ( char * mesg ) {

//    Wire.begin(1);
//    Wire.setClock(400000); // fast speed
    for ( uint8_t i2cAddr = MIN_LCD_ADDR ; i2cAddr <= MAX_LCD_ADDR ; i2cAddr++ ) {
      if ( i2cAddr == 0x28 )
        i2cAddr = 0x38;
      Wire.begin(1);
      Wire.setClock(400000); // fast speed
      Wire.beginTransmission(i2cAddr);
      int error = Wire.endTransmission();
      if ( error == 0 ) {
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
/*      Wire.begin(1);
      Wire.setClock(400000); // fast speed
      //Wire.setClock(100000); // normal speed
      Wire.beginTransmission(i2cAddr);
      int error = Wire.endTransmission();
      if ( error == 0 ) {
         //i2cList[i++]=i2cAddr;
         lcd.seti2cAddr(i2cAddr);
         lcd.begin(16, 2, Wire);
         lcd.setCursor(0, 0);
         char line1[20];
         sprintf(line1,"LCD i2cAddr=0x%02x",i2cAddr);
         //             0123456789abcdef
         lcd.print( line1 );
         lcd.setCursor(0, 1);
         lcd.print( line1 );
         Serial.println ( line1 );
      }*/
    } 
    /*
      char buff[80];
      Wire.begin(1);
      Wire.setClock(400000); // fast speed
      for ( int i2c_address = 1 ; i2c_address < 128 ; i2c_address++ ) {
        Wire.begin(1);
        Wire.setClock(400000); // fast speed
        Wire.beginTransmission(i2c_address);
        int error = Wire.endTransmission();
        if ( error == 0 ) {
            if ( mesg[80] ) {
              strncpy( buff , mesg, LCD_SIZE );
            } else {

              sprintf ( buff , "VirtualPipeOrganI2CAddr=%2d 0x%02x",i2c_address,i2c_address);
            }
            //                0123456789abcdef0123456789abcdef
            //buff[LCD_SIZE]=0; // Null terminate
          //if ( false ) { 

            lcd.seti2cAddr( i2c_address ); // We assume an address response is from a lcd
            lcd.begin(16, 2, Wire);  // 7913us initialize the lcd  
            lcd.setCursor(0, 0);
            lcd.setBacklight(255);
            lcd.print(buff);
          //}
            //Write ( i2c_address , buff  );
        }
      }*/
    }

 
int MultiLCD::Write(uint8_t lcd_address, char *buffer )
{
   int lcd_index = getLCDIndex(lcd_address);
   if ( lcd_index >= 0 ) {
      Wire.begin(1);      
      Wire.setClock(400000); // fast speed
      Wire.beginTransmission(lcd_address);
      int error = Wire.endTransmission(false);
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
#ifdef MDEBUG
        char buff[80];
        sprintf(buff,"i2c=%02x x0=%2d xe0=%2d line0=[%s]",lcd_address,x0,xe0,line0+x0, lcdData[lcd_index].dmesg);
        CompositeSerial.println(buff);
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
#ifdef MDEGUG
        sprintf(buff,"i2c=%02x x1=%2d xe1=%2d line1=[%s],lcd=[%s]",lcd_address,x1,xe1,line1+x1, lcdData[lcd_index].dmesg);
        CompositeSerial.println(buff);
#endif
        if ( x1 < 16 ) {
          lcd.setCursor(x1, 1);
          lcd.print( line1+x1 );
        }
        lcdData[lcd_index].responding = TRUE;
    }
    strncpy( &lcdData[lcd_index].dmesg[0] , buffer , LCD_SIZE ); // Record for print method
    lcdData[lcd_index].i2cAddr=lcd_address;
    return error;
  }
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
    for ( int index=0 ; index < NUM_LCD ; index++ ) {
      if ( lcdData[index].dmesg[0] ) {
        uint8_t i2cAddr = lcdData[index].i2cAddr;
        char *active = (char *)"Not Responding";
        if ( lcdData[index].responding )
          active = (char *) "";
        char line0[20];
        strncpy ( line0 , lcdData[index].dmesg, 16 );
        line0[16]=0;
        sprintf ( buff," %02d 0x%02X [%s][%s] %s\r\n",i2cAddr,i2cAddr,line0,lcdData[index].dmesg+16,active);
        CompositeSerial.write(buff);
      }
    }
  }

  void MultiLCD::Test(void)
{
  char buff[80];
  static int test_count = 0;
  sprintf(buff, "LCD Check %6d    \r\n", test_count++);
  CompositeSerial.write(buff);
  buff[16]=0; // Top line only
  Begin(buff); // Scan i2c bus to discover any displays & write to them
}
