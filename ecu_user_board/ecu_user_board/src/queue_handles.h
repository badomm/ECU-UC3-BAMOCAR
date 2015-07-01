/*
 * queue_handles.h
 *
 * Created: 20.02.2014 22:05:03
 *  Author: oyvinaak
 */ 


#ifndef QUEUE_HANDLES_H_
#define QUEUE_HANDLES_H_

#include "FreeRTOS.h"
#include "queue.h"

#define QUEUE_INVERTER_RX_LEN	5
#define QUEUE_DASH_MSG_LEN		5
#define QUEUE_BMS_RX_LEN		5
#define QUEUE_ECU_RX_LEN		5
#define QUEUE_WHEEL_SPEED_RX_LEN 10;

/* Queues for task communication */
xQueueHandle queue_from_inverter;
xQueueHandle queue_to_inverter;
xQueueHandle queue_dash_msg;
xQueueHandle queue_bms_rx;
xQueueHandle queue_ecu_rx;
xQueueHandle queue_torque_request_ecu;
xQueueHandle queue_wheelSpeed;



#endif /* QUEUE_HANDLES_H_ */