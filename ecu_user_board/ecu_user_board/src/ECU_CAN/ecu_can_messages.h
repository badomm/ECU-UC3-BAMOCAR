/*
 * ecu_can_messages.h
 *
 * Created: 25.05.2014 16:37:44
 *  Author: oyvinaak
 */ 


#ifndef ECU_CAN_MESSAGES_H_
#define ECU_CAN_MESSAGES_H_
////////////////////////////
// Inverter Messages
///////////////////////////
void ecu_can_send_to_inverter(uint8_t inverter_reg, uint16_t data);
void ecu_can_inverter_torque_cmd(int16_t torque);
void ecu_can_inverter_read_reg(uint8_t inverter_reg);
void ecu_can_inverter_enable_drive(void);
void ecu_can_inverter_disable_drive(void);
void ecu_can_inverter_read_torque_periodic(void);

//////////////////////////////////
//Dash messages
////////////////////////////////

void ecu_can_send_to_dash(uint16_t data);


void ecu_can_send_alive();
void ecu_can_send_voltage(float voltage);
void ecu_can_send_rpm(float rpm);

#endif /* ECU_CAN_MESSAGES_H_ */