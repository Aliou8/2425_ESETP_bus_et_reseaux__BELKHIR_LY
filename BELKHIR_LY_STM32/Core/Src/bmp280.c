#include "bmp280.h"
#include "stm32f4xx_hal.h"

extern I2C_HandleTypeDef hi2c1;
static uint8_t BMP280_ADDRESS = (0x77 << 1);

uint8_t BMP280_REG_ID = 0xD0;
static uint8_t BMP280_ID_Value = 0x58;

static uint8_t BMP280_REG_CONFIG = 0xF4;
static uint8_t BMP280_REG_CALIB_DATA = 0x88;
static uint8_t BMP280_REG_TEMP = 0xFA;
static uint8_t BMP280_REG_PRESSURE = 0xF7;

static uint8_t calibData[BMP280_CALIB_DAATA_LEN];
static uint32_t t_fine ;

int BMP280_RegisterWrite(uint8_t reg, uint8_t data) {
    uint8_t buffer[2] = {reg, data};
    if (HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDRESS, buffer, sizeof(buffer), HAL_MAX_DELAY)) {
        return 0;
    }
    return 1;
}

int BMP280_RegisterRead(uint8_t reg, uint8_t *data) {
    if (HAL_OK != HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDRESS, &reg, 1, HAL_MAX_DELAY)) {
        return 1;
    }
    if (HAL_OK != HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDRESS, data, sizeof(data), HAL_MAX_DELAY)) {
        return 1;
    }
    return 0;
}

void BMP280_Init(void) {
    uint8_t ConfigValue = 0x57;
    if (1 == BMP280_RegisterWrite(BMP280_REG_CONFIG, ConfigValue)) {
        printf("Erreur pendant l'écriture de la configuration \r\n");
        return;  // Quitter la fonction après une erreur
    }
    printf("Configuration OK \r\n");
}

void BMP280_ReadID(void) {
    uint8_t idValue;
    if (1 == BMP280_RegisterRead(BMP280_REG_ID, &idValue)) {
        printf("Problème de lecture du registre ID \r\n");
        return;  // Quitter la fonction après une erreur
    }
    if (BMP280_ID_Value != idValue) {
        printf("La valeur du registre ID n'est pas bonne \r\n");
        return;  // Quitter la fonction si la valeur n'est pas correcte
    }
    printf("La valeur lue est : %u\r\n", idValue);
}

void BMP280_ReadCalibrationData(void) {
    if (1 == BMP280_RegisterRead(BMP280_REG_CALIB_DATA, calibData)) {
        printf("La lecture de calibration a échoué \r\n");
        return;  // Quitter la fonction après une erreur
    }

    printf("Lecture des données de calibration OK \r\n");
    for (int i = 0; i < BMP280_CALIB_DAATA_LEN; i++) {
        printf("La valeur lue %d est 0x%X \r\n", i, calibData[i]);
    }
}

s BMP280_ReadTemperature(void) {
    uint8_t tempData[BMP280_DATA_LEN];
    if (1 == BMP280_RegisterRead(BMP280_REG_TEMP, tempData)) {
        return 0;  // Quitter la fonction après une erreur
    }
    return ((uint32_t)tempData[0] << 16) | ((uint32_t)tempData[1] << 8) | (uint32_t)tempData[2];
}

uint32_t BMP280_ConvertTemperature(uint32_t rawTemp, int32_t t1, int32_t t2, int32_t t3) {
    int32_t var1, var2, T;

    var1 = ((rawTemp >> 3) - (t1 << 1)) * t2 >> 11;
    var2 = ((((rawTemp >> 4) - t1) * ((rawTemp >> 4) - t1)) >> 12) * t3 >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

uint32_t BMP280_ReadPressure(void) {
    uint8_t pressureData[3];
    if (1 == BMP280_RegisterRead(BMP280_REG_PRESSURE, pressureData)) {
        return 1;  // Quitter la fonction après une erreur
    }

    return ((uint32_t)pressureData[0] << 16) | ((uint32_t)pressureData[1] << 8) | (uint32_t)pressureData[2];
}

uint32_t BMP280_ConvertPressure(uint32_t rawPressure, uint32_t *p) {
    uint32_t var1, var2, P;
    var1 = (t_fine >> 1) - 64000;
    var2 = ((var1 >> 2) * (var1 >> 2) >> 11) * p[5];
    var2 = var2 + ((var1 * p[4]) << 1);
    var2 = (var2 >> 2) + (p[3] << 16);
    var1 = (((p[2] * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + (((p[1]) * var1) >> 1)) >> 18;
    var1 = ((32768 + var1) * p[0]) >> 25;
    if (var1 == 0) {
        return 0;
    }
    P = ((1048576 - rawPressure) - (var2 >> 12)) * 3125;
    if (P < 0x80000000) {
        P = (P << 1) / var1;
    } else {
        P = (P / var1) * 2;
    }
    return P;
}
