#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "I2C.h"
#include "Delay.h"
#include "LCD.h"
#include "HMC5883L.h"
#include "UART.h"

// Trang thai cua he thong

volatile uint8_t systemState = 1;
volatile uint8_t systemReset = 1;
volatile uint8_t switchState = 1;
volatile uint32_t ticks = 0;

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
  RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // AHB = SYSCLK khong chia
  RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // APB1 = HCLK chia 2
  RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; // APB2 = HCLK không chia

  // Chuyen he thong Clock nguon sang PLL
  FLASH->ACR |= FLASH_ACR_LATENCY_2; // Cau hinh Flash latency 2 wait states
  RCC->CFGR |= RCC_CFGR_SW_PLL; // Chon PLL lam SYSCLK
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL); // Doi cho PLL duoc su dung lam SYSCLK
}


void LED_Init() {
  // Bat clock cho GPIOB
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	
  // Cau hình PB8 va PB9 là output push-pull, toc do 2MHz
  GPIOB->CRH &= ~(GPIO_CRH_MODE8 | GPIO_CRH_MODE9); 
  GPIOB->CRH |=  (GPIO_CRH_MODE8_1 | GPIO_CRH_MODE9_1); // 2 MHz
  GPIOB->CRH &= ~(GPIO_CRH_CNF8 | GPIO_CRH_CNF9); // Push-pull
}

void Switch_Init() {
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	
	// Cau hinh PA0 la input pull up
  GPIOA->CRL &= ~0x0FF; // Reset
  GPIOA->CRL |= GPIO_CRL_CNF0_1; // Input pull up
  GPIOA->CRL |= GPIO_CRL_CNF1_1; // Input pull up
	GPIOA->ODR |= GPIO_ODR_ODR0 | GPIO_ODR_ODR1; 
}

void SysTick_Init() {
    SysTick->CTRL = 0;
    SysTick->LOAD = (SystemCoreClock / 1000) - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void EXTI_Config()
{
  // Kich hoat clock cho AFIO de cau hình EXTI
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

  // Ket noi EXTI line 0 voi PA0
  AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0;
  AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PA;

  // Cau hình EXTI line 0 do kich hoat ngat boi falling edge
  EXTI->PR |= EXTI_PR_PR0 | EXTI_PR_PR1; // clear co ngat PR0 PR1
	EXTI->FTSR |= EXTI_FTSR_TR0 | EXTI_FTSR_TR1; // ngat theo xuon xuong
	EXTI->IMR |= EXTI_IMR_MR0 | EXTI_EMR_MR1; // cho phep ngat A0, A1
	
	
  // Bat ngat EXTI line 0 trong NVIC
  NVIC->ISER[0] |= 0x40; // Kich hoat ngat EXTI0_IRQn : Position = 6
	NVIC->ISER[0] |= 0x80; // Kich hoat ngat EXTI1_IRQn : Position = 7

  // Thiet lap uu tien cho ngat EXTI line 0
  NVIC->IP[EXTI0_IRQn] = 0x03 << 4; // Muc uu tien thu nhat cho EXTI0_IRQn
  NVIC->IP[EXTI1_IRQn] = 0x02 << 4; // Muc uu tien thu hai cho EXTI1_IRQn
}

int main(void)
{	
	int16_t x, y, z;
	char buffer[32] = "";
	char* direct = "";
	float heading = 0.0;
	
	SystemClock_Config();
	LED_Init();
	Switch_Init();
	EXTI_Config();
	SysTick_Init();
	
	Delay_Config();
	I2C_Init();
	LCD_Init();
	HMC5883L_Init();

	LCD_PutCur(0, 0);
	LCD_SendString("Calibrating...");

	HMC5883L_Calibrate();
	DelayMs(1);
	LCD_Clear();
	
	while (1)
  {
		HMC5883L_GetCalibratedData(&x, &y, &z);
		heading = CalculateHeading(x, y);
		direct = CalculateDirect(heading);
		sprintf(buffer, "%-2s: %-3.0f", direct, heading); 
		LCD_PutCur (0,0);
		LCD_SendString(buffer);
  }
}

void SysTick_Handler(void) {
    static uint32_t ticksB8 = 0;
    static uint32_t ticksB9 = 0;

    ticks++;

    
    if ((++ticksB8 >= 1000) & (systemState == 1)) 
		{ 
      GPIOB->ODR &= ~GPIO_ODR_ODR9;  
			ticksB8 = 0;
      GPIOB->ODR ^= GPIO_ODR_ODR8; 
    }

    
    if ((++ticksB9 >= 500) & (systemState != 1)) 
		{ 
      GPIOB->ODR &= ~GPIO_ODR_ODR8;    
			ticksB9 = 0;
      GPIOB->ODR ^= GPIO_ODR_ODR9; 
    }
}
void EXTI0_IRQHandler()
{
	EXTI->PR |= EXTI_PR_PR0;
	systemState = !systemState;
}
void EXTI1_IRQHandler(void)
{
	EXTI->PR |= EXTI_PR_PR1;
	systemReset = 1;
}
