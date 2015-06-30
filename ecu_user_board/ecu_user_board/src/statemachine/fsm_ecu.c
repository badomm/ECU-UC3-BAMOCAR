/*
 * fsm_ecu.c
 *
 * Created: 26.03.2014 19:20:59
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


#define ATTEMPT_LIMIT				100
#define MOMENT_OF_TRUTH				1638 //5 % Torque
#define SOFTWARE_TIMER_10_SEC		500
#define SOFTWARE_TIMER_5_SEC		250
#define SOFTWARE_TIMER_0_5_SEC		25
#define SOFTWARE_TIMER_1_SEC		50



void fsm_ecu_init(fsm_ecu_data_t *ecu_data) {
	ecu_data->state = STATE_STARTUP;
	ecu_data->vdc_battery = 0;
	ecu_data->inverter_vdc = 0;
	ecu_data->rpm = 0;
	ecu_data->motor_temp = 0;
	ecu_data->inverter_temp = 0;
	ecu_data->flag_start_precharge = 0;
	ecu_data->max_cell_temp = 0;
	ecu_data->drive_enable = false;
	ecu_data->inverter_error = 0;
	ecu_data->ecu_error = 0;
	ecu_data->reboot = 0;
	ecu_data->config_max_trq = 100;
	ecu_data->kers_factor = 0;
	ecu_data->bms_current = 0;
}

void setLED(bool a, bool b, bool c, bool d){
	if(a)gpio_set_pin_low(LED1); 
	else gpio_set_pin_high(LED1);
	
	if(b)gpio_set_pin_low(LED2);
	else gpio_set_pin_high(LED2);
	
	if(c)gpio_set_pin_low(LED3);
	else gpio_set_pin_high(LED3);
	
	if(d)gpio_set_pin_low(LED4);
	else gpio_set_pin_high(LED4);
}

fsm_ecu_state_t fsm_ecu_run_state( fsm_ecu_state_t current_state, fsm_ecu_data_t *data) {
	get_new_data(data);

	fsm_ecu_state_t newState;
	switch(current_state){
		case STATE_STARTUP: newState = fsm_ecu_state_startup_func( data ); setLED(1,0,0,0); break;
		case STATE_CHARGED: newState = fsm_ecu_state_charged_func( data); setLED(0,1,0,0); break;
		case STATE_ENABLE_DRIVE: newState = fsm_ecu_state_enable_drive_func( data ); setLED(1,1,0,0); break;
		case STATE_READY: newState = fsm_ecu_state_ready_func(data); setLED(0,0,1,0); break;
		case STATE_ERROR: newState = fsm_ecu_state_error_func(data); setLED(1,0,1,0); break;
		default: newState = STATE_ERROR;
	}
	return newState;
}

enum startup_states{STARTUP_CHECK_INVERTER_VOLTAGE, STARTUP_PRECHARGING, STARTUP_REQUEST_INVERTER_ERROR, STARTUP_CHECK_INVERTER_ERRROR };
fsm_ecu_state_t fsm_ecu_state_startup_func( fsm_ecu_data_t *ecu_data ) {
	fsm_ecu_state_t next_state = STATE_STARTUP;
	static enum startup_states internal_state = STARTUP_CHECK_INVERTER_VOLTAGE;
	static uint8_t precharge_timer = 0;
	static uint16_t attempts =	0;

	switch (internal_state) {
		case STARTUP_CHECK_INVERTER_VOLTAGE:
					if (ecu_data->inverter_vdc > 0) {
						internal_state = STARTUP_PRECHARGING;
						attempts = 0;
						}else{
						attempts++;
					}
		break;
		case STARTUP_PRECHARGING:
					if (precharge_timer < 3*SOFTWARE_TIMER_1_SEC) {
						precharge_timer++;
						} else {
						precharge_timer = 0;
						ecu_dio_inverter_clear_error();
						ecu_data->inverter_error = 0xDEAD;
						ecu_can_inverter_read_reg(ERROR_REG);
						internal_state = STARTUP_REQUEST_INVERTER_ERROR;
					}
		break;
			
		case STARTUP_REQUEST_INVERTER_ERROR:
					if (ecu_data->inverter_error != 0xDEAD) {
						internal_state = STARTUP_CHECK_INVERTER_ERRROR;
						attempts = 0;
					} else {
						ecu_can_inverter_read_reg(ERROR_REG);
						attempts++;
					}
		break;
			
		case STARTUP_CHECK_INVERTER_ERRROR:
					if( !check_inverter_error(ecu_data)) {
						attempts = 0; //Reset
						internal_state = STARTUP_CHECK_INVERTER_VOLTAGE; //Reset
						gpio_set_pin_high(AIR_PLUS);
						next_state =  STATE_CHARGED;
					}else{
						attempts++;
					}
		break;
			
		default:
		break;
	}
	
	
	
	if (attempts > ATTEMPT_LIMIT) {
		switch (internal_state) {
			case 0:
				ecu_data->ecu_error |= (1 << ERR_BMS_COM);
			break;
			case 1:
				ecu_data->ecu_error |= (1 << ERR_INVERTER_VDC_LOW);
			break;
			case 2:
				ecu_data->ecu_error |= (1 << ERR_INVERTER_COM);
			break;
			case 3:
				ecu_data->ecu_error |= (1 << ERR_INVERTER_INTERNAL);
			break;
		}
		attempts = 0;
		internal_state = 0;
		next_state =  STATE_ERROR;
	}
	return next_state;
}

fsm_ecu_state_t fsm_ecu_state_charged_func( fsm_ecu_data_t *ecu_data ) {
	fsm_ecu_state_t next_state = STATE_CHARGED;
	
	//Check charge rate, if bad, go to startup
	if (ecu_data->inverter_vdc < 50) {
		inverter_turnOff();
		ecu_data->flag_start_precharge = 0;
		//Reinitialize ECU here if still <90% error
		return STATE_STARTUP;
	}
	if (gpio_pin_is_low(AIR_PLUS)) {
		ecu_data->ecu_error |= (1 << ERR_AIR_PLUS);
		next_state = STATE_ERROR;
	}
	if (next_state != STATE_ERROR &&  ecu_data->drive_enable) {
		next_state = STATE_ENABLE_DRIVE;
	}
	return next_state;
};


enum enableDriveStates{ENABLE_DRIVE_TURN_ON_INVERTER, ENABLE_DRIVE_ERROR_CHECK1, ENABLE_DRIVE_ERROR_CHECK2};
fsm_ecu_state_t fsm_ecu_state_enable_drive_func( fsm_ecu_data_t *ecu_data ) {
	fsm_ecu_state_t next_state = STATE_ENABLE_DRIVE;
	static enum enableDriveStates internal_state = 0;
	static uint16_t attempts =	0;
	
	if (ecu_data->inverter_vdc < 50) {
		inverter_turnOff();
		ecu_data->flag_start_precharge = 0;
		//Reinitialize ECU here if still <90% error
		return STATE_STARTUP;
	}

	if(!ecu_data->drive_enable){
		inverter_turnOff();
		gpio_set_pin_high(AIR_PLUS);
		return STATE_CHARGED;
	}
	
	switch (internal_state) {
		case ENABLE_DRIVE_TURN_ON_INVERTER:
					gpio_set_pin_high(RFE_PIN);
					ecu_can_inverter_enable_drive();
					gpio_set_pin_high(FRG_PIN);
					if (gpio_pin_is_high(INVERTER_DOUT1)) {
						attempts = 0;
						internal_state = ENABLE_DRIVE_ERROR_CHECK1;
						ecu_dio_inverter_clear_error();
						ecu_data->inverter_error = 0xDEAD;
						ecu_can_inverter_read_reg(ERROR_REG);
					}
					attempts++;
		break;
			
		case ENABLE_DRIVE_ERROR_CHECK1:
					if (ecu_data->inverter_error != 0xDEAD) {
						internal_state = ENABLE_DRIVE_ERROR_CHECK2;
						attempts = 0;
						} else {
						ecu_can_inverter_read_reg(ERROR_REG);
						attempts++;
					}
		break;
			
		case ENABLE_DRIVE_ERROR_CHECK2:
					if (check_inverter_error(ecu_data) == 0) {
						internal_state = ENABLE_DRIVE_TURN_ON_INVERTER; //Reset for next possible restart
						next_state = STATE_READY;
					} else {
						//set error code - return inverters error register?
						ecu_data->ecu_error |= (1 << ERR_INVERTER_INTERNAL);
						next_state = STATE_ERROR;
					}
		break;
			
		default:
		break;
	}

	if (attempts == ATTEMPT_LIMIT) {
		if (internal_state == 0) {
			ecu_data->ecu_error |= (1 << ERR_FRG);
		} else if (internal_state == 1) {
			ecu_data->ecu_error |= (1 << ERR_INVERTER_COM);
		}
		attempts = 0; //Reset
		internal_state = 0; //Reset
		next_state = STATE_ERROR;
	}
	
	return next_state;
};
	
fsm_ecu_state_t fsm_ecu_state_ready_func( fsm_ecu_data_t *ecu_data ) {
	fsm_ecu_state_t next_state = STATE_READY;
	int16_t kers = 0;

	if (ecu_data->inverter_vdc < 50) {
		inverter_turnOff();
		ecu_data->flag_start_precharge = 0;
		return STATE_STARTUP;
	}
	
	if(!ecu_data->drive_enable){
		inverter_turnOff();
		gpio_set_pin_high(AIR_PLUS);
		return STATE_CHARGED;
	}
	
	/* First set trq_cmd to 0. Will be updated if the following tests are passed. 
	 * If not, the motor will be disabled and zero torque requested (stored in inverter?). 
	 * When transitioning from plausibility error state to ready state, the zero command
	 * stored in inverter memory will be used (it may also be zero by default) */
 	int16_t trq_cmd = 0;

	kers = calc_kers(ecu_data);
	if (kers < 0) {
		trq_cmd = kers;
	} else {
		trq_cmd = (int16_t)ecu_data->trq_request;
	}
	
 	ecu_can_inverter_torque_cmd(trq_cmd);
 	
	return next_state;
};

fsm_ecu_state_t fsm_ecu_state_error_func( fsm_ecu_data_t *ecu_data ) {
	fsm_ecu_state_t next_state = STATE_ERROR;

	inverter_turnOff();
	ecu_can_inverter_torque_cmd(0x0);
	
	if (ecu_data->reboot == 1) {
		ecu_data->reboot = 0;
		uint8_t max_torque = ecu_data->config_max_trq;
		fsm_ecu_init(ecu_data); // Reinitialize data struct
		ecu_data->config_max_trq = max_torque;
		next_state = STATE_STARTUP;
	}
	return next_state;
};

