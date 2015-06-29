/*
 * fsm_ecu_functions.c
 *
 * Created: 22.05.2014 17:26:07
 *  Author: oyvinaak
 */ 
#include <asf.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "INVERTER_defines.h"
#include "queue_handles.h"
#include "fsm_ecu.h"
#include "fsm_ecu_functions.h"


#define BMS_PRECHARGE_BIT			3
#define MAX_KERS					(int16_t)-3277

#define TRQ_TIMEOUT  5;
static int torqueRequestTimeout;



int16_t calc_kers(fsm_ecu_data_t *ecu_data) {
	float speed = 0;//ecu_data->WRR_sens & 0xFF; //
	speed = speed*2.574;
	static bool allow_kers = false;
	
	if (speed > 10) {
		allow_kers = true;
	}
	
	if (allow_kers) {
			if (speed > 5.5) { //km/h
				if ((ecu_data->max_cell_temp > 0) && (ecu_data->max_cell_temp < 44)) {
					return (MAX_KERS*ecu_data->kers_factor)/100; //TODO Return a value from ecu_data that is received from dash
				}
			} else {
				allow_kers = false;
			}
	}
	return 1;
}



uint8_t check_inverter_error(fsm_ecu_data_t *ecu_data) {
	uint16_t temp = ecu_data->inverter_error;
	return (uint8_t)(temp & 1 << PWR_FAULT) | (temp & 1 << RFE_FAULT) | (temp & 1 << RESOLVER_FAULT);
}




uint16_t convert_num_to_vdc(uint32_t num) {
	/* num = 33.2*vdc - 827 */
	uint32_t num_be = convert_to_big_endian(num);
	return (uint16_t)(float)((num_be + 827) / 33.2);
}

uint16_t convert_to_big_endian(uint32_t data) {
	/* Input: 0x49 D2 2A 00 Output:0x2A D2 */
	uint16_t relevant_data = (data & 0xFFFF00) >> 8;
	return ((relevant_data & 0xFF) << 8 | (relevant_data & 0xFF00) >> 8);
}

void ecu_dio_inverter_clear_error() {
	gpio_set_pin_high(INVERTER_DIN1);
	delay_us(200);
	gpio_set_pin_low(INVERTER_DIN1);
}


void handle_inverter_data(fsm_ecu_data_t *ecu_data, inverter_can_msg_t inverter_can_msg) {
	/* Note on receiving inverter data
	 * Most data is 4 byte long, but e.g. error and state
	 * register will produce a 6 byte message.
	 * When checking for FRG_BIT it is implied that the message
	 * consist of 4 bytes etc. Refer to manual for the individual
	 * bits in state and error reg.
	 * Temp e.g.: 49d62a00 
	 */
	uint16_t temp;
	switch (inverter_can_msg.data.u8[0]) {
		case BTB_REG:
			break;
		case FRG_REG:
			break;
		case MOTOR_TEMP_REG:
			ecu_data->motor_temp = convert_to_big_endian(inverter_can_msg.data.u32[0]);
			break;
		case IGBT_TEMP_REG:
			ecu_data->inverter_temp = convert_to_big_endian(inverter_can_msg.data.u32[0]);
			break;
		case CURRENT_REG:
			break;
		case VDC_REG:
			/* 16 bit value */
			temp = convert_num_to_vdc(inverter_can_msg.data.u32[0]);
			if (temp < 30) {
				ecu_data->inverter_vdc = 0;	
			} else {
				ecu_data->inverter_vdc = temp;
			}	 
			break;
		case RPM_REG:
			ecu_data->rpm = (MAX_RPM * convert_to_big_endian(inverter_can_msg.data.u32[0])) / 32767;
			break;
		case ERROR_REG:
			ecu_data->inverter_error = (inverter_can_msg.data.u32[0] & 0x00FFFF00) >> 8;
			break;
		default:
			break;
	}
}

#define ECU_CURRENT_CAR_STATE 0x633
#define ECU_IMPLAUSIBILITIES  0x635


void get_new_data(fsm_ecu_data_t *ecu_data) {
	uint8_t i;
	float torqueRequest;
	car_can_msg_t can_msg;
	inverter_can_msg_t inverter_can_msg;
	
	//Take care of Inverter Data
	while(xQueueReceive( queue_from_inverter, &inverter_can_msg, 0 ) == pdTRUE) {
		handle_inverter_data(ecu_data, inverter_can_msg);
	}
	
	//Handle TorqueRequest, if timeout, set request = 0
	if(xQueueReceive( torque_request_ecu, &torqueRequest, 0 ) == pdTRUE) {
		ecu_data->trq_request = torqueRequest;
		torqueRequestTimeout = TRQ_TIMEOUT;
	}else{
		if(!torqueRequestTimeout--){
			ecu_data->trq_request = 0;
		}
	}
}