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
/* Note on reception using .u64
 * Format 0xDATADATA		DATADATA
 *          u32[0]			u32[1]
 *			u16[0]u16[1]	u16[2]u16[3]
 */


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
	mob_tx_dash.handle = 1;
	mob_rx_ecu.handle = 2;
	mob_torque_request_ecu.handle = 7;


	/* Initialize CAN channels */
	can_init(CAN_BUS_0, ((uint32_t)&mob_ram_ch0[0]), CANIF_CHANNEL_MODE_NORMAL,	can_out_callback_channel0);
	can_init(CAN_BUS_1, ((uint32_t)&mob_ram_ch1[0]), CANIF_CHANNEL_MODE_NORMAL,	can_out_callback_channel1);
	
	
	/* Prepare for message reception */
	setupRxmailbox(CAN_BUS_1, mob_torque_request_ecu);
	setupRxmailbox(CAN_BUS_1, mob_rx_ecu);
	asm("nop");
}

void can_out_callback_channel0(U8 handle, U8 event){
	
	can_mob_t *can_mob = NULL;
	car_can_msg_t can_msg = {.data.u64 = can_get_mob_data(CAN_BUS_0, handle).u64,
							 .id = can_get_mob_id(CAN_BUS_0, handle)
							};
	
	/*Reset mailbox and prepeare for reception*/
	if(can_mob != NULL){
		can_mob->can_msg->data.u64 = 0x0LL; /* Empty message field */
		setupRxmailbox(CAN_BUS_0, *can_mob); /* Prepare message reception */
	}
}

/* Call Back called by can_drv, channel 1 */
void can_out_callback_channel1(U8 handle, U8 event){
	can_mob_t *can_mob = NULL;
	car_can_msg_t can_msg = {.data.u64 = can_get_mob_data(CAN_BUS_1, handle).u64,
							 .id = can_get_mob_id(CAN_BUS_1, handle)
							};
	/*Check which handle the message is*/
	
	//Torque Request from ECU
	if (handle == mob_torque_request_ecu.handle) {
		can_mob = &mob_torque_request_ecu;
		xQueueOverwriteFromISR(torque_request_ecu, &mob_torque_request_ecu.can_msg->data.f[0], NULL );
	}
	else if (handle == mob_rx_ecu.handle) {
		can_mob = &mob_rx_ecu;
		xQueueOverwriteFromISR(queue_ecu_rx, &mob_rx_ecu.can_msg, NULL );
	}
	
	/*Reset mailbox and prepare for reception*/
	if(can_mob != NULL){
			can_mob->can_msg->data.u64 = 0x0LL; /* Empty message field */
			setupRxmailbox(CAN_BUS_1, *can_mob); /* Prepare message reception */
	}
}

