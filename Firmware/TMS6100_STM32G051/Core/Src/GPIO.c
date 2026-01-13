/*
 * GPIO.c
 *
 *  Created: Dec 2025
 *   Author: Radiofan
 */
#include "main.h"
#include "stdbool.h"
#include "GPIO.h"

void GPIO_Init_Add8_D(uint32_t Mode) {
  // Optimised for quick configuration of this specific pin
  GPIO_TypeDef  *GPIOx = GPIOA;
  uint32_t position = 4; // ADD8_Data_Pin = 2
  uint32_t temp;

  /* Check the parameters */
  assert_param(IS_GPIO_ALL_INSTANCE(GPIOx));
  assert_param(IS_GPIO_MODE(Mode));

  if ((Mode & GPIO_MODE) == MODE_OUTPUT) {
    /* Configure the IO Output Type */
    temp = GPIOx->OTYPER;
    temp &= ~(GPIO_OTYPER_OT0 << position);
    temp |= (((Mode & OUTPUT_TYPE) >> OUTPUT_TYPE_Pos) << position);
    GPIOx->OTYPER = temp;
  }

  /* Configure IO Direction mode (Input, Output, etc) */
  temp = GPIOx->MODER;
  temp &= ~(GPIO_MODER_MODE0 << (position * 2u));
  temp |= ((Mode & GPIO_MODE) << (position * 2u));
  GPIOx->MODER = temp;
}

void GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // GPIO Ports Clock Enable
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // Configure GPIO pin Output Level
  GPIO_ClrPin(GPIOA, TP1_Pin);
  GPIO_ClrPin(GPIOA, LED_Red_Pin);
  GPIO_ClrPin(GPIOA, LED_Green_Pin);

  // Configure GPIO pins : LED_Green_Pin TP1_Pin LED_Red_Pin
  GPIO_InitStruct.Pin = LED_Green_Pin|TP1_Pin|LED_Red_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Configure GPIO pins : ADD8_Data_Pin ADD4_Pin ADD2_Pin ADD1_Pin
  GPIO_InitStruct.Pin = ADD8_Data_Pin|ADD4_Pin|ADD2_Pin|ADD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Configure GPIO pin : TMS_CLK_Pin
  GPIO_InitStruct.Pin = TMS_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TMS_CLK_GPIO_Port, &GPIO_InitStruct);

  // Configure GPIO pins : M0_Pin M1_Pin
  GPIO_InitStruct.Pin = M0_Pin|M1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // EXTI interrupts
  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}
