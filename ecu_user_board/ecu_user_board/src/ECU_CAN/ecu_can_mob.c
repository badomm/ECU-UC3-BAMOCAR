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

can_msg_t msg_rxWheelSpeed = {
	{
		{
			.id			= 0x620,
			.id_mask	= 0x7FC,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_rxWheelSpeed = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_rxWheelSpeed,
	1,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

can_msg_t msg_rx_bmsTemp = {
	{
		{
			.id			= 0x62B,
			.id_mask	= 0x7FF,
		},
	},
	.data.u64 = 0x0LL,
};
can_mob_t mob_rx_bmsTemp = {
	CAN_MOB_NOT_ALLOCATED,
	&msg_rx_bmsTemp,
	1,
	CAN_DATA_FRAME,
	CAN_STATUS_NOT_COMPLETED,
};

