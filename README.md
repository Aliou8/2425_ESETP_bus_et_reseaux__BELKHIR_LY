# TP Bus et Réseaux

> Réalisé par : Aliou LY et Bayazid BELKHIR

---

## Table des Matières
- [TP 1 - Bus I2C](#tp-1---bus-i2c)
  - [Objectif](#objectif)
  - [Interfaçage du BMP280](#interfaçage-du-bmp280)
  - [Calcul des Valeurs Compensées](#calcul-des-valeurs-compensées)
- [TP 2 - Interfaçage STM32 - Raspberry](#tp-2---interfaçage-stm32---raspberry)
  - [Configuration du Port Série](#configuration-du-port-série)
  - [Communication STM32 - Raspberry](#communication-stm32---raspberry)
- [TP 3 - Interface REST](#tp-3---interface-rest)
  - [Serveur Web et Réponses JSON](#serveur-web-et-réponses-json)
  - [API CRUD](#api-crud)
- [TP 4 - Bus CAN](#tp-4---bus-can)

---

## TP 1 - Bus I2C

### Objectif
L'objectif est d'interfacer un microcontrôleur STM32 avec des capteurs via le bus I2C. Le capteur utilisé est le **BMP280**, qui permet de mesurer la température et la pression.

### Interfaçage du BMP280

#### Adresses I²C possibles
Le capteur BMP280 peut utiliser deux adresses I²C en fonction de la connexion de la broche SDO :
- **0x76** : lorsque la broche SDO est connectée à GND.
- **0x77** : lorsque la broche SDO est connectée à VDDIO.

#### Registre et Valeur d'Identification
Pour identifier le BMP280, on utilise le registre suivant :
- **Registre** : `0xD0`
- **Valeur attendue** : `0x58`

#### Mode Normal du BMP280
Pour placer le capteur en mode de fonctionnement normal :
- **Registre** : `0xF4` (bits [1:0])
- **Valeur** : `0b11`

#### Registres d'Étalonnage
Les coefficients d'étalonnage nécessaires à la compensation des mesures sont stockés dans les registres de `0x88` à `0xA1`.

#### Registres de Température et Pression
Les mesures brutes de température et de pression sont lues depuis les registres suivants :
- **Température** : `0xFA`, `0xFB`, `0xFC` (20 bits)
- **Pression** : `0xF7`, `0xF8`, `0xF9` (20 bits)

### Calcul des Valeurs Compensées
Les valeurs brutes lues du BMP280 sont compensées grâce aux coefficients d'étalonnage. Les fonctions en C suivantes permettent de convertir ces valeurs en températures et pressions compensées :

##### Fonction de Température

```c
uint32_t BMP280_ConvertTemperature(uint32_t rawTemp, BMP280_CompenParameter_t * param) {
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
La communication entre le STM32 et la Raspberry Pi est gérée dans le fichier``` STM32_Raspberry.c```. Dans ce fichier, la fonction principale, PI_RUN(), fonctionne comme un shell. Elle assure la réception et le traitement des commandes reçues via le port série (UART).

La fonction ```PI_GetCommand() ``` traite les commandes reçues via UART, en vérifiant la commande, en exécutant les actions associées, puis en envoyant la réponse appropriée.
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

Les commandes disponibles pour la communication entre le STM32 et la Raspberry Pi sont :

```GET_T``` : Renvoie la température mesurée par le capteur BMP280, formatée en degrés Celsius.
```GET_P``` : Renvoie la pression mesurée par le capteur BMP280 en Pascals.
```SET_K=<valeur> ```: Met à jour une constante K avec la valeur fournie.
```GET_K``` : Renvoie la valeur de la constante K actuelle.
```GET_A ```: Renvoie l’angle mesuré.

## TP3 - Interface REST
Dans le fichier ```Python_PI/helloTP3.py```, un serveur web simple affiche une page avec le message ```"hello, world" ```en HTML. Pour spécifier la page d'accès, la fonction @app.route est utilisée, et le fragment <int:index> permet de récupérer une variable passée dans l'URL.
![Texte alternatif](/images/rest1_html.png)
![Texte alternatif](/images/curl1.png)

Pour renvoyer des réponses au format JSON, deux méthodes sont utilisées :
```python
return json.dumps({"index":index,"val":welcome[index]}),{"content-Type" : "application/json"}
```
et 
```python
return jsonify({"index": index, "val": welcome[index]})
```
![Texte alternatif](/images/rest2_JSON.png) 
En cas de requête vers une page inexistante, une erreur 404 est affichée via un template.

![Texte alternatif](/images/E404.png)
 
### API CRUD 
Le fichier Python_PI/methodesTP3.py contient les différentes méthodes CRUD pour manipuler les données. Ces fonctions sont organisées selon qu'elles nécessitent un index ou non.

## TP4-Bus CAN
La communication sur le bus CAN est gérée dans le fichier ```Moteur_Can```. Ce programme permet d'envoyer des données evia le protocole CAN pour commander un moteur pas a pas 


