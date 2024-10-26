#ifndef __BMP280_H__
#define __BMP280_H__

#include <stdint.h>
#include <stdio.h>


#define BMP280_DATA_LEN 3
#define BMP280_CALIB_DAATA_LEN 24

void BMP280_Init(void);
void BMP280_ReadID(void);
void BMP280_ReadCalibrationData(void);
uint32_t BMP280_ReadTemperature(void);
uint32_t BMP280_ConvertTemperature(uint32_t rawTemp, int32_t t1, int32_t t2, int32_t t3);
uint32_t BMP280_ReadPressure(void);
uint32_t BMP280_ConvertPressure(uint32_t rawPressure, uint32_t *p);
int BMP280_RegisterRead(uint8_t  reg ,uint8_t * data);
int BMP280_RegisterWrite(uint8_t  reg ,uint8_t data );

#endif // BMP280_H
