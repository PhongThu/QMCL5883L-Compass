void I2C1_Init(void);
void I2C2_Init(void);
void I2C2_Start(void);
void I2C2_Write(uint8_t data);
void I2C2_Address(uint8_t address);
void I2C2_Stop(void);
void I2C2_WriteMul(uint8_t *data, uint8_t size);
void I2C1_WriteData(uint8_t address, uint8_t reg, uint8_t data);
void I2C1_ReadData(uint8_t address, uint8_t reg, int32_t* buffer, uint8_t length);