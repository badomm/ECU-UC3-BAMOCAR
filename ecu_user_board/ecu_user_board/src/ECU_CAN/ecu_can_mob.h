/*
 * ecu_can_mob.h
 *
 * Created: 25.05.2014 16:47:45
 *  Author: oyvinaak
 */ 


#ifndef ECU_CAN_MOB_H_
#define ECU_CAN_MOB_H_

extern can_msg_t msg_rx_speed_sens_fl;
extern can_mob_t mob_rx_speed_sens_fl;
extern can_msg_t msg_rx_speed_sens_fr;
extern can_mob_t mob_rx_speed_sens_fr;
extern can_msg_t msg_rx_speed_sens_rl;
extern can_mob_t mob_rx_speed_sens_rl;
extern can_msg_t msg_rx_speed_sens_rr;
extern can_mob_t mob_rx_speed_sens_rr;
extern can_msg_t msg_rx_dash_data;
extern can_mob_t mob_rx_dash_data;
extern can_msg_t msg_rx_dash_pri;
extern can_mob_t mob_rx_dash_pri;
extern can_msg_t msg_rx_trq_sens0;
extern can_mob_t mob_rx_trq_sens0;
extern can_msg_t msg_rx_trq_sens1;
extern can_mob_t mob_rx_trq_sens1;
extern can_msg_t msg_ecu_temp_and_maxTrq;
extern can_mob_t mob_ecu_temp_and_maxTrq;
extern can_msg_t msg_ecu_inverter_status;
extern can_mob_t mob_ecu_inverter_status;
extern can_msg_t msg_tx_dash;
extern can_mob_t mob_tx_dash;
extern can_msg_t msg_tx_bms;
extern can_mob_t mob_tx_bms;
extern can_msg_t msg_rx_bms_precharge;
extern can_mob_t mob_rx_bms_precharge;
extern can_msg_t msg_rx_bms_battvolt;
extern can_mob_t mob_rx_bms_battvolt;
extern can_msg_t msg_rx_bspd;
extern can_mob_t mob_rx_bspd;
extern can_msg_t msg_brk;
extern can_mob_t mob_brk;





#endif /* ECU_CAN_MOB_H_ */