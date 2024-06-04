void QMC5883L_Init(void);
void QMC5883L_ReadData(int16_t* x, int16_t* y, int16_t* z);
char* CalculateDirect(float k);
float CalculateHeading(float x, float y);
void QMC5883L_Calibrate();
void QMC5883L_GetCalibratedData(int16_t *x, int16_t *y, int16_t *z);