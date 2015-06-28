/*
 * ecu_can_messages.c
 *
 * Created: 25.05.2014 16:37:29
 *  Author: oyvinaak
 */ 

#include <asf.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "INVERTER_defines.h"
#include "queue_handles.h"
#include "revolve_can_definitions.h"
#include "ecu_can.h"
#include "ecu_can_mob.h"
#include "ecu_can_messages.h"


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


//////////////////////////////////
//Dash messages
////////////////////////////////


void ecu_can_send_fast_data(uint16_t inverter_vdc, uint16_t ecu_error, uint16_t rpm, int16_t trq_cmd) {
	mob_ecu_fast_data.can_msg->data.u64	  = 0x0LL;
	mob_ecu_fast_data.can_msg->data.u16[0] = inverter_vdc;
	mob_ecu_fast_data.can_msg->data.u16[1] = ecu_error;
	mob_ecu_fast_data.can_msg->data.u16[2] = rpm;
	mob_ecu_fast_data.can_msg->data.s16[3] = trq_cmd;
	
	can_tx(CAN_BUS_0,
	mob_ecu_fast_data.handle,
	mob_ecu_fast_data.dlc,
	CAN_DATA_FRAME,
	mob_ecu_fast_data.can_msg);
}

void ecu_can_send_slow_data(uint16_t motor_temp, uint16_t inverter_temp, uint8_t max_trq) {
	mob_ecu_slow_data.can_msg->data.u64	 = 0x0LL;
	mob_ecu_slow_data.can_msg->data.u16[0] = motor_temp;
	mob_ecu_slow_data.can_msg->data.u16[1] = inverter_temp;
	mob_ecu_slow_data.can_msg->data.u8[4] = max_trq;
	
	can_tx(CAN_BUS_1,
	mob_ecu_slow_data.handle,
	mob_ecu_slow_data.dlc,
	CAN_DATA_FRAME,
	mob_ecu_slow_data.can_msg);
}


void ecu_can_send_tractive_system_active(void) {
	mob_tx_dash.can_msg->data.u64	 = 0x0LL;
	mob_tx_dash.can_msg->data.u16[0]  = 0x1;
	
	mob_tx_dash.can_msg->id = CANR_FCN_PRI_ID | CANR_GRP_ECU_ID | CANR_MODULE_ID0_ID;
	mob_tx_dash.dlc = 2;
	
	can_tx(CAN_BUS_0,
	mob_tx_dash.handle,
	mob_tx_dash.dlc,
	CAN_DATA_FRAME,
	mob_tx_dash.can_msg);
}

void ecu_can_send_play_rtds(void) {
	mob_tx_dash.can_msg->data.u64	 = 0x0LL;
	mob_tx_dash.can_msg->data.u16[0]  = 0x2;
	
	mob_tx_dash.can_msg->id = CANR_FCN_PRI_ID | CANR_GRP_ECU_ID | CANR_MODULE_ID0_ID;
	mob_tx_dash.dlc = 2;
	
	can_tx(CAN_BUS_0,
	mob_tx_dash.handle,
	mob_tx_dash.dlc,
	CAN_DATA_FRAME,
	mob_tx_dash.can_msg);
}

void ecu_can_send_ready_to_drive(void) {
	mob_tx_dash.can_msg->data.u64	 = 0x0LL;
	mob_tx_dash.can_msg->data.u16[0]  = 0x3;
	
	mob_tx_dash.can_msg->id = CANR_FCN_PRI_ID | CANR_GRP_ECU_ID | CANR_MODULE_ID0_ID;
	mob_tx_dash.dlc = 2;
	
	can_tx(CAN_BUS_0,
	mob_tx_dash.handle,
	mob_tx_dash.dlc,
	CAN_DATA_FRAME,
	mob_tx_dash.can_msg);
}

void ecu_can_send_drive_disabled(void) {
	mob_tx_dash.can_msg->data.u64	 = 0x0LL;
	mob_tx_dash.can_msg->data.u16[0]  = 0x4;
	
	mob_tx_dash.can_msg->id = CANR_FCN_PRI_ID | CANR_GRP_ECU_ID | CANR_MODULE_ID0_ID;
	mob_tx_dash.dlc = 2;
	
	can_tx(CAN_BUS_0,
	mob_tx_dash.handle,
	mob_tx_dash.dlc,
	CAN_DATA_FRAME,
	mob_tx_dash.can_msg);
}

void ecu_can_send_alive() {
	mob_tx_dash.can_msg->data.u8[0]  = ALIVE_ECU;	
	mob_tx_dash.can_msg->id = CANR_FCN_DATA_ID | CANR_GRP_DASH_ID | CANR_MODULE_ID7_ID;
	mob_tx_dash.dlc = CANR_ALIVE_MSG_DLC;
	can_tx(CAN_BUS_0, mob_tx_dash.handle, mob_tx_dash.dlc, CAN_DATA_FRAME,mob_tx_dash.can_msg);
}

void ecu_can_confirm_activate_launch(void) {
	mob_tx_dash.can_msg->data.u64	= 0x0LL;
	mob_tx_dash.can_msg->data.u8[0]	= 0x1;
	
	mob_tx_dash.can_msg->id = CANR_FCN_PRI_ID | CANR_GRP_ECU_ID | CANR_MODULE_ID1_ID;
	mob_tx_dash.dlc = 1;
	
	can_tx(CAN_BUS_0,
	mob_tx_dash.handle,
	mob_tx_dash.dlc,
	CAN_DATA_FRAME,
	mob_tx_dash.can_msg);
}

void ecu_can_send_launch_ready(void) {
	mob_tx_dash.can_msg->data.u64	= 0x0LL;
	mob_tx_dash.can_msg->data.u8[0]	= 0x2;
	
	mob_tx_dash.can_msg->id = CANR_FCN_PRI_ID | CANR_GRP_ECU_ID | CANR_MODULE_ID1_ID;
	mob_tx_dash.dlc = 1;
	
	can_tx(CAN_BUS_0,
	mob_tx_dash.handle,
	mob_tx_dash.dlc,
	CAN_DATA_FRAME,
	mob_tx_dash.can_msg);
}

void ecu_can_send_launch_stop(void) {
	mob_tx_dash.can_msg->data.u64	= 0x0LL;
	mob_tx_dash.can_msg->data.u8[0]	= 255;
	
	mob_tx_dash.can_msg->id = CANR_FCN_PRI_ID | CANR_GRP_ECU_ID | CANR_MODULE_ID1_ID;
	mob_tx_dash.dlc = 1;
	
	can_tx(CAN_BUS_0,
	mob_tx_dash.handle,
	mob_tx_dash.dlc,
	CAN_DATA_FRAME,
	mob_tx_dash.can_msg);
}

