#include <stdint.h>
#include "stm32f407xx.h"
#include "core_cm4.h"

/* UIF (Update Interrupt Flag) is set when there is a pending interrupt,
 * The UIF is a bit that is set in the SR register (Status Register)
 * UIF = 1 means the Timer overflowed event pending, hardware set flag
 * Interrrupt pending in NVIC: the interrupt line is asserted (because UIF is set)
 * waiting for CPU to service it
 * (TIMx->SR & TIM_SR_UIF) != 0 means a TIMER update event has occured
 *
 *
 * This is a simple demonstration of a General Purpose TIMER blinking an
 * LED */

#define PA0    0
#define LED12  12    // GREEN
#define LED13  13    // ORANGE
#define LED14  14    // RED
#define LED15  15    // BLUE

static void GPIOA_Init(void);
static void GPIOD_Init(void);
static void TIMx_Init(TIM_TypeDef *TIMx, IRQn_Type irqn, uint16_t psc, uint32_t arr);

int main(void)
{
	uint16_t TIM4_PSC = 1343;
	uint32_t TIM4_ARR = 31249;

	uint32_t TIM2_ARR = 30000;
	uint16_t TIM2_PSC = 1200;

	uint16_t TIM3_PSC = 1100;
	uint32_t TIM3_ARR = 31000;

	uint16_t TIM5_PSC = 1000;
	uint32_t TIM5_ARR = 31500;

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIODEN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN | RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM5EN;

	GPIOA_Init();
	GPIOD_Init();

	TIMx_Init(TIM2, TIM2_IRQn, TIM2_PSC, TIM2_ARR);  // Init TIM2
	TIMx_Init(TIM3, TIM3_IRQn, TIM3_PSC, TIM3_ARR);  // Init TIM3
	TIMx_Init(TIM4, TIM4_IRQn, TIM4_PSC, TIM4_ARR);  // Init TIM4
	TIMx_Init(TIM5, TIM5_IRQn, TIM5_PSC, TIM5_ARR);  // Init TIM5

	/* Loop until the update event flag is set*/
	while (
			((TIM4->SR & TIM_SR_UIF) == 0) &&
			((TIM2->SR & TIM_SR_UIF) == 0) &&
			((TIM3->SR & TIM_SR_UIF) == 0) &&
			((TIM5->SR & TIM_SR_UIF) == 0)
	);
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

	// LED15
	GPIOD->MODER   &= ~(3UL << (LED14 * 2));
	GPIOD->MODER   |=  (1UL << (LED14 * 2));
	GPIOD->OTYPER  &= ~(1UL << LED14);
	GPIOD->OSPEEDR &= ~(3UL << (LED14 * 2));
	GPIOD->PUPDR   &= ~(3UL << (LED14 * 2));
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

	TIMx->CR1 |= TIM_CR1_CEN;   // Start counting
}

/* TIM2 Interrupt handler */
void TIM2_IRQHandler(void)
{
	if (TIM2->SR & TIM_SR_UIF) {
		TIM2->SR &= ~TIM_SR_UIF;
		GPIOD->ODR ^= (1U << LED12);
	}
}

/* TIM3 Interrupt handler */
void TIM3_IRQHandler(void)
{
	if (TIM3->SR & TIM_SR_UIF) {
		TIM3->SR &= ~TIM_SR_UIF;
		GPIOD->ODR ^= (1U << LED13);
	}
}

/* TIM4 Interrupt handler */
void TIM4_IRQHandler(void)
{
	if (TIM4->SR & TIM_SR_UIF) {
		TIM4->SR &= ~TIM_SR_UIF;
		GPIOD->ODR ^= (1U << LED15);
	}
}

/* TIM5 Interrupt handler */
void TIM5_IRQHandler(void)
{
	if (TIM5->SR & TIM_SR_UIF) {
		TIM5->SR &= ~TIM_SR_UIF;
		GPIOD->ODR ^= (1U << LED14);
	}
}


