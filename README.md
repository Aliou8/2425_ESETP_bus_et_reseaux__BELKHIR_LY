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
- **Registre** : `0xF4`
- **Valeur** : `0x57` pour `0b010` ,`0b011` et `0b11`

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

