#include "stm32f10x.h"
#include "Delay.h"

void I2C1_Init(void) {
  // Bat clock cho I2C1 và GPIOB
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
  RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	
	
	GPIOB->CRL |= GPIO_CRL_CNF6 | GPIO_CRL_CNF7;
	GPIOB->CRL |= GPIO_CRL_MODE6  | GPIO_CRL_MODE7;
  GPIOB->ODR |= (GPIO_ODR_ODR6 | GPIO_ODR_ODR7);

  // Reset I2C1
  I2C1->CR1 |= I2C_CR1_SWRST;
  I2C1->CR1 &= ~I2C_CR1_SWRST;
	
  // Cau hình I2C1
  I2C1->CR2 |= 36; // PCLK1 = 36MHz
  I2C1->CCR |= 180; // 100kHz
  I2C1->TRISE = 37; // 1000ns / (1/36MHz) + 1
  I2C1->CR1 |= I2C_CR1_PE; // Kích hoat I2C1
	
}
void I2C1_WriteData(uint8_t address, uint8_t reg, uint8_t data) {
    // Bat dau truyen
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));

    // Gui dia chi
    I2C1->DR = (address << 1);
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR2; // Ğ?c SR2 d? xóa c?

    // Gui thanh ghi
    I2C1->DR = reg;
    while (!(I2C1->SR1 & I2C_SR1_TXE));

    // Gui du lieu
    I2C1->DR = data;
    while (!(I2C1->SR1 & I2C_SR1_TXE));

    // Dung truyen
    I2C1->CR1 |= I2C_CR1_STOP;
}

void I2C1_ReadData(uint8_t address, uint8_t reg, int16_t* buffer, uint8_t length) {
    // Bat dau truyen
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));

    // Gui dia chi
    I2C1->DR = (address << 1);
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR2; // Doc SR2 de xoa co

    // Gui thanh ghi
    I2C1->DR = reg;
    while (!(I2C1->SR1 & I2C_SR1_TXE));

    // Bat dau truyen nhan du lieu
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));

    // Gui dia chi
    I2C1->DR = (address << 1) | 1;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR2; // Doc SR2 de xoa co

    // Doc du lieu
    while (length) {
        if (length == 1) {
            I2C1->CR1 &= ~I2C_CR1_ACK; // Tat ACK
            I2C1->CR1 |= I2C_CR1_STOP; // Tao tín hieu STOP
        }

        if (I2C1->SR1 & I2C_SR1_RXNE) {
            *buffer = I2C1->DR;
            buffer++;
            length--;
        }
    }

    // Bat lai ACK
    I2C1->CR1 |= I2C_CR1_ACK;
}

void I2C2_Init(void) {
	/* enable clock */ 
	RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; // I2C1
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // port B
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	
	/* config i2c gpio PB10 PB11 : alternate function */
	GPIOB->CRH |= GPIO_CRH_CNF10 | GPIO_CRH_CNF11;
	GPIOB->CRH |= GPIO_CRH_MODE10  | GPIO_CRH_MODE11;
	
	/*   */
	I2C2->CR1 |= I2C_CR1_SWRST;
	I2C2->CR1 &= ~ I2C_CR1_SWRST;
	
	I2C2->CR2 = 36;
	I2C2->OAR1 |= (1 << 14);
	I2C2->CCR = 180;
	I2C2->TRISE = 37;	
	I2C2->CR1 |= I2C_CR1_PE;
}

void I2C2_Start(void) {
		
	I2C2->CR1 |= I2C_CR1_ACK;
	I2C2->CR1 |= I2C_CR1_START;
	DelayMs(1);
	while(!(I2C2->SR1 & I2C_SR1_SB));
}

void I2C2_Write(uint8_t data) {
	while(!(I2C2->SR1 & I2C_SR1_TXE));
	I2C2->DR = data;
	while(!(I2C2->SR1) & I2C_SR1_BTF);
}
void I2C2_Address(uint8_t address) {
	uint8_t addr = (uint8_t)(address << 1);
	I2C2->DR = addr;
	while(!(I2C2->SR1 & I2C_SR1_ADDR));
	
	(void) I2C2->SR1;
	(void) I2C2->SR2;
}
void I2C2_Stop(void) {
	I2C2->CR1 |= I2C_CR1_STOP;
}
void I2C2_WriteMul(uint8_t *data, uint8_t size) {
	while(!(I2C2->SR1 & I2C_SR1_TXE));
	while(size)
	{
		while(!(I2C2->SR1 & I2C_SR1_TXE));
		I2C2->DR = *data;
		data++;
		size--;
	}
	while(!(I2C2->SR1 & I2C_SR1_BTF));
}