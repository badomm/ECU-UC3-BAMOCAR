/*
 * ecu_can_messages.c
 *
 * Created: 25.05.2014 16:37:29
 *  Author: oyvinaak
 */ 

#include <asf.h>
#include <can.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "INVERTER_defines.h"
#include "queue_handles.h"
#include "revolve_can_definitions.h"
#include "ecu_can.h"
#include "ecu_can_mob.h"
#include "ecu_can_messages.h"
#include "endianSwapper.h"

////////////////////////////
// Inverter Messages
///////////////////////////
void ecu_can_send_to_inverter(uint8_t inverter_reg, uint16_t data) {
	inverter_can_msg_t message;
	
	message.data.u64 = 0x0LL;
	message.dlc = INVERTER_DLC_3;
	message.data.u32[0] = inverter_reg << 24 | data << 8;
	xQueueSendToBack(queue_to_inverter,&message,0);
}

void ecu_can_inverter_enable_drive() {
	ecu_can_send_to_inverter(MODE_REG, 0x0000);
}

void ecu_can_inverter_disable_drive() {
	ecu_can_send_to_inverter(MODE_REG, 0x0400);
}

void ecu_can_inverter_torque_cmd(int16_t torque) {
	/* This code also handles negative numbers */
	uint16_t torque_intel = ((torque >> 8) & 0xff) | ((torque & 0xff) << 8);
	
	inverter_can_msg_t message;
	
	message.data.u64 = 0x0LL;
	message.dlc = INVERTER_DLC_3;
	message.data.u32[0] = TORQUE_CMD << 24 | torque_intel << 8;
	
	xQueueSendToBack(queue_to_inverter,&message,0);
}

void ecu_can_inverter_read_torque_periodic() {
	ecu_can_send_to_inverter(READ_CMD, 0x90FA); //FA = 250 ms period
}

void ecu_can_inverter_read_reg(uint8_t inverter_reg) {
	/* Msg = 0x3D inverter_reg 00, ex: 0x3DE800 (read FRG_RUN) */
	
	inverter_can_msg_t message;
	
	message.data.u64 = 0x0LL;
	message.data.u32[0] = READ_CMD << 24 | inverter_reg << 16;
	message.dlc = INVERTER_DLC_3;
	
	xQueueSendToBack(queue_to_inverter,&message,0);
}


