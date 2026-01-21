/*
 * ROMdata.c
 *
 *  Created: Jan 2026
 *   Author: Radiofan
 */
#include "main.h"
#include "stdbool.h"
#include "GPIO.h"
#include "ROMdata.h"

/* Measurements from BBC Micro:
 * M0 and M1 pulses are precisely 1 clock cycle in duration
 * M1 (address) pulses are on average 10 clock cycles apart
 *   and lag ADD8 setup by at least 1 clock cycle
 * M0 (read data) edges lag rising edge of clock signal by only 190ns
 * The gap between M0 pulses varies between 1 clock cycle and 350us,
 *   not necessarily aligned to word boundaries.
 * During indirect load M1 lags M0 by only 60ns, considered simultaneous
 */

/* Test scenarios to consider:
 *
 * 1) Reads from initial state should do nothing
 * 2) 5 address writes followed by reads should return the correct data
 * 3) Incomplete address writes followed by a read should reset to initial state
 * 4) 5 valid address writes followed by indirect read should return the correct data
 * 5) Indirect read from an unknown ROM address should reset to the initial unresponsive state
 * 6) Successive reads should continue incrementing the address
 * 7) Reading past end of ROM boundary (to next ROM) should return the correct data
 * 8) Reading past end of ROM boundary (to unknown ROM) should increment the address
 */

volatile uint8_t active = 0;

static volatile uint32_t currentAddress = 0;
static volatile const uint8_t *dataBank;
static volatile uint32_t phromSize;
static volatile uint32_t dataLength;

static volatile bool outputEnabled = false;
static volatile uint8_t outputByte;
static volatile uint8_t outputBitIndex;

static volatile rd_state readState = RD_INIT;
static volatile int8_t addrCount = 0;

#ifdef PHROMS_TI99
static const int8_t PHROM_idx[16] = {1,2,-1,-1,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0};
static const PHROM_desc PHROMs[N_PHROMS] = {
  { phromData_Acorn, 0x4000, DLEN_ACORN},
  { phromData_TI99, 0x4000, 0x4000},
  { &phromData_TI99[0x4000], 0x4000, DLEN_USA % 0x4000},
  { PHROMdata_Clock, 0x1000, DLEN_CLOCK}
};
#else
static const int8_t PHROM_idx[16] = {1,-1,3,-1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0};
static const PHROM_desc PHROMs[N_PHROMS] = {
  { phromData_Acorn, 0x4000, DLEN_ACORN},
  { phromData_USA, 0x4000, DLEN_USA},
  { PHROMdata_Clock, 0x1000, DLEN_CLOCK},
  { PHROMdata_Mil, 0x4000, DLEN_Milit}
};
#endif

static inline void GPIO_Dout(void) {
  if (!outputEnabled) {
    GPIO_Init_Add8_D(GPIO_MODE_OUTPUT_OD);
    outputEnabled = true;
  }
}

static void GPIO_A8_in(void) {
  if (outputEnabled) {
    // Set the ADD8 bus pin to output mode and set the pin high
    // (as this is what the original TMS6100 does)
    GPIO_SetPin(GPIOA, ADD8_Data_Pin);
    GPIO_Init_Add8_D(GPIO_MODE_INPUT);
    outputEnabled = false;
  }
}

static void GPIO_A8_Out_Safe() {
  if (dataBank) {
    // Set the ADD8 bus pin to output mode
    GPIO_Dout();
  } else {
    // Set the ADD8 bus pin to input mode
    GPIO_A8_in();
  }
}

/* Most PHROMs are 16K in size, but the clock PHROM is only 4K.
 * Since some PHROMs use the last entry as a silent delay (series of zeros)
 * it is important to detect the end of the PHROM so that the delay can
 * stop in a timely manner. Releasing output on A8/D after the 4K boundary
 * results in 0xFF being read by the TMS5220, which stops speech.
 */

static void read_data_currentbank() {
  // Load the first byte to be sent (if this is our bank)
  if (dataBank) {
    uint32_t localAddress = (currentAddress & 0x3FFF);
    if (localAddress >= phromSize) {
      readState = RD_AOOB;
    } else {
      if (localAddress < dataLength) {
        outputByte = dataBank[localAddress];
      } else {
        // PHROMs are padded with 0s at the end
        outputByte = 0;
      }
      if (readState == RD_AOOB) {
        // Entering valid address space, enable data out
        readState = RD_READY;
        GPIO_A8_Out_Safe();
      }
    }
  } else {
    // No longer valid address. Note: switching A8 to input here
    //  would corrupt the last bit being sent
    readState = RD_AOOB;
  }
  // Reset the bit pointer
  outputBitIndex = 0;
}

static void read_data() {
  // Get the current bank and local address
  uint32_t currentBank = (currentAddress & 0x3C000) >> 14;
  int8_t i = PHROM_idx[currentBank];
  dataBank = NULL;
  if (i >= 0) {
      dataBank = PHROMs[i].address;
      phromSize = PHROMs[i].rom_size;
      dataLength = PHROMs[i].length;
  }
  // And read one byte
  read_data_currentbank();
}

static void M0_Dummy_Handler() {
  // This is the first M0 pulse after an M1 (address) pulse
  // Load the first byte to be sent (if from our bank)
  read_data();
  if (readState == RD_DUMMY) {
    readState = RD_READY;
    GPIO_A8_Out_Safe();
  }
}

static inline void M0_IncPointer() {
  // Increment the bit pointer
  if (++outputBitIndex == 8) {
    // Get the next byte to be sent
    currentAddress++;
    read_data();
  }
}

static inline void addr_reset() {
  addrCount = 0;
  readState = RD_INIT;
  currentAddress = 0;
}

static inline void M1_LoadAddress_Handler() {
  uint32_t addressNibble = 0;

  // Read the nibble from the address bus
  if (GPIO_ReadPin(GPIOA, ADD1_Pin)) addressNibble += 1;
  if (GPIO_ReadPin(GPIOA, ADD2_Pin)) addressNibble += 2;
  if (GPIO_ReadPin(GPIOA, ADD4_Pin)) addressNibble += 4;
  if (GPIO_ReadPin(GPIOA, ADD8_Data_Pin)) addressNibble += 8;

   // Shift the current address register right one nibble
   currentAddress = currentAddress >> 4;

   // Place the received nibble in the top of the current address register
   currentAddress |= (addressNibble << 16);

   if (++addrCount == 5) {
     addrCount = 0;
     readState = RD_DUMMY;
   } else
     readState = RD_ADDR;
}

static inline void M1_Indirect_Handler() {
  if (readState == RD_DUMMY) {
    // Address is complete, read the next two bytes as the next address
    read_data();
    if (readState == RD_DUMMY) {
      uint32_t locaAddress = (currentAddress & 0x3C000) | outputByte;
      currentAddress++;
      read_data_currentbank();
      currentAddress = locaAddress | ((outputByte & 0b00111111) << 8);
      if (readState == RD_DUMMY)
        M0_Dummy_Handler();
    } else {
      // Bad indirect address, reset
      readState = RD_INIT;
    }
  }
}

/* Actions are initiated on the rising edge of M0 and M1 but some operations
 * are only ready to complete on the next falling edge of the clock. The
 * rising edge handler sets a pending action in the tms_command variable,
 * the clock falling edge handler then carries out the action.
 */
#define TMS_NONE 0
#define TMS_M0_DUMMY 1
#define TMS_M1_ADDR  2
#define TMS_M0_INCP  4

static volatile uint8_t tms_command = 0;

/* M0_READ / M1_ADDR handler, must complete within 3.125uS (100 clock cycles at 32MHz)
 * ISR compiles to 85 instructions, should be OK
 */
void EXTI4_15_IRQHandler(void) {
  if (__HAL_GPIO_EXTI_GET_RISING_IT(M1_Pin) != 0x00u) {
    __HAL_GPIO_EXTI_CLEAR_RISING_IT(M1_Pin);
    GPIO_SetPin(GPIOA, LED_Red_Pin);
    // Make sure the ADD8 bus pin is in input mode
    GPIO_A8_in();
    // Read address on next falling edge of clock
    tms_command |= TMS_M1_ADDR;
    active = 1;
  }

  if (__HAL_GPIO_EXTI_GET_RISING_IT(M0_Pin) != 0x00u) {
    __HAL_GPIO_EXTI_CLEAR_RISING_IT(M0_Pin);
    if (readState == RD_DUMMY) {
      GPIO_SetPin(GPIOA, LED_Green_Pin);
      // Dummy read on next falling edge of clock
      tms_command |= TMS_M0_DUMMY;
    } else if (readState == RD_READY) {
      /* This is a bit read M0 pulse, data must be ready by the next
       * falling edge of clock, so we send it immediately...
       */
      GPIO_SetPin(GPIOA, LED_Green_Pin);
      // Push data to the output pin
      if (outputByte & (1 << outputBitIndex)) {
        GPIO_SetPin(GPIOA, ADD8_Data_Pin);
      } else {
        GPIO_ClrPin(GPIOA, ADD8_Data_Pin);
      }
      // Increment bit pointer on next falling edge of clock
      tms_command = TMS_M0_INCP;
    } else if (readState == RD_AOOB) {
      GPIO_A8_in();
      GPIO_SetPin(GPIOA, LED_Green_Pin);
      // Keep incrementing, in case we reach a valid ROM address
      tms_command |= TMS_M0_INCP;
    } else {
      // Bad address, reset
      addr_reset();
    }
    active = 1;
  }
}

/* ISR to handle falling edge of clock
 * We execute commands, inputs should be stable at this point.
 * Must complete within 3.125uS (100 clock cycles at 32MHz)
 * Compiles to 122 instructions but only one part of it
 *  is executed during each invocation.
 */
void EXTI2_3_IRQHandler(void) {
  if (__HAL_GPIO_EXTI_GET_RISING_IT(TMS_CLK_Pin) != 0x00u)
    __HAL_GPIO_EXTI_CLEAR_RISING_IT(TMS_CLK_Pin);

  if (__HAL_GPIO_EXTI_GET_FALLING_IT(TMS_CLK_Pin) != 0x00u) {
    __HAL_GPIO_EXTI_CLEAR_FALLING_IT(TMS_CLK_Pin);
    if (tms_command == TMS_M0_DUMMY) {
      M0_Dummy_Handler();
    } else if (tms_command == TMS_M1_ADDR) {
      M1_LoadAddress_Handler();
    } else if (tms_command == TMS_M0_INCP) {
      M0_IncPointer();
    } else if (tms_command == (TMS_M0_DUMMY | TMS_M1_ADDR)) {
      M1_Indirect_Handler();
    } else if (tms_command == (TMS_M0_INCP | TMS_M1_ADDR)) {
      // Bad indirect address, reset
      readState = RD_INIT;
    } else {
      GPIO_ClrPin(GPIOA, LED_Green_Pin);
      GPIO_ClrPin(GPIOA, LED_Red_Pin);
    }
    tms_command = TMS_NONE;
  }
}
