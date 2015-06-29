/*
 * ecu_can_mob.c
 *
 * Created: 25.05.2014 16:48:02
 *  Author: oyvinaak
 */ 


#include <asf.h>
#include "INVERTER_defines.h"
#include "ecu_can_mob.h"
#include "revolve_can_definitions.h"


////////////////
//RX mailboxes for dash data
///////////////
can_msg_t msg_rx_dash_data = {
	{
		{
			.id			= CANR_FCN_DATA_ID | CANR_GRP_DASH_ID,
			.id_mask	= 0x7F0,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_rx_dash_data = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_rx_dash_data,
	8,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

////////////////
//RX mailboxes for TPS data
///////////////
can_msg_t msg_rx_trq_sens0  = {
	{
		{
			.id			= CANR_FCN_PRI_ID | CANR_GRP_SENS_ROTARY_ID | CANR_MODULE_ID0_ID,
			.id_mask	= 0x7F8,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_rx_trq_sens0 = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_rx_trq_sens0,
	3,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

can_msg_t msg_rx_trq_sens1  = {
	{
		{
			.id			= CANR_FCN_PRI_ID | CANR_GRP_SENS_ROTARY_ID | CANR_MODULE_ID1_ID,
			.id_mask	= 0x7F8,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_rx_trq_sens1 = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_rx_trq_sens1,
	3,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};




can_msg_t msg_ecu_temp_and_maxTrq  = {
	{
		{
			.id			= CANR_FCN_DATA_ID | CANR_GRP_ECU_ID | CANR_MODULE_ID1_ID,
			.id_mask	= 0x7F8,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_ecu_temp_and_maxTrq  = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_ecu_temp_and_maxTrq,
	5,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

can_msg_t msg_ecu_inverter_status  = {
	{
		{
			.id			= CANR_FCN_DATA_ID | CANR_GRP_ECU_ID | CANR_MODULE_ID0_ID,
			.id_mask	= 0x7F8,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_ecu_inverter_status = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_ecu_inverter_status,
	8,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

can_msg_t msg_tx_dash  = {
	{
		{
			.id			= CANR_FCN_CMD_ID | CANR_GRP_ECU_ID | CANR_MODULE_ID0_ID,
			.id_mask	= 0x7FF,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_tx_dash = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_tx_dash,
	1,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

can_msg_t msg_tx_bms  = {
	// Addr 633 by default
	{
		{
			.id			= CANR_FCN_CMD_ID | 0x27 | CANR_MODULE_ID4_ID,
			.id_mask	= 0x7F8,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_tx_bms  = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_tx_bms,
	8,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

can_msg_t msg_rx_bms  = {
	{
		{
			.id			= CANR_FCN_CMD_ID | CANR_GRP_BMS_ID,
			.id_mask	= 0xFF0,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_rx_bms  = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_rx_bms,
	8,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};


can_msg_t msg_rx_bspd  = {
	{
		{
			.id			= CANR_FCN_DATA_ID | CANR_GRP_SENS_BSPD_ID | CANR_MODULE_ID0_ID,
			.id_mask	= 0x7FF,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_rx_bspd  = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_rx_bspd,
	1,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

can_msg_t msg_brk  = {
	{
		{
			.id			= CANR_FCN_DATA_ID | CANR_GRP_SENS_BRK_ID | CANR_MODULE_ID0_ID,
			.id_mask	= 0x7FE,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_brk  = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_brk,
	8,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

