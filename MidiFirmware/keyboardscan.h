#ifndef __KEYBOARDSCAN_H
#define __KEYBOARDSCAN_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Fast KEYBOARD Scan of 74HC164 shift register connected KEYBOARDs and LEDs
 * Call KEYBOARD scan with ptrs to input and output array,
 * returns number of active KEYBOARD inputs
 */

const int NUM_KEYBOARD_OUTPUTS = 8;
const int NUM_KEYBOARD_INPUTS = 16;

class KeyboardScan {
private:
uint32_t get_input(bool pedalboard);
void activePulldown(bool pedalboard);
  // STM32F103 Cortex M3 Memory Mapped hardware register addresses Ref: RM0008-STM32F1... Sec3.3 P51
  const uint32_t IDR =   0x00000008; // RM0008-STM32F1... Table 59 P194
  const uint32_t ODR   = 0x0000000C;
  const uint32_t GPIO  = 0x40000000; 
  const uint32_t BB    = 0x42000000;
  const uint32_t PORTA = 0x00010800;
  const uint32_t PORTB = 0x00010c00;
  const uint32_t PORTC = 0x00011000;
  const volatile uint32_t* BB_PORTB_ODR_B07 = (uint32_t*) (BB + ( (PORTB+ODR)<<5 ) + ((uint32_t) 7 << 2 ));
  volatile uint32_t*   IDR_B = (volatile uint32_t*)  0x40010C08; // Port B  Input data register
  volatile uint32_t*   ODR_B = (volatile uint32_t*)  0x40010C0C; // Port B Output data register
  // STM32 Cortex M3 Bit Band addresses ( 1 bit per 32 bit word ) Ref: RM0008-STM32F1.. Sec 3.3.2 P53
  
  volatile uint32_t* PB07_BB = (volatile uint32_t*) (0x42000000 + (0x10c0C<<5) + ( 7<<2) ); // BB Port B ODR bit  7
  volatile uint32_t* PB11_BB = (volatile uint32_t*) (0x42000000 + (0x10c0C<<5) + (11<<2) ); // BB Port B ODR bit 11
  volatile uint32_t* PB12_BB = (volatile uint32_t*) (0x42000000 + (0x10c08<<5) + (12<<2) ); // BB Port B IDR bit 12
public:
  KeyboardScan();
  uint32_t Scan(uint32_t *kb_input,uint32_t*kb_image, bool pedalboard );
};

#ifdef __cplusplus
}
#endif

#endif /* __KEYBOARDSCAN_H */
