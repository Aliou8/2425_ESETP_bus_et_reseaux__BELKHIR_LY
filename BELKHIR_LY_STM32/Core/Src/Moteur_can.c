/*
 * Moteur_can.c
 *
 *  Created on: Nov 6, 2024
 *      Author: aliou
 */

#include "Moteur_can.h"
#include "bmp280.h"
#include "can.h"

int32_t coefficientK=1 ;
int32_t Angle ;

void setK(int32_t k)
{
    coefficientK = k ;
}
int32_t getK(void)
{
    return coefficientK ;
}

int32_t getAngle(void)
{
    return Angle ;
}


static void CAN_SendMessage(CAN_TxHeaderTypeDef pHeader, uint8_t *data , uint32_t pTxMailbox ) {

    if (HAL_CAN_AddTxMessage(&hcan1, &pHeader, data, &pTxMailbox) != HAL_OK) {
        //printf("Erreur d'envoi CAN pour l'ID: 0x%lX\r\n", pHeader.StdId);
    } else {
       // printf("Message CAN envoyé, ID: 0x%lX, Data: 0x%X 0x%X\r\n", pHeader.StdId, data[0], data[1]);
    }
}

void manualMode( uint8_t rotation , uint8_t steps , uint8_t speed)
{
    CAN_TxHeaderTypeDef pHeader ;
    pHeader.IDE = CAN_ID_STD;
	pHeader.RTR = CAN_RTR_DATA;
    pHeader.StdId = ManualModeID;
    pHeader.DLC = 3;
    uint32_t pMailbox = 0 ;
    uint8_t data[] = {rotation , steps , speed};
    CAN_SendMessage(pHeader , data ,pMailbox ) ;

}

void setMotorAngle( uint8_t angle , uint8_t sign)
{
    CAN_TxHeaderTypeDef pHeader ;
    pHeader.IDE = CAN_ID_STD;
	pHeader.RTR = CAN_RTR_DATA;
    pHeader.StdId = AngleID;
    pHeader.DLC = 2;
    uint32_t pMailbox = 0 ;
    uint8_t data[] = {angle , sign} ;
    CAN_SendMessage(pHeader , data ,pMailbox ) ;
    if (sign==0x00)
    {
        Angle = angle ;
    }
    else
    {
        Angle = -angle ;
    }


}

void setPositionTo0(void)
{
    CAN_TxHeaderTypeDef pHeader ;
    pHeader.IDE = CAN_ID_STD;
	pHeader.RTR = CAN_RTR_DATA;
    pHeader.StdId = setTo0;
    pHeader.DLC = 0;
    int32_t pMailbox = 0;
    CAN_SendMessage(pHeader , NULL ,pMailbox ) ;

}

void moteurRun(void)

{
	int Temp0 = 25 ;
	uint8_t angle = ((Temp0-BMP280_ReadTemperature()/100)*coefficientK) ;
    setMotorAngle( angle , 0) ;
}




