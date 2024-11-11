#ifndef __STM32_RASPBERRY__
#define __STM32_RASPBERRY__

#include <stdio.h>
#include "bmp280.h"
#include "Moteur_can.h"

#define UART_PI huart3
#define BUFFER_SIZE 100

void PI_Init(void);
int PI_RUN(void);

#endif /* __STM32_RASPBERRY__ */
