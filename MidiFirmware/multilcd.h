#ifndef __MULTI_LCD_H
#define __MULTI_LCD_H


#ifdef __cplusplus
extern "C" {
#endif

class MultiLCD {
  private:
  const int MIN_LCD_ADDR = 0x20;
  const int MAX_LCD_ADDR = 0x3F;
  static const int LCD_SIZE = 32;
  static const int NUM_LCD = 16;
  struct LCD_INFO { uint8_t responding; uint8_t i2cAddr; char dmesg[LCD_SIZE+1]; } ;
  LCD_INFO lcdData[NUM_LCD*2];
  int getLCDIndex( unsigned int lcd_address ) ;
  public:
  MultiLCD( void );
  void Begin ( char * mesg ) ;
  int I2cCheck( void ) ;
  void Test(void);
  void Print( void ) ;
  void saveDisplayText( int lcd_address, char *buff);
  int Write(uint8_t lcd_address, char *buff ) ;
};

#ifdef __cplusplus
}
#endif

#endif /* __MULTI_LCD_H */
