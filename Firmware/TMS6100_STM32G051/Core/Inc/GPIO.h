/*
 * GPIO.h
 *
 *  Created: Dec 2025
 *   Author: Radiofan
 */

void GPIO_Init(void);

void GPIO_Init_Add8_D(uint32_t Mode);

#define GPIO_SetPin(GPIOx, GPIO_Pin)  GPIOx->BSRR = (uint32_t)GPIO_Pin;
#define GPIO_ClrPin(GPIOx, GPIO_Pin)  GPIOx->BRR  = (uint32_t)GPIO_Pin;
#define GPIO_ReadPin(GPIOx, GPIO_Pin) ((GPIOx->IDR & GPIO_Pin) != 0x00u)
