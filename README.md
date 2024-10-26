# TP Bus et Réseaux

> Aliou LY et Bayazid BELKHIR

## TP 1 - Bus I2C

### Objectif
Interfacer un microcontrôleur STM32 avec des capteurs connectés via le bus I2C.

### 1. Capteur de Température et Pression BMP280

#### Adresses I²C possibles pour le BMP280
Le BMP280 peut utiliser deux adresses I²C différentes :
- **0x76** : lorsque la broche SDO est connectée à la masse (GND).
- **0x77** : lorsque la broche SDO est connectée à la tension d'alimentation (VDDIO).

#### Registre et Valeur d'Identification
- **Registre** : `0xD0`
- **Valeur** : `0x58` (valeur permettant d’identifier le BMP280)

#### Mode Normal du BMP280
Pour placer le BMP280 en mode normal :
- **Registre** : `0xF4`
- **Valeur** : `0x27` (configuration du capteur en mode normal avec oversampling x1 pour pression et température)

#### Registres contenant l'étalonnage
Les registres d’étalonnage sont situés de `0x88` à `0xA1` et de `0xE1` à `0xE7`. Ces registres contiennent les coefficients nécessaires pour compenser les mesures de température et de pression.

#### Registres de Température
- **Registres** : `0xFA`, `0xFB`, `0xFC`
- **Format** : 20 bits (les bits [7:0] de `0xFA`, `0xFB`, et les bits [7:4] de `0xFC` composent la valeur brute de la température)

#### Registres de Pression
- **Registres** : `0xF7`, `0xF8`, `0xF9`
- **Format** : 20 bits (les bits [7:0] de `0xF7`, `0xF8`, et les bits [7:4] de `0xF9` composent la valeur brute de la pression)

#### Calcul de la Température et de la Pression Compensées

Les fonctions suivantes permettent de calculer la température et la pression compensées en format entier 32 bits :

##### Fonction pour la Température

```c
int32_t bmp280_compensate_temperature(int32_t adc_T) {
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)calib_param.dig_T1 << 1))) * ((int32_t)calib_param.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib_param.dig_T1)) * ((adc_T >> 4) - ((int32_t)calib_param.dig_T1))) >> 12) *
            ((int32_t)calib_param.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}
uint32_t bmp280_compensate_pressure(int32_t adc_P) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib_param.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib_param.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib_param.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib_param.dig_P3) >> 8) + ((var1 * (int64_t)calib_param.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib_param.dig_P1) >> 33;
    if (var1 == 0) {
        return 0; // Eviter division par 0
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib_param.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib_param.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib_param.dig_P7) << 4);
    return (uint32_t)p;
}
