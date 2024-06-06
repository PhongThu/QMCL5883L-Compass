#include "stm32f10x.h"
#include "I2C.h"
#include "Delay.h"
#include "math.h"
#include "stdio.h"

#define QMC5883L_ADDRESS 0x0D
#define HMC5883L_ADDRESS 0x1E

float PI = 3.14158;
float offsetX = 0.0;
float offsetY = 0.0;
float offsetZ = 0.0;
float scaleX = 1.0;
float scaleY = 1.0;
float scaleZ = 1.0;
float avg_scale = 1.0;

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
	else if (k < 330)
		return "TB";
	else 
		return "B";
}

void HMC5883L_Init() {
    // Cau hinh thanh ghi dieu khien A
    I2C_WriteData(HMC5883L_ADDRESS, 0x00, 0x70);

    // Cau hinh thanh ghi dieu khien B
    I2C_WriteData(HMC5883L_ADDRESS, 0x01, 0xA0);

    // Cau h�nh thanh ghi mode
    I2C_WriteData(HMC5883L_ADDRESS, 0x02, 0x00);
}

void HMC5883L_ReadData(int16_t* x, int16_t* y, int16_t* z) {
    int16_t buffer[6];
    I2C_ReadData(HMC5883L_ADDRESS, 0x03, buffer, 6);

    *x = (int16_t)(buffer[0] << 8 | buffer[1]);
    *y = (int16_t)(buffer[2] << 8 | buffer[3]);
    *z = (int16_t)(buffer[4] << 8 | buffer[5]);
}

void HMC5883L_Calibrate() {

	int16_t x, y, z;
	int16_t xMin = INT16_MAX, xMax = INT16_MIN;
	int16_t yMin = INT16_MAX, yMax = INT16_MIN;
	int16_t zMin = INT16_MAX, zMax = INT16_MIN;
	HMC5883L_ReadData(&x, &y, &z);
	DelayMs(1);
	for (int i = 0; i < 10000; i++) {
		HMC5883L_ReadData(&x, &y, &z);
		
		if (x < xMin) xMin = x;
		if (x > xMax) xMax = x;
		if (y < yMin) yMin = y;
		if (y > yMax) yMax = y;
		if (z < zMin) zMin = z;
		if (z > zMax) zMax = z;
		
		DelayUs(100);
	}
	
	offsetX = (xMax + xMin) / 2.0;
  offsetY = (yMax + yMin) / 2.0;
  offsetZ = (zMax + zMin) / 2.0;
	
	scaleX = (xMax - xMin) / 2.0;
	scaleY = (yMax - yMin) / 2.0;
	scaleZ = (zMax - zMin) / 2.0;
	
	avg_scale = (scaleX + scaleY + scaleZ) / 3;
	
}

void HMC5883L_GetCalibratedData(int16_t *x, int16_t *y, int16_t *z) {
    int16_t rawX, rawY, rawZ;
    HMC5883L_ReadData(&rawX, &rawY, &rawZ);

    *x = (rawX - offsetX) * avg_scale / scaleX; // Gia tri cuong do tu truong
    *y = (rawY - offsetY) * avg_scale / scaleY;
    *z = (rawZ - offsetZ) * avg_scale / scaleZ;
}
float CalculateHeading(float x, float y) {
	float k = atan2(y,x) * 180 / PI;
	if (k < 0)
		k = k+360;
	return k;
}

void HMC5883L_GetOffset(char* s) {
	sprintf(s,"OffsetX: %.2f, OffsetY: %.2f, OffsetZ: %.2f\r\n",offsetX, offsetY, offsetZ);
}


float QMC_CalculateHeading(float x, float y) {
	float k = atan2(x, y)*180/PI;
	if (k < 0)
		k = 180 - k;
	return k;
}
void QMC5883L_Init(void) {
    // Set QMC5883L to continuous measurement mode, 200Hz, 8G range
    I2C_WriteData(QMC5883L_ADDRESS, 0x09, 0x1D); //0001 1101 
}
void QMC5883L_ReadData(int16_t* x, int16_t* y, int16_t* z) {
    int16_t buffer[6];
    I2C_ReadData(QMC5883L_ADDRESS, 0x00, buffer, 6);
    *x = (int16_t)(buffer[1] << 8 | buffer[0]);
    *y = (int16_t)(buffer[3] << 8 | buffer[2]);
    *z = (int16_t)(buffer[5] << 8 | buffer[4]);
}
void QMC5883L_Calibrate() {

	int16_t x, y, z;
	int16_t xMin = INT16_MAX, xMax = INT16_MIN;
	int16_t yMin = INT16_MAX, yMax = INT16_MIN;
	int16_t zMin = INT16_MAX, zMax = INT16_MIN;
	QMC5883L_ReadData(&x, &y, &z);
	DelayMs(1);
	for (int i = 0; i < 10000; i++) {
		QMC5883L_ReadData(&x, &y, &z);
		
		if (x < xMin) xMin = x;
		if (x > xMax) xMax = x;
		if (y < yMin) yMin = y;
		if (y > yMax) yMax = y;
		if (z < zMin) zMin = z;
		if (z > zMax) zMax = z;
		
		DelayUs(1000);
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