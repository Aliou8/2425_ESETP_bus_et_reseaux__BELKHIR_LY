/*
 * Moteur_can.h
 *
 *  Created on: Nov 6, 2024
 *      Author: aliou
 */

#ifndef INC_MOTEUR_CAN_H_
#define INC_MOTEUR_CAN_H_

#include <stdint.h>

#define ManualModeID 0X60
#define AngleID 0x61
#define setTo0 0x62
#define AntiClockwise 0x01
#define Clockwise 0x01
#define Signepositive = 0x00
#define Signegative = 0x01

void setK(int32_t k) ;
int32_t getK(void) ;
int32_t getAngle(void) ;
void manualMode( uint8_t rotation , uint8_t steps , uint8_t speed) ;
void setMotorAngle( uint8_t angle , uint8_t sign) ;
void setPositionTo0(void) ;
void moteurRun(void) ;

#endif /* INC_MOTEUR_CAN_H_ */
