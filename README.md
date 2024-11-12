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
La Raspberry Pi obtient son adresse IP grâce au DHCP. Lorsqu'elle est connectée à un réseau via Wi-Fi, elle envoie une demande d'adresse IP au routeur. Le routeur, qui agit en tant que serveur DHCP, lui attribue automatiquement une adresse IP privée unique pour le réseau local. 

### Port Série
#### Loppback
Avec la commande `minicom -D /dev/ttyAMA0` on revoie la valeur saisie dans le terminal minicom la liaison UART fonctionne

#### Communication avec la STM32
La gestion de la communication se fait dans le fichier `STM32_Raspberry` avec une fonction ```PI_RUN()``` qui se comporte comme un shell et assure la reception des commande .Une fois la commande términée elle est comparée puis la fonction ```PI_GetCommande``` renvoie la reponse .
``` C

// Fonction pour traiter les commandes reçues via UART
static void PI_GetCommand(char *buffer)
{
    // Vérifiez la commande et construisez la réponse sans afficher la commande
    if (strcmp(buffer, "GET_T") == 0) {
       int32_t tempNc = BMP280_ReadTemperature() ;
        int32_t temp = BMP280_ConvertTemperature(tempNc) ;
        int size = snprintf(printfBuffer, BUFFER_SIZE, "T=+%ld%ld.%ld%ld_C\r\n",(temp/1000)%10,(temp/100)%10,(temp/10)%10,temp%10);
        uart_write(printfBuffer, size);
    } else if (strcmp(buffer, "GET_P") == 0) {
       int32_t PresNc =BMP280_ReadPressure() ;
        int32_t Pres = BMP280_ConvertPressure(PresNc) ;
        int size = snprintf(printfBuffer, BUFFER_SIZE, "P=%ldPa\r\n",Pres);
        uart_write(printfBuffer, size);
    }
    else if (strncmp(buffer, "SET_K=", 6) == 0) {  // Vérifie si le buffer commence par "SET_K="
        int32_t value;
        if (sscanf(buffer + 6, "%ld", &value) == 1) {  // Extrait l'entier après "SET_K="
            setK(value);
            int size = snprintf(printfBuffer, BUFFER_SIZE, "SET_K=OK =%ld\r\n",value);
            uart_write(printfBuffer, size) ;
    }
    else if(strcmp(buffer, "GET_K") == 0){
        int16_t K= 1234 ; // getK() ;
        int size = snprintf(printfBuffer, BUFFER_SIZE, "K=%d%d.%d%d000\r\n",(K/1000)%10,(K/100)%10,(K/10)%10,K%10);
        uart_write(printfBuffer, size);
    }
    else if(strcmp(buffer, "GET_A") == 0){
        int16_t A= 90 ;//getAngle() ;
        int size = snprintf(printfBuffer, BUFFER_SIZE, "A=%d%d%d.%d000\r\n",(A/100)%10,(A/10)%10,(A)%10,(A*10)%10);
        uart_write(printfBuffer, size);
    }
    else {
        int size = snprintf(printfBuffer, BUFFER_SIZE, "\r\nCommande inconnue\r\n");
        uart_write(printfBuffer, size);
    }
}
}
```
## TP3 - Interface REST
Pour cette partie l'ensemble du code se trouve dans le fichier ```Python_PI``` avec :
```helloTP3.py``` qui affiche une page ```hello, world``` en format html : """insert images\rest1_html.png , images\curl1.png  cote a cote """ 

Le rôle du décodeur @app.route et de pouvoir spécifier la page à laquelle on
veut accéder. Le rôle du fragment <int:index> et de pouvoir récupérer une variable
passé dans l’URL.

pour avoir le format json on utilise :
```python
return json.dumps({"index":index,"val":welcome[index]}),{"content-Type" : "application/json"}
```
et 
```python
return jsonify({"index": index, "val": welcome[index]})
```
