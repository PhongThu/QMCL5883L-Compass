#include "stm32f10x.h"
#include <string.h>
#include <math.h>

#define QMC5883L_ADDRESS 0x0D
#define SCALE_FACTOR 3000 // T? l? chuy?n d?i cho ±2 gauss
#define SLAVE_ADDRESS_LCD 0x27

float PI = 3.14158;
float offsetX = 0.0;
float offsetY = 0.0;
float offsetZ = 0.0;
char message[100];

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
	Delay_ms(1);
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
void LCD_Write(uint8_t address, uint8_t *data, int size) {
	I2C2_Start();

	I2C2_Address(address);
	
   while (size)
   {
       while (!(I2C2->SR1 & I2C_SR1_TXE)); 
       I2C2->DR = *data; 
       data++;
       size--;
    }
    while (!(I2C2->SR1 & I2C_SR1_BTF)); 
}

/* LCD */
void lcd_send_cmd(char cmd) {
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  //en=1, rs=0
	data_t[1] = data_u|0x08;  //en=0, rs=0
	data_t[2] = data_l|0x0C;  //en=1, rs=0
	data_t[3] = data_l|0x08;  //en=0, rs=0
	LCD_Write (SLAVE_ADDRESS_LCD, data_t, 4);
}

void lcd_send_data (char data) {
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=1
	data_t[1] = data_u|0x09;  //en=0, rs=1
	data_t[2] = data_l|0x0D;  //en=1, rs=1
	data_t[3] = data_l|0x09;  //en=0, rs=1
	LCD_Write (SLAVE_ADDRESS_LCD, data_t, 4);
}

void lcd_clear (void) {
	lcd_send_cmd (0x80);
	for (int i=0; i<70; i++)
	{
		lcd_send_data (' ');
	}
}

void lcd_put_cur(int row, int col) {
    switch (row)
    {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0xC0;
            break;
    }

    lcd_send_cmd (col);
}

void lcd_init (void) {
	// 4 bit initialisation
	Delay_ms(50);  // wait for >40ms
	lcd_send_cmd (0x30);
	Delay_ms(5);  // wait for >4.1ms
	lcd_send_cmd (0x30);
	Delay_us(150);  // wait for >100us
	lcd_send_cmd (0x30);
	Delay_ms(10);
	lcd_send_cmd (0x20);  // 4bit mode
	Delay_ms(10);

  // dislay initialisation
	lcd_send_cmd (0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	Delay_ms(1);
	lcd_send_cmd (0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	Delay_ms(1);
	lcd_send_cmd (0x01);  // clear display
	Delay_ms(1);
	Delay_ms(1);
	lcd_send_cmd (0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	Delay_ms(1);
	lcd_send_cmd (0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
}

void lcd_send_string (char *str) {
	while (*str) lcd_send_data (*str++);
}



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

void Delay_us(uint32_t us) {
	TIM2->CNT = 0;
	while(TIM2->CNT < us);
}

void Delay_ms(uint32_t ms) {
	for(int i=0; i<ms; i++)
	{
		Delay_us(1000);
	}
}

void I2C1_WriteData(uint8_t address, uint8_t reg, uint8_t data) {
    // Bat dau truyen
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));

    // Gui dia chi
    I2C1->DR = (address << 1);
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR2; // Ð?c SR2 d? xóa c?

    // Gui thanh ghi
    I2C1->DR = reg;
    while (!(I2C1->SR1 & I2C_SR1_TXE));

    // Gui du lieu
    I2C1->DR = data;
    while (!(I2C1->SR1 & I2C_SR1_TXE));

    // Dung truyen
    I2C1->CR1 |= I2C_CR1_STOP;
}

void QMC5883L_Init(void) {
    // Set QMC5883L to continuous measurement mode, 200Hz, 8G range
    I2C1_WriteData(QMC5883L_ADDRESS, 0x09, 0x1D); //0001 1101 
}


void I2C1_ReadData(uint8_t address, uint8_t reg, int32_t* buffer, uint8_t length) {
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


void QMC5883L_ReadData(int16_t* x, int16_t* y, int16_t* z) {
    int32_t buffer[6];
    I2C1_ReadData(QMC5883L_ADDRESS, 0x00, buffer, 6);
    *x = (int16_t)(buffer[1] << 8 | buffer[0]);
    *y = (int16_t)(buffer[3] << 8 | buffer[2]);
    *z = (int16_t)(buffer[5] << 8 | buffer[4]);
}
char* CalculateDirect(float k) {
	if (k < 30)
		return "B";
	else if (k < 60)
		return "DB";
	else if (k < 120)
		return "D";
	else if (k < 150)
		return "DN";
	else if (k < 210)
		return "N";
	else if (k < 240)
		return "TN";
	else if (k < 300)
		return "T";
	else 
		return "B";
}

//float CalculateHeading(float x, float y) {
//	float heading ;
//	if (y > 0) 
//	{
//		heading = 90 - atan2(x, y) * 180 / PI;
//	}
//	else if (y < 0)
//	{
//		heading = 270 - atan2(x, y) * 180 / PI;
//	}
//	else if (x < 0)
//		heading = 180;
//	else
//		heading = 0;
//	
//    return heading;
//}

float CalculateHeading(float x, float y) {
	float k = atan2(x, y)*180/PI;
	if (k < 0)
		k = 180 - k;
	return k;
}
void QMC5883L_Calibrate() {

	int16_t x, y, z;
	int16_t xMin = INT16_MAX, xMax = INT16_MIN;
	int16_t yMin = INT16_MAX, yMax = INT16_MIN;
	int16_t zMin = INT16_MAX, zMax = INT16_MIN;
	QMC5883L_ReadData(&x, &y, &z);
	Delay_ms(1);
	for (int i = 0; i < 10000; i++) {
		QMC5883L_ReadData(&x, &y, &z);
		
		if (x < xMin) xMin = x;
		if (x > xMax) xMax = x;
		if (y < yMin) yMin = y;
		if (y > yMax) yMax = y;
		if (z < zMin) zMin = z;
		if (z > zMax) zMax = z;
		
		Delay_us(1000);
	}
	
	 offsetX = (xMax + xMin) / 2.0;
   offsetY = (yMax + yMin) / 2.0;
   offsetZ = (zMax + zMin) / 2.0;
}

void QMC5883L_GetCalibratedData(int16_t *x, int16_t *y, int16_t *z) {
    int16_t rawX, rawY, rawZ;
    QMC5883L_ReadData(&rawX, &rawY, &rawZ);

    *x = rawX - offsetX;
    *y = rawY - offsetY;
    *z = rawZ - offsetZ;
}
int main(void)
{
	int16_t x, y, z;
	char buffer[32];
	float heading;
	
	Delay_Config ();
	
	I2C1_Init();
	I2C2_Init();
	
	lcd_init ();
	
	QMC5883L_Init();
	QMC5883L_ReadData(&x, &y, &z);
	
	lcd_put_cur(0, 0);
	lcd_send_string("Wait for calibrate");
	QMC5883L_Calibrate();
	lcd_clear();
//	Delay_ms(10);
//	lcd_send_string("Complete");
//	lcd_clear();
	
  while (1)
   {
		QMC5883L_GetCalibratedData(&x, &y, &z);
		
		lcd_put_cur (0,0);
		sprintf(buffer, "X %d, Y %d", x, y);
		lcd_send_string(buffer);
		
		heading = CalculateHeading(x, y);
		char* direct = CalculateDirect(heading);
		lcd_put_cur (1,0);
		 sprintf(buffer, "%s: %.2f",direct, heading);
		lcd_send_string(buffer);
		Delay_ms(500);
		lcd_clear();
   }
}
