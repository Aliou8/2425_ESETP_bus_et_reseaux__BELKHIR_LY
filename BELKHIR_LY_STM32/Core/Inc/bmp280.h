#ifndef __BMP280_H__
#define __BMP280_H__

#include <stdint.h>
#include <stdio.h>

// Taille des données de température et de pression en octets
#define BMP280_DATA_LEN 3
// Taille des données de calibration en octets
#define BMP280_CALIB_DATA_LEN 24

// Prototypes des fonctions
void BMP280_Init(void);
void BMP280_ReadID(void);
void BMP280_ReadCalibrationData();
int32_t BMP280_ReadTemperature(void);
int32_t BMP280_ConvertTemperature(int32_t rawTemp);
int32_t BMP280_ReadPressure(void);
int32_t BMP280_ConvertPressure(int32_t rawPressure);

#endif // __BMP280_H__
