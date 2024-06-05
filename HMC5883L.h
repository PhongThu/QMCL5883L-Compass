char* CalculateDirect(float k);
float CalculateHeading(float x, float y);
void HMC5883L_Init(void);
void HMC5883L_ReadData(int16_t* x, int16_t* y, int16_t* z);
void HMC5883L_Calibrate();
void HMC5883L_GetCalibratedData(int16_t *x, int16_t *y, int16_t *z);
