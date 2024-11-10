#ifndef __BMP280_H__
#define __BMP280_H__

#include <stdint.h>
#include <stdio.h>

#define BMP280_DATA_LEN 3
#define BMP280_CALIB_DAATA_LEN 24

typedef struct

{
    uint32_t dig_T1;
    int32_t dig_T2;
    int32_t dig_T3;
    uint32_t dig_P1;
    int32_t dig_P2;
    int32_t dig_P3;
    int32_t dig_P4;
    int32_t dig_P5;
    int32_t dig_P6;
    int32_t dig_P7;
    int32_t dig_P8;
    int32_t dig_P9;
} BMP280_CompenParameter_t;

void BMP280_Init(void);
void BMP280_ReadID(void);
void BMP280_ReadCalibrationData(BMP280_CompenParameter_t * param);
int32_t BMP280_ReadTemperature(void);
int32_t BMP280_ConvertTemperature(int32_t rawTemp, BMP280_CompenParameter_t * param) ;
int32_t BMP280_ReadPressure(void);
int32_t BMP280_ConvertPressure(int32_t rawPressure, BMP280_CompenParameter_t * param);


#endif // BMP280_H
