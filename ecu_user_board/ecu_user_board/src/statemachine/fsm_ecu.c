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


#define BSPD_SIGNAL_LOSS_WARNING	0x42
#define BSPD_I_AM_ALIVE				0xAA
#define BSPD_PLAUSIBILITY_OCCURED	0x99
#define ATTEMPT_LIMIT				100
#define MOMENT_OF_TRUTH				1638 //5 % Torque
#define SOFTWARE_TIMER_10_SEC		500
#define SOFTWARE_TIMER_5_SEC		250
#define SOFTWARE_TIMER_0_5_SEC		25
#define SOFTWARE_TIMER_1_SEC		50

static uint16_t attempts =	0;

void fsm_ecu_init(fsm_ecu_data_t *ecu_data) {
	ecu_data->state = STATE_STARTUP;
	ecu_data->inverter_can_msg = (inverter_can_msg_t){.data.u64 = 0x0LL, .dlc = 0};
	ecu_data->trq_cmd = 0;
	ecu_data->dash_msg = (dash_can_msg_t){.data.u64 = 0x0LL, .id = 0};
	ecu_data->bms_msg = (bms_can_msg_t){.data.u64 = 0x0LL, .id = 0};
	ecu_data->vdc_battery = 0;
	ecu_data->inverter_vdc = 0;
	ecu_data->rpm = 0;
	ecu_data->motor_temp = 0;
	ecu_data->inverter_temp = 0;
	ecu_data->flag_start_precharge = 0;
	ecu_data->max_cell_temp = 0;
	ecu_data->flag_drive_enable = DRIVE_DISABLED;
	ecu_data->inverter_error = 0;
	ecu_data->ecu_error = 0;
	ecu_data->reboot = 0;
	ecu_data->config_max_trq = 100;
	ecu_data->kers_factor = 0;
	ecu_data->bms_current = 0;
}

fsm_ecu_state_func_t *const fsm_ecu_state_table[ FSM_ECU_NUM_STATES ] = {
	fsm_ecu_state_startup_func,
	fsm_ecu_state_charged_func,
	fsm_ecu_state_enable_drive_func,
	fsm_ecu_state_ready_func,
	fsm_ecu_state_plausibility_error_func,
	fsm_ecu_state_error_func,
};

fsm_ecu_state_t fsm_ecu_run_state( fsm_ecu_state_t current_state, fsm_ecu_data_t *data) {
	return fsm_ecu_state_table[ current_state ]( data );
};

fsm_ecu_state_t fsm_ecu_state_startup_func( fsm_ecu_data_t *ecu_data ) {
	fsm_ecu_state_t next_state = STATE_STARTUP;
	static uint8_t internal_state = 0;
	static uint8_t precharge_timer = 0;

	get_new_data(ecu_data);

	if (ecu_data->flag_start_precharge == 1) {
		switch (internal_state) {
			case 0:
			if (ecu_data->vdc_battery > 0) {
				if (ecu_data->inverter_vdc > 0) {
					internal_state = 1;
					attempts = 0;	
				}
			}
			attempts++;
			break;
			
			case 1:
			if (precharge_timer < 3*SOFTWARE_TIMER_1_SEC) {
				precharge_timer++;
			} else {
				precharge_timer = 0;
				ecu_dio_inverter_clear_error();
				ecu_data->inverter_error = 0xDEAD;
				ecu_can_inverter_read_reg(ERROR_REG);
				internal_state = 2;	
			}
			break;
			
			case 2:
			if (ecu_data->inverter_error != 0xDEAD) {
				internal_state = 3;
				attempts = 0;
			} else {
				ecu_can_inverter_read_reg(ERROR_REG);
				attempts++;
			}
			break;
			
			case 3:
			if(check_inverter_error(ecu_data) == 0) {
				attempts = 0; //Reset
				internal_state = 0; //Reset
				gpio_set_pin_high(AIR_PLUS);
				ecu_can_send_tractive_system_active();
				next_state =  STATE_CHARGED;
			}
			attempts++;
			break;
			
			default:
			break;
		}
	}
	
	
	if (attempts == ATTEMPT_LIMIT) {
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

	get_new_data(ecu_data);
	
	if (ecu_data->inverter_vdc < 50) {
		gpio_set_pin_low(FRG_PIN);
		gpio_set_pin_low(RFE_PIN);
		gpio_set_pin_low(AIR_PLUS);
		ecu_data->flag_start_precharge = 0;
		//Reinitialize ECU here if still <90% error
		return STATE_STARTUP;
	}
	
	if(ecu_data->flag_drive_enable == DRIVE_ENABLE_REQUEST) {
		if (gpio_pin_is_low(AIR_PLUS)) {
			ecu_data->ecu_error |= (1 << ERR_AIR_PLUS);
			next_state = STATE_ERROR;
		}
		if (next_state != STATE_ERROR) {
				next_state = STATE_ENABLE_DRIVE;
		}
	}
	return next_state;
};
	
fsm_ecu_state_t fsm_ecu_state_enable_drive_func( fsm_ecu_data_t *ecu_data ) {
	fsm_ecu_state_t next_state = STATE_ENABLE_DRIVE;
	static uint8_t internal_state = 0;
	get_new_data(ecu_data);
	
	if (ecu_data->inverter_vdc < 50) {
		gpio_set_pin_low(FRG_PIN);
		gpio_set_pin_low(RFE_PIN);
		gpio_set_pin_low(AIR_PLUS);
		ecu_data->flag_start_precharge = 0;
		//Reinitialize ECU here if still <90% error
		return STATE_STARTUP;
	}
	
	if (ecu_data->flag_drive_enable == DRIVE_ENABLE_RTDS_PLAYS) {
		switch (internal_state) {
			case 0:
			gpio_set_pin_high(RFE_PIN);
			ecu_can_inverter_enable_drive();
			gpio_set_pin_high(FRG_PIN);
			if (gpio_pin_is_high(INVERTER_DOUT1)) {
				attempts = 0;
				internal_state = 1;
				ecu_dio_inverter_clear_error();
				ecu_data->inverter_error = 0xDEAD;
				ecu_can_inverter_read_reg(ERROR_REG);
			}
			attempts++;
			break;
			
			case 1:
			if (ecu_data->inverter_error != 0xDEAD) {
				internal_state = 2;
				attempts = 0;
			} else {
				ecu_can_inverter_read_reg(ERROR_REG);
				attempts++;
			}
			break;
			
			case 2:
			if (check_inverter_error(ecu_data) == 0) {
				internal_state = 0; //Reset for next possible restart
				ecu_data->flag_drive_enable = DRIVE_ENABLED;
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
	
	get_new_data(ecu_data);

	if (ecu_data->flag_drive_enable == DRIVE_DISABLE_REQUEST) {
		gpio_set_pin_low(FRG_PIN);
		ecu_data->flag_drive_enable = DRIVE_DISABLED;
		return STATE_CHARGED;
	}
	
	if (ecu_data->inverter_vdc < 50) {
		gpio_set_pin_low(FRG_PIN);
		gpio_set_pin_low(RFE_PIN);
		gpio_set_pin_low(AIR_PLUS);
		ecu_data->flag_drive_enable = DRIVE_DISABLED;
		ecu_data->flag_start_precharge = 0;

		return STATE_STARTUP;
	}
	
	
	uint8_t bspd = check_bspd();
	
	/* First set trq_cmd to 0. Will be updated if the following tests are passed. 
	 * If not, the motor will be disabled and zero torque requested (stored in inverter?). 
	 * When transitioning from plausibility error state to ready state, the zero command
	 * stored in inverter memory will be used (it may also be zero by default) */
 	ecu_data->trq_cmd = 0;
	if ( bspd == BSPD_SIGNAL_LOSS_WARNING ) {
		gpio_set_pin_low(RFE_PIN);
		gpio_set_pin_low(FRG_PIN);
		gpio_set_pin_low(AIR_PLUS);
		ecu_data->ecu_error |= (1 << ERR_BSPD);
		next_state = STATE_ERROR;
		
 	} 
		 
	else {
		kers = calc_kers(ecu_data);
		if (kers < 0) {
			ecu_data->trq_cmd = kers;
		} else {
			ecu_data->trq_cmd = (int16_t)ecu_data->trq_pedal;
 		}
	}
 	
 	ecu_can_inverter_torque_cmd(ecu_data->trq_cmd);
 	
	return next_state;
};

fsm_ecu_state_t fsm_ecu_state_plausibility_error_func( fsm_ecu_data_t *ecu_data ) {
	fsm_ecu_state_t next_state = STATE_PLAUSIBILITY_ERROR;

	get_new_data(ecu_data);

	gpio_set_pin_high(FRG_PIN);
	next_state = STATE_READY;
	
	ecu_data->trq_cmd = 0x0;
	ecu_can_inverter_torque_cmd(ecu_data->trq_cmd);
	return next_state;
};

fsm_ecu_state_t fsm_ecu_state_error_func( fsm_ecu_data_t *ecu_data ) {
	fsm_ecu_state_t next_state = STATE_ERROR;
	get_new_data(ecu_data);
	/* Disable AIR+ */
	gpio_set_pin_low(AIR_PLUS);
	gpio_set_pin_low(FRG_PIN);
	gpio_set_pin_low(RFE_PIN);
	ecu_data->trq_cmd = 0x0;
	ecu_can_inverter_torque_cmd(ecu_data->trq_cmd);
	
	if (ecu_data->reboot == 1) {
		ecu_data->reboot = 0;
		uint8_t max_torque = ecu_data->config_max_trq;
		fsm_ecu_init(ecu_data); // Reinitialize data struct
		ecu_data->config_max_trq = max_torque;
		next_state = STATE_STARTUP;
	}
	return next_state;
};

