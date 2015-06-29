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


//////////////////////////////////
//Dash messages
////////////////////////////////


void can_send(U8 CAN, can_mob_t mob){
	can_tx(CAN, mob.handle, mob.dlc, CAN_DATA_FRAME,mob.can_msg);
}

void ecu_can_send_alive() {
	mob_tx_dash.can_msg->data.u8[0]  = ALIVE_INVERTER;	
	mob_tx_dash.can_msg->id = CANR_FCN_DATA_ID | CANR_GRP_DASH_ID | CANR_MODULE_ID7_ID;
	mob_tx_dash.dlc = CANR_ALIVE_MSG_DLC;
	can_send(CAN_BUS_0, mob_tx_dash);
}

void ecu_can_send_voltage(float voltage) {
	Union32 voltageBigEndian;
	Union32 voltagelittleEndian;
	voltageBigEndian.f = voltage;
	voltagelittleEndian.u32 = endianSwapperU32(voltageBigEndian.u32);
	
	mob_tx_voltage.can_msg->data.u32[0]  = voltagelittleEndian.u32;
	mob_tx_voltage.dlc = 4;
	mob_tx_voltage.can_msg->id = 0x638;
	can_send(CAN_BUS_0, mob_tx_voltage);
}

void ecu_can_send_rpm(float rpm) {
	Union32 rpmBigEndian;
	Union32 rpmlittleEndian;
	rpmBigEndian.f = rpm;
	rpmlittleEndian.u32 = endianSwapperU32(rpmBigEndian.u32);
	
	mob_tx_rpm.can_msg->data.u32[0]  = rpmlittleEndian.u32;
	mob_tx_rpm.can_msg->id = 0x63A;
	mob_tx_rpm.dlc = 4;
	can_send(CAN_BUS_0, mob_tx_rpm);
}
