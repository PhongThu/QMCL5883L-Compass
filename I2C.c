#include "stm32f10x.h"
#include "Delay.h"
#include "LCD.h"
void I2C_Init(void) {
	/* enable clock */ 
	RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; // I2C2
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // port B
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	
	/* config i2c gpio PB10 PB11 : alternate function */
	GPIOB->CRH |= GPIO_CRH_CNF10 | GPIO_CRH_CNF11;
	GPIOB->CRH |= GPIO_CRH_MODE10  | GPIO_CRH_MODE11;
	
	/*   */
	I2C2->CR1 |= I2C_CR1_SWRST;
	I2C2->CR1 &= ~ I2C_CR1_SWRST;
	
	I2C2->CR2 = 36; // Dat tan so la 36Mhz
	I2C2->OAR1 |= (1 << 14); // Cau hinh dia chi. Kich hoat che do 7 bit or 10 bit
	I2C2->CCR = 180; // Dat toc do cua I2C vs tan so 36MHz
	I2C2->TRISE = 37;	// Thoi gian tang toi da
	I2C2->CR1 |= I2C_CR1_PE; // Bat I2C, Peripheral
}

// Bat dau truyen i2c
void I2C_Start(void) {
		
	I2C2->CR1 |= I2C_CR1_ACK; // Bat che do ACK. Gui ACK sau khi nhan dc 1 bit tin hieu
	I2C2->CR1 |= I2C_CR1_START; // Tin hieu bat dau frame truyen 
	DelayMs(1); // Dam bao START da dc truyen
	while(!(I2C2->SR1 & I2C_SR1_SB)); // Cho bit START dc truyen xong
}
// Gui 1 byte data qua bus I2C
void I2C_Write(uint8_t data) {
	while(!(I2C2->SR1 & I2C_SR1_TXE)); // Cho den khi DR empty -> TXE = 1
	I2C2->DR = data; // Ghi data vao thanh ghi DR
	while(!(I2C2->SR1) & I2C_SR1_BTF); // Cho den khi truyen xong
}

// Gui dia chi den bus I2C. Kiem tra dia chi cua Slave xem co trung hay k
void I2C_Address(uint8_t address) {
	uint8_t addr = (uint8_t)(address << 1); // Dinh dang 7 bit
	I2C2->DR = addr;
	while(!(I2C2->SR1 & I2C_SR1_ADDR)); // Cho den khi dia chi truyen xong
	
	// Xoa bit ADDR
	(void) I2C2->SR1;
	(void) I2C2->SR2;
}
void I2C_Stop(void) {
	I2C2->CR1 |= I2C_CR1_STOP;
}


void I2C_WriteMul(uint8_t *data, uint8_t size) {
	while(!(I2C2->SR1 & I2C_SR1_TXE)); // Cho den khi DR empty -> TXE = 1
	
	// Gui bit data 
	while(size)
	{
		while(!(I2C2->SR1 & I2C_SR1_TXE));
		I2C2->DR = *data;
		data++;
		size--;
	}
	while(!(I2C2->SR1 & I2C_SR1_BTF));
}

void I2C_ReadData(uint8_t address, uint8_t reg, int16_t* buffer, uint8_t length) {
    // Bat dau truyen
    I2C2->CR1 |= I2C_CR1_START;
    while (!(I2C2->SR1 & I2C_SR1_SB));

    // Gui dia chi
    I2C2->DR = (address << 1);
    while (!(I2C2->SR1 & I2C_SR1_ADDR));
    (void)I2C2->SR2; 

    // Gui thanh ghi
    I2C2->DR = reg;
    while (!(I2C2->SR1 & I2C_SR1_TXE));

    // Khoi dong lai I2c de chuyen sang che do doc
    I2C2->CR1 |= I2C_CR1_START;
    while (!(I2C2->SR1 & I2C_SR1_SB));

    // Gui dia chi bit doc
    I2C2->DR = (address << 1) | 1;
    while (!(I2C2->SR1 & I2C_SR1_ADDR));
    (void)I2C2->SR2; // Doc SR2 de xoa co

    // Doc du lieu
    while (length) {
        if (length == 1) {
            I2C2->CR1 &= ~I2C_CR1_ACK; // Tat ACK
            I2C2->CR1 |= I2C_CR1_STOP; // Tao tín hieu STOP
        }

        if (I2C2->SR1 & I2C_SR1_RXNE) {
            *buffer = I2C2->DR;
            buffer++;
            length--;
        }
    }

    // Bat lai ACK cho lan doc sau
    I2C2->CR1 |= I2C_CR1_ACK;
}
void I2C_WriteData(uint8_t address, uint8_t reg, uint8_t data) {
    // Bat dau truyen
    I2C2->CR1 |= I2C_CR1_START;
    while (!(I2C2->SR1 & I2C_SR1_SB));

    // Gui dia chi
    I2C2->DR = (address << 1);
    while (!(I2C2->SR1 & I2C_SR1_ADDR));
    (void)I2C2->SR2; 

    // Gui thanh ghi
    I2C2->DR = reg;
    while (!(I2C2->SR1 & I2C_SR1_TXE));

    // Gui du lieu
    I2C2->DR = data;
    while (!(I2C2->SR1 & I2C_SR1_TXE));

    // Dung truyen
    I2C2->CR1 |= I2C_CR1_STOP;
}
