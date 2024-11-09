#ifndef __STM32_RASPBERRY__
#define __STM32_RASPBERRY__

#include <stdio.h>
#include "bmp280.h"

#define UART_PI huart2
#define BUFFER_SIZE 100

void PI_Init(void) ;
int PI_RUN(void) ;

#endif
