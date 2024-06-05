void I2C_Init();
void I2C_Start();
void I2C_Write(uint8_t data);
void I2C_Address(uint8_t address);
void I2C_Stop();
void I2C_WriteMul(uint8_t *data, uint8_t size);
void I2C_ReadData(uint8_t address, uint8_t reg, int16_t* buffer, uint8_t length) ;
void I2C_WriteData(uint8_t address, uint8_t reg, uint8_t data);