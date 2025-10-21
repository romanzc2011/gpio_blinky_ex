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
static void TIMx_Init(TIM_TypeDef *TIMx, IRQn_Type irqn, uint16_t psc, uint32_t arr);

__IO uint8_t counter = 0;
__IO uint8_t pressed = 0;

int main(void)
{
	uint16_t TIM4_PSC = 6;
	uint32_t TIM4_ARR = (59999-1);

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIODEN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN | RCC_APB1ENR_TIM2EN;

	GPIOA_Init();
	GPIOD_Init();

	TIMx_Init(TIM4, TIM4_IRQn, TIM4_PSC, TIM4_ARR);  // Init TIM4

	while(1) {
		/******* BUTTON PUSH SECTION ***********************/
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

		/******* BLINK LED14 via TIMER ***********************/

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
    GPIOD->OTYPER &= ~(1UL << LED13);
    GPIOD->OSPEEDR &= ~(3UL << (LED13 * 2));
    GPIOD->PUPDR &= ~(3UL << (LED13 * 2));

    // LED15
	GPIOD->MODER   &= ~(3UL << (LED15 * 2));
	GPIOD->MODER   |=  (1UL << (LED15 * 2));
	GPIOD->OTYPER  &= ~(1UL << LED15);
	GPIOD->OSPEEDR &= ~(3UL << (LED15 * 2));
	GPIOD->PUPDR   &= ~(3UL << (LED15 * 2));

	// LED12 --- This LED will blink 1/sec on TIM2
	GPIOD->MODER   &= ~(3UL << (LED12 * 2));
	GPIOD->MODER   |=  (1UL << (LED12 * 2));
	GPIOD->OTYPER  &= ~(1UL << LED12);
	GPIOD->OSPEEDR &= ~(3UL << (LED12 * 2));
	GPIOD->PUPDR   &= ~(3UL << (LED12 * 2));
}

static void TIMx_Init(TIM_TypeDef *TIMx, IRQn_Type irqn, uint16_t psc, uint32_t arr)
{
	TIMx->CR1 &= ~TIM_CR1_CEN;

	TIMx->ARR = arr;
	TIMx->PSC = psc;

	TIMx->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_OPM);  // upcount, continuous

	TIMx->EGR = TIM_EGR_UG;      // Load PSC/ARR now
	TIMx->SR = 0;                // Clear flags
	TIMx->DIER |= TIM_DIER_UIE;  // update interrupt

	NVIC_SetPriority(irqn, 15);
	NVIC_EnableIRQ(irqn);

	TIMx->CR1 |= TIM_CR1_CEN;
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
