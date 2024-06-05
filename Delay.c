#include "stm32f10x.h"

/* Delay Systick */
void Delay_Config(void) {
	/* enable clock cho timer2 */
	RCC->APB1ENR |= 0x01;
	/* Dat reload la gia tri cao nhat */
	TIM2->ARR = 0xFFFF;
	/* Set gia tri cho thanh PSC de chia: 1 tick = fCLK/(PSC[15]-1) */
	TIM2->PSC = 71;
	TIM2->CR1 |= 0x01;
	TIM2->EGR |= 0x01;
}

void DelayUs(uint32_t us) {
	TIM2->CNT = 0;
	while(TIM2->CNT < us);
}

void DelayMs(uint32_t ms) {
	for(int i=0; i<ms; i++)
	{
		DelayUs(1000);
	}
}
