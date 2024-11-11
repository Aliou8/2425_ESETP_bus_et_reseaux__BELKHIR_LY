#include "bmp280.h"
#include "usart.h"

extern I2C_HandleTypeDef hi2c1;
static uint8_t BMP280_ADDRESS = (0x77 << 1);

uint8_t BMP280_REG_ID = 0xD0;
static uint8_t BMP280_ID_Value = 0x58;

static uint8_t BMP280_REG_CONFIG = 0xF4;
static uint8_t BMP280_REG_CALIB_DATA = 0x88;
static uint8_t BMP280_REG_TEMP = 0xFA;
static uint8_t BMP280_REG_PRESSURE = 0xF7;

short dig_T1 = 0;
signed short dig_T2 = 0;
signed short dig_T3 = 0;
short dig_P1 = 0;
signed short dig_P2 = 0;
signed short dig_P3 = 0;
signed short dig_P4 = 0;
signed short dig_P5 = 0;
signed short dig_P6 = 0;
signed short dig_P7 = 0;
signed short dig_P8 = 0;
signed short dig_P9 = 0;

uint32_t t_fine;

static int BMP280_RegisterWrite(uint8_t reg, uint8_t data) {
    uint8_t buffer[2] = {reg, data};
    if (HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDRESS, buffer, sizeof(buffer), HAL_MAX_DELAY)) {
        return 0;
    }
    return 1;
}

static int BMP280_RegisterRead(uint8_t reg, uint8_t *data, uint16_t length) {
    if (HAL_OK != HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDRESS, &reg, 1, HAL_MAX_DELAY)) {
        return 1;
    }
    if (HAL_OK != HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDRESS, data, length, HAL_MAX_DELAY)) {
        return 1;
    }
    return 0;
}

void BMP280_Init(void) {
    uint8_t data[1];
    uint8_t ConfigValue = 0x57;
    if (1 == BMP280_RegisterWrite(BMP280_REG_CONFIG, ConfigValue)) {
        printf("Erreur pendant l'écriture de la configuration \r\n");
        return;
    }
    if (HAL_OK != HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDRESS, data, sizeof(data), HAL_MAX_DELAY)) {
        return;
    }
    printf("La valeur configurée est : 0x%X \r\n", data[0]);
}

void BMP280_ReadID(void) {
    uint8_t idValue;
    if (1 == BMP280_RegisterRead(BMP280_REG_ID, &idValue, 1)) {
        printf("Problème de lecture du registre ID \r\n");
        return;
    }
    if (BMP280_ID_Value != idValue) {
        printf("La valeur du registre ID n'est pas bonne \r\n");
        return;
    }
    printf("La valeur id = 0x%X\r\n", idValue);
}

void BMP280_ReadCalibrationData() {
    uint8_t calibData[BMP280_CALIB_DATA_LEN];
    if (1 == BMP280_RegisterRead(BMP280_REG_CALIB_DATA, calibData, BMP280_CALIB_DATA_LEN)) {
        printf("La lecture de calibration a échoué \r\n");
        return;
    }

    printf("Lecture des données de calibration OK \r\n");
    for (int i = 0; i < BMP280_CALIB_DATA_LEN; i++) {
        printf("Le contenu du registre 0x%X est  %d \r\n", 0x88 + i, calibData[i]);
    }

    dig_T1 = (calibData[0] | (calibData[1] << 8));
    dig_T2 = (calibData[2] | (calibData[3] << 8));
    dig_T3 = (calibData[4] | (calibData[5] << 8));

    dig_P1 = (calibData[6] | (calibData[7] << 8));
    dig_P2 = (calibData[8] | (calibData[9] << 8));
    dig_P3 = (calibData[10] | (calibData[11] << 8));
    dig_P4 = (calibData[12] | (calibData[13] << 8));
    dig_P5 = (calibData[14] | (calibData[15] << 8));
    dig_P6 = (calibData[16] | (calibData[17] << 8));
    dig_P7 = (calibData[18] | (calibData[19] << 8));
    dig_P8 = (calibData[20] | (calibData[21] << 8));
    dig_P9 = (calibData[22] | (calibData[23] << 8));
}

int32_t BMP280_ReadTemperature(void) {
    uint8_t tempData[BMP280_DATA_LEN];
    if (1 == BMP280_RegisterRead(BMP280_REG_TEMP, tempData, BMP280_DATA_LEN)) {
        return 0;
    }
    return (int32_t)((tempData[0] << 12) | (tempData[1] << 4) | (tempData[2]>> 4));
}

int32_t BMP280_ConvertTemperature(int32_t rawTemp) {
    int32_t var1, var2, T;

    var1 = (((rawTemp >> 3) - ((int32_t)dig_T1 << 1)) * (int32_t)dig_T2) >> 11;
    var2 = (((((rawTemp >> 4) - (int32_t)dig_T1) * ((rawTemp >> 4) - (int32_t)dig_T1)) >> 12) * (int32_t)dig_T3) >> 14;

    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

int32_t BMP280_ReadPressure(void) {
    uint8_t pressureData[BMP280_DATA_LEN];
    if (1 == BMP280_RegisterRead(BMP280_REG_PRESSURE, pressureData, BMP280_DATA_LEN)) {
        return 1;
    }
    return (int32_t)((pressureData[0] << 16) | (pressureData[1] << 4) |(pressureData[2] >> 4));
}

int32_t BMP280_ConvertPressure(int32_t rawPressure) {
    int32_t var1, var2, P;

    var1 = (t_fine >> 1) - 64000;
    var2 = ((var1 >> 2) * (var1 >> 2) >> 11) * dig_P6;
    var2 = var2 + ((var1 * dig_P5) << 1);
    var2 = (var2 >> 2) + (dig_P4 << 16);
    var1 = (((dig_P3 * ((var1 >> 2) * (var1 >> 2) >> 13)) >> 3) + ((dig_P2 * var1) >> 1)) >> 18;
    var1 = ((32768 + var1) * dig_P1) >> 25;

    if (var1 == 0) {
        return 0;
    }

    P = ((1048576 - rawPressure) - (var2 >> 12)) * 3125;
    if (P < 0x80000000) {
        P = (P << 1) / var1;
    } else {
        P = (P / var1) * 2;
    }

    var1 = (dig_P9 * ((P >> 3) * (P >> 3) >> 13)) >> 12;
    var2 = ((P >> 2) * dig_P8) >> 13;
    P = (int32_t)(P + ((var1 + var2 + dig_P7) >> 4));
    return P;
}
