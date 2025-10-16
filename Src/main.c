#include <stdint.h>
#include "stm32f407xx.h"
#include "core_cm4.h"

#define PA0    0
#define LED12  12
#define LED13  13
#define LED14  14
#define LED15  15

static void GPIOA_Init(void);
static void GPIOD_Init(void);
static void TIM4_Init(void);

__IO uint8_t counter = 0;
__IO uint8_t pressed = 0;

int main(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIODEN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

	GPIOA_Init();
	GPIOD_Init();
	TIM4_Init();

	while(1) {
		// Turn off LED15 if interrupt happened
		if ((GPIOD->ODR & (1U << LED15)) != 0) {
			GPIOD->ODR &= ~(1U << LED15);
		}

		// Press button and debounce
		if (pressed) {
			GPIOD->ODR ^= (1U << LED13);
			pressed = 0;
			while ((GPIOA->IDR & (1U << PA0)) != 0);
		}
	}
}

/* PA0 input with pull down */
static void GPIOA_Init(void)
{
	GPIOA->MODER &= ~(3UL << (PA0 * 2));

	GPIOA->PUPDR &= ~(3UL << (PA0 * 2));
	GPIOA->PUPDR |=  (2UL << (PA0 * 2));
}

/* PD15 output push-pull, low speed, no pull */
static void GPIOD_Init(void)
{
    /* MODER: 01 = output for PD15 */
    GPIOD->MODER &= ~(3UL << (LED13 * 2));
    GPIOD->MODER |=  (1UL << (LED13 * 2));

    /* Push-pull: clear bit 15 in OTYPER */
    GPIOD->OTYPER &= ~(1UL << LED13);

    /* Low speed */
    GPIOD->OSPEEDR &= ~(3UL << (LED13 * 2));

    /* No pull */
    GPIOD->PUPDR &= ~(3UL << (LED13 * 2));

    // PD15  <-- add this block
	GPIOD->MODER   &= ~(3UL << (LED15 * 2));
	GPIOD->MODER   |=  (1UL << (LED15 * 2));
	GPIOD->OTYPER  &= ~(1UL << LED15);
	GPIOD->OSPEEDR &= ~(3UL << (LED15 * 2));
	GPIOD->PUPDR   &= ~(3UL << (LED15 * 2));
}

static void TIM4_Init(void)
{
	TIM4->CR1 &= ~TIM_CR1_CEN;  // Disable timer clock                                                                                                             z
	TIM4->ARR = 59999 - 1;
	TIM4->PSC = 6;

	TIM4->CR1 &= ~TIM_CR1_DIR;
	TIM4->CR1 &= ~TIM_CR1_OPM;

	TIM4->DIER |= TIM_DIER_UIE;
	TIM4->EGR = TIM_EGR_UG;
	TIM4->SR = 0;

	NVIC_EnableIRQ(TIM4_IRQn);   // Enabling the TIM4 interrupt handler
	NVIC_SetPriority(TIM4_IRQn, 15);

	TIM4->CR1 |= TIM_CR1_CEN;
}

/* TIM4 Interrupt handler */
void TIM4_IRQHandler(void)
{
	if (TIM4->SR & TIM_SR_UIF) {
		TIM4->SR &= ~TIM_SR_UIF;
		GPIOD->ODR |= (1U << LED15);

		// sample PA0 button
		if ((GPIOA->IDR & (1UL << PA0)) != 0) { // is button high?
			if (counter++ >= 4) {
				pressed = 1;
				counter = 0;
			}
		} else {
			counter = 0;
		}
	}
}
