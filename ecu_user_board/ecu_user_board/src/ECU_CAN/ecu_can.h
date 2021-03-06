/*
 * ecu_can.h
 *
 * Created: 20.02.2014 13:01:15
 *  Author: oyvinaak
 */ 


#ifndef ECU_CAN_H_
#define ECU_CAN_H_

#include "FreeRTOS.h"
#include "revolve_can_definitions.h"
#include "ecu_can_messages.h"
#include "can.h"

#define CAN_BUS_0	0
#define CAN_BUS_1	1

#define CANR_DASH_ID_TX	CANR_FCN_CMD_ID  | CANR_GRP_DASH_ID | CANR_MODULE_ID0_ID
#define DASH_MSG0		CANR_FCN_CMD_ID | CANR_GRP_DASH_ID | CANR_MODULE_ID0_ID
#define DASH_MSG1		CANR_FCN_CMD_ID | CANR_GRP_DASH_ID | CANR_MODULE_ID1_ID
#define DASH_MSG2		CANR_FCN_CMD_ID | CANR_GRP_DASH_ID | CANR_MODULE_ID2_ID

typedef struct inverter_can_msg{
	Union64 data;
	uint32_t dlc;
}inverter_can_msg_t;

typedef struct car_can_msg{
	Union64 data;
	uint32_t id;
	U8 dlc;
}car_can_msg_t;


void ecu_can_init(void);
bool ecu_can_send(U8 CAN, uint32_t id, U8 dlc, U8* data, portTickType tickToWait);
void taskCan0Send();
void taskCan1Send();
#endif /* ECU_CAN_H_ */