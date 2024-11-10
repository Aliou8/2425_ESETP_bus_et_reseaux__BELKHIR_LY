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
- **Valeur** : `0x58`

#### Mode Normal du BMP280
Pour placer le BMP280 en mode normal :
- **Registre** : `0xF4` pour les bits [1:0]
- **Valeur** : `0b11`

#### Registres contenant l'étalonnage
Les registres d’étalonnage sont situés de `0x88` à `0xA1`. Ces registres contiennent les coefficients nécessaires pour compenser les mesures de température et de pression.

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

uint32_t BMP280_ConvertTemperature(uint32_t rawTemp, BMP280_CompenParameter_t * param)
{
    int32_t var1, var2, T;
    var1 = ((rawTemp >> 3) - (param->dig_T1 << 1)) * param->dig_T2 >> 11;
    var2 = ((((rawTemp >> 4) - param->dig_T1) * ((rawTemp >> 4) - param->dig_T1)) >> 12) * param->dig_T3 >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}
```

##### Fonction pour la Pression
```c
uint32_t BMP280_ConvertPressure(uint32_t rawPressure, BMP280_CompenParameter_t * param) 
{
    int32_t var1, var2, P;
    
    var1 = (t_fine >> 1) - 64000;
    var2 = ((var1 >> 2) * (var1 >> 2) >> 11) * param->dig_P6;
    var2 = var2 + ((var1 * param->dig_P5) << 1);
    var2 = (var2 >> 2) + (param->dig_P4 << 16);
    var1 = (((param->dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((param->dig_P2 * var1) >> 1)) >> 18;
    var1 = ((32768 + var1) * param->dig_P1) >> 25;

    if (var1 == 0) {
        return 0;  // Pour éviter la division par zéro
    }

    P = ((1048576 - rawPressure) - (var2 >> 12)) * 3125;
    if (P < (int32_t)0x80000000) {
        P = (P << 1) / var1;
    } else {
        P = (P / var1) * 2;
    }

    return P;
}

```
## TP2 - Interfaçage STM32 - Raspberry
Le Raspberry Pi obtient son adresse IP via l'un des deux mécanismes suivants : 
Adresse IP statique : 
Si le fichier de configuration réseau du Raspberry Pi a été modifié (généralement dans /etc/dhcpcd.conf ou /etc/network/interfaces selon la version du système), il peut avoir une adresse IP statique. Cela signifie que l'adresse IP est définie manuellement et reste fixe, même après redémarrage ou changement de réseau ( On va obtenir laddres ip de raspberry depuis ladresse de ENSEA )




