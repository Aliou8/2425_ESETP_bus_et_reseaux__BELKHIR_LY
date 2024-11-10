#include "STM32_Raspberry.h"
#include <string.h>
#include "usart.h"

static char printfBuffer[BUFFER_SIZE];
static BMP280_CompenParameter_t paramPI ;

// Fonction pour lire un caractère via UART
static char uart_read() {

    char c;
    HAL_UART_Receive(&UART_PI, (uint8_t *)(&c), 1, HAL_MAX_DELAY);
    return c;
}

// Fonction pour écrire une chaîne via UART
static int uart_write(char *s, uint16_t size) {
    HAL_UART_Transmit(&UART_PI, (uint8_t *)s, size, HAL_MAX_DELAY);
    return size;
}

// Initialisation de la communication
void PI_Init(void) {
    int size = snprintf(printfBuffer, BUFFER_SIZE, "\r\nCommunication entre STM32 et Raspberry PI\r\n");
    uart_write(printfBuffer, size);
}

// Fonction pour traiter les commandes
static void PI_GetCommand(char *buffer) {

    uint32_t temperature =BMP280_ConvertTemperature(BMP280_ReadTemperature(), &paramPI);
    uint32_t pression = BMP280_ReadPressure();

    // Vérifiez la commande et construisez la réponse sans afficher la commande
    if (strcmp(buffer, "GET_T") == 0) {
        int size = snprintf(printfBuffer, BUFFER_SIZE, "La temperature est : %lu °c\r\n", temperature);
        uart_write(printfBuffer, size);
    } else if (strcmp(buffer, "GET_P") == 0) {
        int size = snprintf(printfBuffer, BUFFER_SIZE, "La pression est : %lu Pa\r\n", pression);
        uart_write(printfBuffer, size);
    } else {
        int size = snprintf(printfBuffer, BUFFER_SIZE, "\r\nCommande inconnue\r\n");
        uart_write(printfBuffer, size);
    }
}

// Fonction principale pour gérer les commandes
int PI_RUN(void) {
    int reading = 0;
    int pos = 0;
    static char cmd_buffer[BUFFER_SIZE];

    while (1) {
        uart_write("> ", 2);  // Afficher le prompt
        reading = 1;

        while (reading) {
            char c = uart_read();

            switch (c) {
                case '\r':   // Touche RETURN
                    uart_write("\n", 2);   // Sauter une ligne après Entrée
                    cmd_buffer[pos] = '\0';   // Terminer la chaîne
                    reading = 0;  // Quitter la boucle de lecture pour exécuter la commande
                    pos = 0;      // Réinitialiser la position du buffer
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

        // Exécuter la commande après avoir lu toute la ligne
        PI_GetCommand(cmd_buffer);
    }
    return 0;
}
