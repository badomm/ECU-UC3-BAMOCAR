/*
 * ecu_can_mob.h
 *
 * Created: 25.05.2014 16:47:45
 *  Author: oyvinaak
 */ 


#ifndef ECU_CAN_MOB_H_
#define ECU_CAN_MOB_H_




////////////////
//RX mailboxes for dash data
///////////////

extern can_msg_t msg_tx_dash;
extern can_mob_t mob_tx_dash;

extern can_msg_t msg_rx_bms;
extern can_mob_t mob_rx_bms;

extern can_msg_t msg_rx_ecu;
extern can_mob_t mob_rx_ecu;

extern can_mob_t mob_torque_request_ecu;

extern can_msg_t msg_tx_voltage;
extern can_mob_t mob_tx_voltage;

extern can_msg_t msg_tx_rpm;
extern can_mob_t mob_tx_rpm;

#endif /* ECU_CAN_MOB_H_ */