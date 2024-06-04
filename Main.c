#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "I2C.h"
#include "Delay.h"
#include "LCD.h"
#include "QMC5883L.h"
#include "UART.h"

char message[100];

void SystemClock_Config() {
	// Kich hoat HSE (High-Speed External) oscillator
	RCC->CR |= RCC_CR_HSEON;
	while (!(RCC->CR & RCC_CR_HSERDY)){}; // Doi cho HSE san sang, HSERDY Flag set len 1

  // Cau hinh va kich hoat PLL
  RCC->CFGR |= RCC_CFGR_PLLSRC; // Chon HSE lam nguon PLL
  RCC->CFGR |= RCC_CFGR_PLLMULL9; // PLL x 9 de dat 72 MHz

  RCC->CR |= RCC_CR_PLLON; // Kich hoat PLL
  while (!(RCC->CR & RCC_CR_PLLRDY)); // Doi cho PLL san sang

  // Cau hinh cac bo chia AHB, APB1 và APB2
  RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // AHB = SYSCLK khang chia
  RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // APB1 = HCLK chia 2
  RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; // APB2 = HCLK không chia

  // Chuyen he thong Clock nguon sang PLL
  FLASH->ACR |= FLASH_ACR_LATENCY_2; // Cau hinh Flash latency 2 wait states
  RCC->CFGR |= RCC_CFGR_SW_PLL; // Chon PLL lam SYSCLK
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL); // Doi cho PLL duoc su dung lam SYSCLK
}

int main(void)
{
	int16_t x, y, z;
	char buffer[32];
	float heading;
	
	USART_Init();
	
	Delay_Config ();
	
	I2C1_Init();
	I2C2_Init();
	
	LCD_Init();
	
	QMC5883L_Init();
//	QMC5883L_ReadData(&x, &y, &z);
	
	LCD_PutCur(0, 0);
	LCD_SendString("Calibrating...");
//	
//	USART_SendString("Calibrating...");
//	DelayMs(100);
	QMC5883L_Calibrate();
//	QMC5883L_GetOffset(message);
//	USART_SendString(message);
//	DelayMs(10000);
//	LCD_Clear();
	
  while (1)
   {
//		QMC5883L_ReadData(&x, &y, &z);
//		sprintf(buffer, "X %d, Y %d\t", x, y);
//		USART_SendString(buffer); 
		 
		QMC5883L_GetCalibratedData(&x, &y, &z);
		heading = CalculateHeading(x, y);
		char* direct = CalculateDirect(heading);
//		 
//		sprintf(buffer, "X %d, Y %d\r\n", x, y);
//		
//		USART_SendString(buffer);
		 
		sprintf(buffer, "%s: %.2f", direct, heading); 
		LCD_PutCur (0,0);
		LCD_SendString(buffer);
//		 
//		sprintf(buffer, "%s: %.2f\r\n",direct, heading);
//		USART_SendString(buffer);
		 
//		LCD_PutCur (1,0);
//		LCD_SendString(buffer);
//		DelayMs(1000);
//		LCD_Clear();
//		 USART_SendString("Hello");
		 DelayMs(300);
		 LCD_Clear();
   }
}
