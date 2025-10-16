/*
 * blinky_addresses.h
 *
 *  Created on: Sep 1, 2025
 *      Author: roman
 */

#ifndef LED_TIMER_EX_H_
#define LED_TIMER_EX_H_
#define __IO         volatile

typedef struct {
	__IO uint32_t MODER; // Mode register
	__IO uint32_t OTYPER;  // Output type register
	__IO uint32_t OSPEEDR;  // Output speed register
	__IO uint32_t PUPDR;    // Pull-up/Pull-down register
	__IO uint32_t IDR;      // Output data register
	__IO uint32_t ODR;      // Input data register
} GPIO_TypeDef;

void GPIOA_Clock_Enable(void);
void GPIOD_Clock_Enable(void);
void GPIOD_Init(void);

#endif /* LED_TIMER_EX_H_ */
