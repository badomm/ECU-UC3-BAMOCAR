/*
 * fsm_ecu_functions.h
 *
 * Created: 22.05.2014 17:26:21
 *  Author: oyvinaak
 */ 


#ifndef FSM_ECU_FUNCTIONS_H_
#define FSM_ECU_FUNCTIONS_H_

bool torque_plausibility_check(fsm_ecu_data_t *ecu_data);
int16_t calc_kers(fsm_ecu_data_t *ecu_data);
uint8_t check_inverter_error(fsm_ecu_data_t *ecu_data);
uint8_t get_trq_sens(fsm_ecu_data_t *ecu_data);
uint16_t convert_num_to_vdc(uint32_t num);
uint16_t convert_to_big_endian(uint32_t data);
void ecu_dio_inverter_clear_error(void);
void get_new_data(fsm_ecu_data_t *ecu_data);
void handle_bms_data(fsm_ecu_data_t *ecu_data, car_can_msg_t bms_msg);
void handle_inverter_data(fsm_ecu_data_t *ecu_data, inverter_can_msg_t inverter_can_msg);
void inverter_turnOff();







#endif /* FSM_ECU_FUNCTIONS_H_ */