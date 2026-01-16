/*
 * mainLoop.c
 *
 *  Created: Jan 2026
 *   Author: Radiofan
 */
#include "main.h"
#include "SysClock.h"
#include "GPIO.h"
#include "ROMdata.h"

// Inactivity timeout = 236ms approx.
#define TIMEOUT 1200000UL
static uint32_t inactive_count = 0;

/* All of the IO functionality is handled by interrupts.
 * This function initialises the MCU and it's peripherals,
 * then looks for idle periods (most of the time) so that
 * it can put the MCU to sleep, thereby saving some leccy.
 */

int main(void) {
  // Reset of all peripherals, Initialises the Flash interface and the Systick.
  HAL_Init();
  // Configure the system clock (64MHz)
  SysClock_Config();
  // Initialise the IO pins
  GPIO_Init();
  // We don't use the HAL timers and we don't want unnecessary interrupts to cause jitter
  HAL_SuspendTick();
  // Blue activity LED on
  GPIO_SetPin(GPIOA, TP1_Pin);

  while (1) {
    if (active) {
      inactive_count = 0;
      active = 0;
    } else
      inactive_count++;
    if (inactive_count > TIMEOUT) {
      // Blue activity LED off
      GPIO_ClrPin(GPIOA, TP1_Pin);
      // Turn off clock interrupts
      HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);
      // Check again, just in case a command has just come in
      if (!active) {
        // Go to power saving mode (6.4mA->1.6mA)
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      }
      // Clock interrupts back on
      HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
      // Blue activity LED on, activates before the end of 1 clock cycle after WFI
      GPIO_SetPin(GPIOA, TP1_Pin);
      // Just in case
      inactive_count = 0;
    }
  }
}

void Error_Handler(void) {
  __disable_irq();
  GPIO_ClrPin(GPIOA, TP1_Pin);
  GPIO_SetPin(GPIOA, LED_Red_Pin);
  GPIO_SetPin(GPIOA, LED_Green_Pin);
  while (1) {
  }
}

