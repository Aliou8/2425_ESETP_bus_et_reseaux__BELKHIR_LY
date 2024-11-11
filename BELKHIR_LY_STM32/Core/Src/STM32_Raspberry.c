#include "STM32_Raspberry.h"
#include <string.h>
#include "usart.h"
#include "bmp280.h"

static char printfBuffer[BUFFER_SIZE];
static char uartRxBuffer[1];  // Buffer pour stocker le caractère reçu
uint8_t prompt[] = "STM32@pig3>" ;
uint8_t start[] = "\r\n Communication entre STM32 et Raspberry PI\r\n " ;
static int uartRxReceived = 0;  // Flag pour indiquer la réception d'un caractère

// Fonction pour écrire une chaîne via UART
static int uart_write(char *s, uint16_t size)
{
    HAL_UART_Transmit(&UART_PI, (uint8_t *)s, size, HAL_MAX_DELAY);
    return size;
}

// Initialisation de la communication
void PI_Init(void)
 {
	HAL_UART_Receive_IT(&UART_PI, (uint8_t *)uartRxBuffer, 1);
	HAL_UART_Transmit(&UART_PI, start, strlen((char *)start), HAL_MAX_DELAY);
	HAL_UART_Transmit(&UART_PI, prompt, strlen((char *)prompt), HAL_MAX_DELAY);
}

// Fonction pour traiter les commandes reçues via UART
static void PI_GetCommand(char *buffer)
{
    // Vérifiez la commande et construisez la réponse sans afficher la commande
    if (strcmp(buffer, "GET_T") == 0) {
       // int32_t tempNc = 51000 ; //BMP280_ReadTemperature() ;
        int32_t temp = 1250 ; //BMP280_ConvertTemperature(tempNc) ;
        int size = snprintf(printfBuffer, BUFFER_SIZE, "T=+%ld%ld.%ld%ld_C\r\n",(temp/1000)%10,(temp/100)%10,(temp/10)%10,temp%10);
        uart_write(printfBuffer, size);
    } else if (strcmp(buffer, "GET_P") == 0) {
       // int32_t PresNc =5800000 ; // BMP280_ReadPressure() ;
        int32_t Pres = 58000 ; //BMP280_ConvertPressure(PresNc) ;
        int size = snprintf(printfBuffer, BUFFER_SIZE, "P=%ldPa\r\n",Pres);
        uart_write(printfBuffer, size);
    }
    else if(strcmp(buffer, "SET_K=1234") == 0){
        //setK(100) ;
        int size = snprintf(printfBuffer, BUFFER_SIZE, "SET_K=OK \r\n");
        uart_write(printfBuffer, size);
    }
    else if(strcmp(buffer, "GET_K") == 0){
        int16_t K= 1234 ; // getK() ;
        int size = snprintf(printfBuffer, BUFFER_SIZE, "K=%d%d.%d%d000\r\n",(K/1000)%10,(K/100)%10,(K/10)%10,K%10);
        uart_write(printfBuffer, size);
    }
    else if(strcmp(buffer, "GET_A") == 0){
        int16_t A= 90 ;//getAngle() ;
        int size = snprintf(printfBuffer, BUFFER_SIZE, "A=%d%d%d.0000\r\n",(A/100)%10,(A/10)%10,(A)%10,A%10);
        uart_write(printfBuffer, size);
    }
    else {
        int size = snprintf(printfBuffer, BUFFER_SIZE, "\r\nCommande inconnue\r\n");
        uart_write(printfBuffer, size);
    }
}

// Fonction principale pour gérer les commandes envoyées via UART
int PI_RUN(void)
{
    static char cmd_buffer[BUFFER_SIZE];
    static int pos = 0;
    char c;

    // Si un caractère est reçu via UART (vérifié par l'interruption)
    if (uartRxReceived) {
        c = uartRxBuffer[0];  // Lire le caractère reçu via UART
        uartRxReceived = 0;    // Réinitialiser le flag

        // Traitement des caractères reçus
        switch (c) {
            case '\r':   // Touche RETURN
                uart_write("\n", 2);   // Sauter une ligne après Entrée
                cmd_buffer[pos] = '\0';   // Terminer la chaîne
                PI_GetCommand(cmd_buffer);  // Exécuter la commande
                pos = 0;      // Réinitialiser la position du buffer
                uart_write("STM32@pig3>", 12);
                break;

            case 127:    // Touche Backspace
                if (pos > 0) {
                    pos--;     // Supprimer le dernier caractère
                    uart_write("\b \b", 3); // Effacer le caractère sur le terminal
                }
                break;

            default:    // Autres caractères
                if (pos < BUFFER_SIZE - 1) {  // Vérifier si le buffer a de l'espace
                    uart_write(&c, 1);        // Afficher le caractère
                    cmd_buffer[pos++] = c;    // Stocker le caractère
                }
                break;
        }

    }

    return 0;
}

// Callback pour la réception d'un caractère via UART (géré par interruption)
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == UART_PI.Instance) {
        uartRxReceived = 1;  // Indiquer que des données ont été reçues
        HAL_UART_Receive_IT(&UART_PI, (uint8_t *)uartRxBuffer, 1);  // Redemander la réception du prochain caractère
    }
}
