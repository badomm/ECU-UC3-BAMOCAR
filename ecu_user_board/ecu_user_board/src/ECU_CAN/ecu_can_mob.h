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
extern can_msg_t msg_rx_dash_data;
extern can_mob_t mob_rx_dash_data;
extern can_msg_t msg_tx_dash;
extern can_mob_t mob_tx_dash;

extern can_msg_t msg_ecu_inverter_status;
extern can_mob_t mob_ecu_inverter_status;

extern can_msg_t msg_tx_bms;
extern can_mob_t mob_tx_bms;
extern can_msg_t msg_rx_bms;
extern can_mob_t mob_rx_bms;
extern can_msg_t msg_rx_bspd;
extern can_mob_t mob_rx_bspd;
extern can_msg_t msg_rx_ecu;
extern can_mob_t mob_rx_ecu;




#endif /* ECU_CAN_MOB_H_ */