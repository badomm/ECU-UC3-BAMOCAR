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
	mob_rx_dash_data.handle			= 1;
	mob_rx_bms.handle	= 2;
	mob_ecu_inverter_status.handle	= 3;
	mob_rx_bspd.handle			= 4;
	mob_rx_ecu.handle = 5;


	/* Initialize CAN channels */
	can_init(CAN_BUS_0, ((uint32_t)&mob_ram_ch0[0]), CANIF_CHANNEL_MODE_NORMAL,	can_out_callback_channel0);
	can_init(CAN_BUS_1, ((uint32_t)&mob_ram_ch1[0]), CANIF_CHANNEL_MODE_NORMAL,	can_out_callback_channel1);
	
	
	/* Prepare for message reception */
	setupRxmailbox(CAN_BUS_0, mob_rx_dash_data);
	setupRxmailbox(CAN_BUS_1, mob_rx_bms);
	setupRxmailbox(CAN_BUS_0, mob_rx_bspd);
	setupRxmailbox(CAN_BUS_1, mob_rx_ecu);
	asm("nop");
}

void can_out_callback_channel0(U8 handle, U8 event){
	if (handle == mob_rx_dash_data.handle) {
		gpio_toggle_pin(LED1);
		mob_rx_dash_data.can_msg->data.u64	= can_get_mob_data(CAN_BUS_0, handle).u64;
		mob_rx_dash_data.can_msg->id			= can_get_mob_id(CAN_BUS_0, handle);
		mob_rx_dash_data.dlc					= can_get_mob_dlc(CAN_BUS_0, handle);
		mob_rx_dash_data.status				= event;
		
		car_can_msg_t dash_can_msg;
		
		dash_can_msg.data.u64 = mob_rx_dash_data.can_msg->data.u64;
		dash_can_msg.id = mob_rx_dash_data.can_msg->id;
		xQueueSendToBackFromISR(queue_dash_msg, &dash_can_msg, NULL);
		/* Empty message field */
		mob_rx_dash_data.can_msg->data.u64 = 0x0LL;
		
		/* Prepare message reception */
		can_rx(CAN_BUS_0, mob_rx_dash_data.handle, mob_rx_dash_data.req_type, mob_rx_dash_data.can_msg);
		
	} else if (handle == mob_rx_bspd.handle) {
		mob_rx_bspd.can_msg->data.u64	= can_get_mob_data(CAN_BUS_0, handle).u64;
		mob_rx_bspd.can_msg->id			= can_get_mob_id(CAN_BUS_0, handle);
		mob_rx_bspd.dlc					= can_get_mob_dlc(CAN_BUS_0, handle);
		mob_rx_bspd.status				= event;
		
		xQueueOverwriteFromISR( queue_bspd, &mob_rx_bspd.can_msg->data.u8[0], NULL );
		/* Empty message field */
		mob_rx_bspd.can_msg->data.u64 = 0x0LL;
		/* Prepare message reception */
		can_rx(CAN_BUS_0, mob_rx_bspd.handle, mob_rx_bspd.req_type, mob_rx_bspd.can_msg);
	} else if (handle == mob_rx_ecu.handle){gpio_toggle_pin(LED1); can_rx(CAN_BUS_0, mob_rx_ecu.handle, mob_rx_ecu.req_type, mob_rx_ecu.can_msg);}
}

/* Call Back called by can_drv, channel 1 */
void can_out_callback_channel1(U8 handle, U8 event){
	can_mob_t *can_mob = NULL;
	car_can_msg_t can_msg = {.data.u64 = can_get_mob_data(CAN_BUS_1, handle).u64,
							 .id = can_get_mob_id(CAN_BUS_1, handle)
							};
	/*Check which handle the message is*/
	
	//BMS
	if(handle == mob_rx_bms.handle){
			can_mob = &mob_rx_bms;
			xQueueSendToBackFromISR(queue_bms_rx, &can_msg, NULL);
	}
	//ECU
	else if(handle == mob_rx_ecu.handle){
			gpio_toggle_pin(LED1);
			can_mob = &mob_rx_ecu;
			xQueueSendToBackFromISR(queue_ecu_rx, &can_msg, NULL);
	}		
	
	/*Reset mailbox and prepeare for reception*/
	if(can_mob != NULL){
			can_mob->can_msg->data.u64 = 0x0LL; /* Empty message field */
			setupRxmailbox(CAN_BUS_1, *can_mob); /* Prepare message reception */
	}
}

