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


can_mob_t mob_tx_dash = {
	CAN_MOB_NOT_ALLOCATED,
	NULL,
	1,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

can_msg_t msg_tx_voltage  = {
	{
		{
			.id			= 0x638,
			.id_mask	= 0x7FF,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_tx_voltage = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_tx_voltage,
	1,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

can_msg_t msg_tx_rpm  = {
	{
		{
			.id			= 0x63A,
			.id_mask	= 0x7FF,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_tx_rpm = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_tx_rpm,
	1,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

can_msg_t msg_rx_ecu = {
	{
		{
			.id			= 0x230,
			.id_mask	= 0x7FF,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_rx_ecu = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_rx_ecu,
	1,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

can_msg_t msg_torque_request_ecu = {
	{
		{
			.id			= 0x033,
			.id_mask	= 0x7FF,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_torque_request_ecu = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_torque_request_ecu,
	1,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};