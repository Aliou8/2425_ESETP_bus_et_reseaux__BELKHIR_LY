#ifndef __BMP280_H__
#define __BMP280_H__

#include <stdint.h>
#include <stdio.h>

#define BMP280_DATA_LEN 3
#define BMP280_CALIB_DAATA_LEN 24

typedef struct
{
    uint8_t dig_T1;
    int8_t dig_T2;
    int8_t dig_T3;
    uint8_t dig_P1;
    int8_t dig_P2;
    int8_t dig_P3;
    int8_t dig_P4;
    int8_t dig_P5;
    int8_t dig_P6;
    int8_t dig_P7;
    int8_t dig_P8;
    int8_t dig_P9;
} BMP280_CompenParameter_t;

void BMP280_Init(void);
void BMP280_ReadID(void);
void BMP280_ReadCalibrationData(void);
uint32_t BMP280_ReadTemperature(void);
uint32_t BMP280_ConvertTemperature(uint32_t rawTemp, BMP280_CompenParameter_t * param) ;
uint32_t BMP280_ReadPressure(void);
uint32_t BMP280_ConvertPressure(uint32_t rawPressure, BMP280_CompenParameter_t * param);
void ParameterFill(BMP280_CompenParameter_t * param) ;


#endif // BMP280_H
