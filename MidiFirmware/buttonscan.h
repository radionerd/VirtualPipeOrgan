#ifndef __BUTTONSCANCPP_H
#define __BUTTONSCANCPP_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Fast Button Scan of 74HC164 shift register connected buttons and LEDs
 * Call button scan with ptrs to input and output array,
 * returns number of active button inputs
 */
const int NUM_SHIFT_REGS = 8;
const int SHIFT_REG_SIZE = 8;
const int NUM_SHIFT_REG_OUTPUTS = NUM_SHIFT_REGS * SHIFT_REG_SIZE;

class ButtonScan {
private:
	void ShiftRegN8ClockBB( int count );
  // STM32F103 Cortex M3 Memory Mapped hardware register addresses Ref: RM0008-STM32F1... Sec3.3 P51
  volatile uint32_t*   IDR_B = (volatile uint32_t*)  0x40010C08; // Port B  Input data register
  volatile uint32_t*   ODR_B = (volatile uint32_t*)  0x40010C0C; // Port B Output data register
  // STM32 Cortex M3 Bit Band addresses ( 1 bit per 32 bit word ) Ref: RM0008-STM32F1.. Sec 3.3.3 P53
  volatile uint32_t* PB07_BB = (volatile uint32_t*) (0x42000000 + (0x10c0C<<5) + ( 7<<2) ); // BB Port B ODR bit  7
  volatile uint32_t* PB11_BB = (volatile uint32_t*) (0x42000000 + (0x10c0C<<5) + (11<<2) ); // BB Port B ODR bit 11
  volatile uint32_t* PB12_BB = (volatile uint32_t*) (0x42000000 + (0x10c08<<5) + (12<<2) ); // BB Port B IDR bit 12

  uint32_t sr_input_list[NUM_SHIFT_REG_OUTPUTS+1];
  uint32_t sr_outputs   [NUM_SHIFT_REG_OUTPUTS+1];
  struct Image {
      int Input;
      unsigned long Time;
      int Output;
      int Error;
  };
  Image ShiftRegImage[NUM_SHIFT_REG_OUTPUTS];

public:
  ButtonScan(void);
  void Scan(void);
	uint32_t ScanSR( uint32_t *sr_input_list, uint32_t *sr_outputs );
  void SetLED(int output, int value );
  void Print(void);
};

#ifdef __cplusplus
}
#endif

#endif /* __BUTTONSCANCPP_H */
