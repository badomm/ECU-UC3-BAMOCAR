/*
 * ecu_can.c
 *
 * Created: 20.02.2014 13:00:52
 *  Author: oyvinaak
 */ 

#include <asf.h>
#include "ecu_can.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "INVERTER_defines.h"
#include "queue_handles.h"
#include "ecu_can_mob.h"
#include "endianSwapper.h"
/* Note on reception using .u64
 * Format 0xDATADATA		DATADATA
 *          u32[0]			u32[1]
 *			u16[0]u16[1]	u16[2]u16[3]
 */


static xQueueHandle queueCanSend_0;
static xQueueHandle queueCanSend_1;


static can_mob_t mob_tx_can0 = {
	CAN_MOB_NOT_ALLOCATED,
	NULL,
	1,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};
static can_mob_t mob_tx_can1 = {
	CAN_MOB_NOT_ALLOCATED,
	NULL,
	1,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

/* Prototypes */
void can_out_callback_channel0(U8 handle, U8 event);
void can_out_callback_channel1(U8 handle, U8 event);

/* Allocate CAN mobs */
volatile can_msg_t mob_ram_ch0[NB_MOB_CHANNEL] __attribute__ ((section (".hsb_ram_loc")));
volatile can_msg_t mob_ram_ch1[NB_MOB_CHANNEL] __attribute__ ((section (".hsb_ram_loc")));

void setupRxmailbox(U8 CAN, can_mob_t mob){
		can_rx(CAN, mob.handle, mob.req_type, mob.can_msg);
}

void ecu_can_init(void) {
	/* Setup the generic clock for CAN output */
	scif_gc_setup(
		AVR32_SCIF_GCLK_CANIF,
		SCIF_GCCTRL_OSC0,
		AVR32_SCIF_GC_NO_DIV_CLOCK,
		0
	);
	/* Now enable the generic clock input for the CAN module */
	scif_gc_enable(AVR32_SCIF_GCLK_CANIF);

	static const gpio_map_t CAN_GPIO_MAP = {
		{CAN0_RX_PIN, CAN0_RX_FUNCTION},
		{CAN0_TX_PIN, CAN0_TX_FUNCTION},
		{CAN1_RX_PIN, CAN1_RX_FUNCTION},
		{CAN1_TX_PIN, CAN1_TX_FUNCTION}
	};
	
	/* Assign GPIO to CAN. */
	gpio_enable_module(CAN_GPIO_MAP, sizeof(CAN_GPIO_MAP) / sizeof(CAN_GPIO_MAP[0]));
	

	/* Initialize interrupt vectors. */
	INTC_init_interrupts();
	
	/* Allocate channel message box */
	
	mob_tx_can0.handle = 5;
	mob_tx_can1.handle = 6;
	mob_rx_ecu.handle = 2;
	mob_torque_request_ecu.handle = 7;
	mob_rxWheelSpeed.handle = 0;
	mob_rx_bmsTemp.handle = 1;


	/* Initialize CAN channels */
	can_init(CAN_BUS_0, ((uint32_t)&mob_ram_ch0[0]), CANIF_CHANNEL_MODE_NORMAL,	can_out_callback_channel0);
	can_init(CAN_BUS_1, ((uint32_t)&mob_ram_ch1[0]), CANIF_CHANNEL_MODE_NORMAL,	can_out_callback_channel1);
	
	
	/* Prepare for message reception */
	setupRxmailbox(CAN_BUS_1, mob_torque_request_ecu);
	setupRxmailbox(CAN_BUS_1, mob_rx_ecu);
	setupRxmailbox(CAN_BUS_0, mob_rxWheelSpeed);
	setupRxmailbox(CAN_BUS_0, mob_rx_bmsTemp);
	
	/* Prepeare Can send queue*/
	queueCanSend_0 = xQueueCreate(20, sizeof(car_can_msg_t));
	queueCanSend_1 = xQueueCreate(20, sizeof(car_can_msg_t));
	asm("nop");
}

void can_out_callback_channel0(U8 handle, U8 event){
	
	can_mob_t *can_mob = NULL;
	car_can_msg_t can_msg = {
							 .data.u64 = can_get_mob_data(CAN_BUS_0, handle).u64,
							 .id = can_get_mob_id(CAN_BUS_0, handle),
							 .dlc = can_get_mob_dlc(CAN_BUS_0, handle)
							};
	/**Handle message****/
	
	//wheelspeed
	if (handle == mob_rxWheelSpeed.handle) {
		can_mob = &mob_rxWheelSpeed;
		xQueueSendFromISR(queue_wheelSpeed, &can_msg, NULL );
	}
	//BMS temp
	if (handle == mob_rx_bmsTemp.handle) {
		can_mob = &mob_rx_bmsTemp;
		xQueueOverwriteFromISR(queue_bms_rx, &can_msg, NULL );
	}
	
	/*Reset mailbox and prepeare for reception*/
	if(can_mob != NULL){
		can_mob->can_msg->data.u64 = 0x0LL; /* Empty message field */
		setupRxmailbox(CAN_BUS_0, *can_mob); /* Prepare message reception */
	}
}

/* Call Back called by can_drv, channel 1 */
void can_out_callback_channel1(U8 handle, U8 event){
	can_mob_t *can_mob = NULL;
	car_can_msg_t can_msg = {
							 .data.u64 = can_get_mob_data(CAN_BUS_1, handle).u64,
							 .id = can_get_mob_id(CAN_BUS_1, handle),
							 .dlc = can_get_mob_dlc(CAN_BUS_1, handle)
							};
	/*Check which handle the message is*/

	//Torque Request from ECU
	if (handle == mob_torque_request_ecu.handle) {
		can_mob = &mob_torque_request_ecu;
		Union32 swappedData;
		swappedData.u32 = endianSwapperU32(can_msg.data.u32[0]);
		xQueueOverwriteFromISR(queue_torque_request_ecu, &swappedData.f, NULL );
	}
	//ECU: drive enable and disable
	else if (handle == mob_rx_ecu.handle) {
		can_mob = &mob_rx_ecu;
		xQueueOverwriteFromISR(queue_ecu_rx, &can_msg, NULL );
	}
	
	/*Reset mailbox and prepare for reception*/
	if(can_mob != NULL){
			can_mob->can_msg->data.u64 = 0x0LL; /* Empty message field */
			setupRxmailbox(CAN_BUS_1, *can_mob); /* Prepare message reception */
	}
}


bool ecu_can_send(U8 CAN, uint32_t id, U8 dlc, U8* data, portTickType tickToWait){
	car_can_msg_t can_msg;
	can_msg.id = id;
	can_msg.dlc = dlc <8 ? dlc:8;
	for(int i = 0; i <dlc; i++){
		can_msg.data.u8[i] = data[i];
	}
	
	if (CAN_BUS_0 == CAN)
	{
		return xQueueSend(queueCanSend_0, &can_msg, tickToWait);
	}else if(CAN_BUS_1 == CAN){
		return xQueueSend(queueCanSend_1, &can_msg, tickToWait);
	}
}


static void task_can_send(U8 CAN, can_mob_t *mob){
	car_can_msg_t message;
	can_msg_t msg;
	bool gotMessage = false;
	while(1){
		gotMessage = false;
		if(CAN_BUS_0 == CAN){  
			gotMessage = xQueueReceive(queueCanSend_0, &message, portMAX_DELAY);
		}
		else if(CAN_BUS_0 == CAN){
			gotMessage = xQueueReceive(queueCanSend_1, &message, portMAX_DELAY);
		}
		
		if (gotMessage)
		{
			msg.data.u64 = message.data.u64;
			msg.id = message.id;
			while(can_tx(CAN, mob->handle, message.dlc, CAN_DATA_FRAME, &msg) != CAN_CMD_ACCEPTED);
		}
		vTaskDelay(1);
	}
}

void taskCan0Send(){
	while(1){
		task_can_send(CAN_BUS_0, &mob_tx_can0);

	}
}
void taskCan1Send(){
	while(1){
		task_can_send(CAN_BUS_1, &mob_tx_can1);
	}
}

