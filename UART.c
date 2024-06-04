#include "stm32f10x.h"

void USART_Init(void)
{
	// Enable GPIOA and USART1 clock
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
	
	// Configure PA9 as alternate function push-pull (TX)
  GPIOA->CRH &= ~(0xF << 4); // Clear PA9 settings
  GPIOA->CRH |= (0xB << 4);  // Set PA9 as AF output push-pull, max speed 50 MHz

  // Configure PA10 as input floating (RX)
  GPIOA->CRH &= ~(0xF << 8); // Clear PA10 settings
  GPIOA->CRH |= (0x4 << 8);  // Set PA10 as input floating
	
  // Configure baud rate
  USART1->BRR = 0x1D4C; // Assuming 72 MHz clock, 9600 baud

  // Enable USART, TX and RX
  USART1->CR1 |= (1 << 13) | (1 << 3) | (1 << 2); // UE, TE, RE
}

void USART_SendChar(uint8_t ch)
{
    while (!(USART1->SR & (1 << 7))); // Wait until TXE (Transmit Data Register Empty) bit is set
    USART1->DR = ch; // Transmit data
		while (!(USART1->SR & 1<<6));
		USART1->SR &= ~(1<<6);
}

void USART_SendString(const char *str)
{
    while(*str)
    {
        USART_SendChar(*str++);
    }
}

uint8_t USART_ReceiveChar(void)
{
    while (!(USART1->SR & (1 << 5))); // Wait until RXNE (Read Data Register Not Empty) bit is set
    return (uint8_t)USART1->DR; // Receive data
}