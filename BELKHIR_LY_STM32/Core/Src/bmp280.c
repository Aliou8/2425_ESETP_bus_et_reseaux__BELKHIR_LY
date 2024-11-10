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
static uint32_t t_fine ;

static int BMP280_RegisterWrite(uint8_t reg, uint8_t data) {
    uint8_t buffer[2] = {reg, data};
    if (HAL_OK == HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDRESS, buffer, sizeof(buffer), HAL_MAX_DELAY)) {
        return 0;
    }
    return 1;
}

static int BMP280_RegisterRead(uint8_t reg, uint8_t *data) {
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
    printf("La valeur id = 0x%X\r\n", idValue);
}

void BMP280_ReadCalibrationData(BMP280_CompenParameter_t * param) {
	int8_t calibData[BMP280_CALIB_DAATA_LEN];
    if (1 == BMP280_RegisterRead(BMP280_REG_CALIB_DATA, calibData)) {
        printf("La lecture de calibration a échoué \r\n");
        return;  // Quitter la fonction après une erreur
    }

    printf("Lecture des données de calibration OK \r\n");
    for (int i = 0; i < BMP280_CALIB_DAATA_LEN; i++) {
        printf("Le contenu du registre 0x%X est  %d \r\n", 0x88+i, calibData[i]);
    }
    param->dig_T1 = (uint32_t)(calibData[0] | (calibData[1] << 8));
    param->dig_T2 = (int32_t)(calibData[2] | (calibData[3] << 8));
    param->dig_T3 = (int32_t)(calibData[4] | (calibData[5] << 8));
    
    param->dig_P1 = (uint32_t)(calibData[6] | (calibData[7] << 8));
    param->dig_P2 = (int32_t)(calibData[8] | (calibData[9] << 8));
    param->dig_P3 = (int32_t)(calibData[10] | (calibData[11] << 8));
    param->dig_P4 = (int32_t)(calibData[12] | (calibData[13] << 8));
    param->dig_P5 = (int32_t)(calibData[14] | (calibData[15] << 8));
    param->dig_P6 = (int32_t)(calibData[16] | (calibData[17] << 8));
    param->dig_P7 = (int32_t)(calibData[18] | (calibData[19] << 8));
    param->dig_P8 = (int32_t)(calibData[20] | (calibData[21] << 8));
    param->dig_P9 = (int32_t)(calibData[22] | (calibData[23] << 8));

}

/*----------------------------------------------------------------------------*/


int32_t BMP280_ReadTemperature(void) {
    uint8_t tempData[BMP280_DATA_LEN];
    if (1 == BMP280_RegisterRead(BMP280_REG_TEMP, tempData)) {
        return 0;  // Quitter la fonction après une erreur
    }
    return ((uint32_t)(tempData[0]) << 12) | ((uint32_t)(tempData[1]) << 4) | ((uint32_t)(tempData[2]) >> 4);
}

int32_t BMP280_ConvertTemperature(int32_t rawTemp, BMP280_CompenParameter_t *param) {
    int32_t var1, var2, T;

    // Calcul de var1 en utilisant des entiers
    var1 = (((rawTemp >> 3) - ((int32_t)param->dig_T1 << 1)) * (int32_t)param->dig_T2) >> 11;

    // Calcul de var2 en utilisant des entiers
    var2 = (((((rawTemp >> 4) - (int32_t)param->dig_T1) * ((rawTemp >> 4) - (int32_t)param->dig_T1)) >> 12) * (int32_t)param->dig_T3) >> 14;

    // Calcul de t_fine pour le calibrage de la température
    t_fine = var1 + var2;

    // Conversion finale en température en dixièmes de degré Celsius
    T = (t_fine * 5 + 128) >> 8;  // Donne la température en centièmes de °C

    return T ;  // Pour obtenir la température en dixièmes de °C
}



int32_t BMP280_ReadPressure(void)
{
    uint8_t pressureData[3];
    if (1 == BMP280_RegisterRead(BMP280_REG_PRESSURE, pressureData)) {
        return 1;  // Quitter la fonction après une erreur
    }

    return ((uint32_t)(pressureData[0] << 16)) | ((uint32_t)pressureData[1] << 4) | (uint32_t)(pressureData[2]>>4);
}

int32_t BMP280_ConvertPressure(int32_t rawPressure, BMP280_CompenParameter_t *param) {
    int32_t var1, var2, P;

    // Calcul de var1 avec des entiers uniquement
    var1 = (t_fine >> 1) - 64000;
    var2 = ((var1 >> 2) * (var1 >> 2) >> 11) * param->dig_P6;
    var2 = var2 + ((var1 * param->dig_P5) << 1);
    var2 = (var2 >> 2) + (param->dig_P4 << 16);
    var1 = (((param->dig_P3 * ((var1 >> 2) * (var1 >> 2) >> 13)) >> 3) + ((param->dig_P2 * var1) >> 1)) >> 18;
    var1 = ((32768 + var1) * param->dig_P1) >> 25;

    if (var1 == 0) {
        return 0;  // Pour éviter la division par zéro
    }

    // Calcul de la pression
    P = ((1048576 - rawPressure) - (var2 >> 12)) * 3125;
    if (P < 0x80000000) {  // Si P est négatif
        P = (P << 1) / var1;  // Décalage à gauche et division par var1
    } else {
        P = (P / var1) * 2;  // Division et multiplication par 2
    }

    // Calcul final de la pression P, en considérant les ajustements de var2
    var1 = (param->dig_P9 * ((P >> 3) * (P >> 3) >> 13)) >> 12;
    var2 = ((P >> 2) * param->dig_P8) >> 13;

    P = P + ((var1 + var2 + param->dig_P7) >> 4);

    return P/100;  // Retourner la pression compensée
}

