//-------------------------------------------------------------
// Blue Pill STM32F103C8 WS2812 LED driver
// 2019.12.12 RSP
// "clockless" technology (blocking code, based on CPU cycle timing)
//  good for small strings
// https://github.com/FearlessNight/bluepill_ws2812/blob/master/bluepill_ws2812.cpp
#include "bluepill_ws2812.h"

void bluepill_neopixel::begin( uint32_t port_addr, uint8_t port_bit )
{

  // Get access to the clock
  CoreDebug->DEMCR  |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0;

  // set pin as output
  volatile uint32_t *CRX;
  if (port_bit < 8)
    CRX = ((volatile uint32 *)(port_addr+0x00)); // GPIOx_CRL
  else
  { CRX = ((volatile uint32 *)(port_addr+0x04)); // GPIOx_CRH
    port_bit -= 8;
  }
  *CRX &= ~(0xF << port_bit*4); // CNF  = 0 (gp push-pull output)
  *CRX |=     1 << port_bit*4;  // MODE = 1 (output mode 10 MHz)
}

//- - - - - - - - - - - - - - - - - - -

__attribute__ ((always_inline)) inline static void clockout8bits(
  register volatile uint32_t *port,
  register uint32_t hi,
  register uint32_t lo,
  register uint8_t  b)
{
  for(register uint32_t i = 8; i > 0; i--) 
  {
    while (DWT->CYCCNT < THL ); // 1250ns
   *port = hi;
    DWT->CYCCNT = 18; // Derived by experimental measurement
    if(b & 0x80) 
    { while (DWT->CYCCNT < T1H ); // 800ns
      *port = lo;
    } else 
    { while (DWT->CYCCNT < T0H ); // 400ns
      *port = lo;
    }
    b <<= 1;
  }
}

static void clockoutPixels( uint8_t *pixels, uint16_t count, uint32_t port_addr, uint8_t port_bit )
{
  register volatile uint32_t *port = ((volatile uint32 *)(port_addr+0x10)); // GPIOx_BSRR
  register uint8_t  b;
  register uint32_t hi = 1 <<  port_bit;    
  register uint32_t lo = 1 << (port_bit+16); 
  cli();
    //DWT->CYCCNT = 0;
    for (register uint32_t i=count; i > 0; i--)
    {
      b = *(pixels++);
      clockout8bits( port, hi, lo, b );
  
      b = *(pixels++);
      clockout8bits( port, hi, lo, b );
  
      b = *(pixels++);
      clockout8bits( port, hi, lo, b );
    }
  sei();
}

void bluepill_neopixel::paint( uint8_t *pixels, uint16_t count, uint32_t port_addr, uint8_t port_bit )
{
  delayMicroseconds(50);
  clockoutPixels(pixels,count,port_addr,port_bit);
}
