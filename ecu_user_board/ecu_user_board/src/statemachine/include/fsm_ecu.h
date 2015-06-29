/*
 * fsm_ecu.h
 *
 * Created: 26.03.2014 19:20:41
 *  Author: oyvinaak
 */ 


#ifndef FSM_ECU_H_
#define FSM_ECU_H_

#include "ecu_can.h"
#include <stdbool.h>
#define TRQ_MISSED_LIMIT 2



/* ECU error register			BIT	 Explanation */
#define ERR_BMS_COM				0 // Lost communicating with BMS
#define ERR_INVERTER_VDC_LOW	1 // Inverter side voltage not >90 % of battery pack voltage
#define ERR_INVERTER_COM		2 // Lost communication with inverter
#define ERR_INVERTER_INTERNAL	3 // Internal error in inverter
#define ERR_TRQ_SENSORS			4 // Lost communication with torque sensors
#define ERR_SPEED_SENSORS		5 // Lost communication with speed sensors
#define ERR_FRG					6 // FRG is low
#define ERR_AIR_PLUS			7 // AIR PLUS is low
#define ERR_BSPD				8 // BSPD signal loss

		


typedef enum fsm_ecu_state{
	STATE_STARTUP,
	STATE_CHARGED,
	STATE_ENABLE_DRIVE,
	STATE_READY,
	STATE_PLAUSIBILITY_ERROR,
	STATE_ERROR,
	FSM_ECU_NUM_STATES,
} fsm_ecu_state_t;



enum carState_t{READY_TO_START = 1, PLAYING_RTDS = 2, READY_TO_DRIVE = 3};

typedef struct fsm_ecu_data{	
	float trq_request;
	fsm_ecu_state_t state;
	
	uint16_t vdc_battery;
	uint16_t inverter_vdc;
	uint16_t rpm;
	uint16_t motor_temp;
	uint16_t inverter_temp;
	uint8_t flag_start_precharge;
	int8_t max_cell_temp;
	bool drive_enable;
	uint16_t inverter_error;
	uint16_t ecu_error;
	uint8_t reboot;
	uint8_t config_max_trq;
	int16_t kers_factor;
	int16_t bms_current;
}fsm_ecu_data_t;

void fsm_ecu_init(fsm_ecu_data_t *data);

typedef fsm_ecu_state_t fsm_ecu_state_func_t( fsm_ecu_data_t *data );

fsm_ecu_state_t fsm_ecu_run_state( fsm_ecu_state_t current_state, fsm_ecu_data_t *data);
fsm_ecu_state_t fsm_ecu_state_startup_func( fsm_ecu_data_t *data );
fsm_ecu_state_t fsm_ecu_state_charged_func( fsm_ecu_data_t *data );
fsm_ecu_state_t fsm_ecu_state_enable_drive_func( fsm_ecu_data_t *data );
fsm_ecu_state_t fsm_ecu_state_ready_func( fsm_ecu_data_t *data );
fsm_ecu_state_t fsm_ecu_state_error_func( fsm_ecu_data_t *data );


#endif /* FSM_ECU_H_ */