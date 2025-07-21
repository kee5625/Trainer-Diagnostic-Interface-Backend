/*
 * USART.c
 *
 *  Created on: Jun 12, 2025
 *      Author: nbatcher
 */
#include "stdio.h"
#include "stdlib.h"
#include "main.h"
#include <stdbool.h>
#include <ctype.h>
#include "cmsis_os.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_uart.h"
#include "USART.h"
#include "blinking.h"
#include "..\..\STM32CubeIDE\Application\User\TouchGFX\App\TC_Bridge.hpp"
#include "UART_COMMS.hpp"

#define uart_start_pad 1
#define uart_end_pad 6
#define IF_BIT_SET(byte,bit)  ((byte) & (1<< (bit)))
#define IF_BIT_RESET(byte,bit) (!((byte) & (1<< (bit))))
#define HEXCHAR(n) ((n) < 10 ? ('0' + (n)) : ('A' + (n) - 10))

//global static
static char *dtcs_list = NULL; 	   				//list of all DTCs(each DTC is 5 characters long)
static int dtcs_size = 0;		   				//total size of DTC list
static int cur_dtcs_list_size = 0; 				//tracks current size for re-alloc
static uint8_t rx_data;            				//raw UART bytes
static bool retry_last = false;    				//used for retry command
uart_comms_t curr_service;		   				//marks current service being request(GUI sets)
static osMessageQueueId_t  send_task_queue;
static osMessageQueueId_t  receive_task_queue;

//thread setup
osThreadId_t send_handle;
const osThreadAttr_t sendTask_attributes = {
  .name = "sendTask",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityHigh,
};
osThreadId_t receive_handle;
const osThreadAttr_t receiveTask_attributes = {
  .name = "receiveTask",
  .stack_size = 1024,
  .priority = (osPriority_t) osPriorityRealtime,
};
osThreadId_t blink_handle;
const osThreadAttr_t blinkTask_attributes = {
  .name = "blinkTask",
  .stack_size = 256,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

//padding added for error checking on commands
static inline uint8_t uart_byte_setup(uart_comms_t command){
		return     ((uart_start_pad & 0x01) << 7) |
				   ((command & 0x0F) << 3) |
				   ((uart_end_pad & 0x07) << 0);
}

//returns true if command is valid
static inline bool is_valid_frame(uint8_t byte){
	return ((byte >> 7) & uart_start_pad) && ((byte & 0x07) & uart_end_pad);
}

void init_error_check(void *ptr){
	if (ptr == NULL){
		Error_Handler();
	}
}

//custom re-alloc for RTOS
void *freertos_realloc(void* old_ptr, size_t old_size, size_t new_size){
	void* new_ptr = pvPortMalloc(new_size);
	if (!new_ptr) return NULL;

	if (old_ptr && old_size > 0) {
		memcpy(new_ptr, old_ptr, (old_size < new_size) ? old_size : new_size);
		vPortFree(old_ptr);
	}else{
		memset(new_ptr, 0, new_size);
	}
	return new_ptr;
}

/**
 * Function Description: Converts read in bytes into trouble codes and then adds them
 * to the static global dtcs_list
 */
static void dtcs_conv_add(uint8_t first_byte, uint8_t second_byte) {
    char dtcs_con[6];  // Fixed-size stack buffer

    // Determine the prefix letter
    if ((first_byte & 0xC0) == 0xC0)      dtcs_con[0] = 'U';
    else if (first_byte & 0x80)           dtcs_con[0] = 'B';
    else if (first_byte & 0x40)           dtcs_con[0] = 'C';
    else                                  dtcs_con[0] = 'P';

    // Convert to hex characters
    dtcs_con[1] = HEXCHAR((first_byte >> 4) & 0x03);
    dtcs_con[2] = HEXCHAR(first_byte & 0x0F);
    dtcs_con[3] = HEXCHAR((second_byte >> 4) & 0x0F);
    dtcs_con[4] = HEXCHAR(second_byte & 0x0F);
    dtcs_con[5] = '\0';

    if (!dtcs_list) {
        dtcs_list = (char *)pvPortMalloc(1);
        if (!dtcs_list) return;
        memset(dtcs_list, 0, 1);
        cur_dtcs_list_size = 1;
    }

    char* new_buf = (char*)freertos_realloc(dtcs_list, cur_dtcs_list_size, cur_dtcs_list_size + 5);
    if (!new_buf) return;
    dtcs_list = new_buf;
    memcpy(dtcs_list + cur_dtcs_list_size - 1, dtcs_con, 5);
    cur_dtcs_list_size += 5;
    dtcs_list[cur_dtcs_list_size - 1] = '\0';
}


/**
 * UART Callback funcitons
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == USART1){
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1){
		osMessageQueuePut(receive_task_queue,&rx_data, 0,0);
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
    if (huart->Instance == USART1) {
    	uint8_t byte;
    	//clear flags
    	__HAL_UART_CLEAR_FLAG(&huart1, UART_CLEAR_PEF | UART_CLEAR_FEF | UART_CLEAR_OREF);
    	//flush buffer
        volatile uint8_t dummy;
		while (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE)) {
			dummy = huart->Instance->RDR;
		}
		//setting UART_TX() to resend last command
		retry_last = true;
		//set receive again based on expected input
		HAL_UART_Receive_IT(huart,&byte,1);

    }
}

/*
 * Note: see UART_RX() for expected command order.
 *
 */
static void UART_TX(){
	uart_comms_t tx_action;
	uint8_t byte;
	uint32_t ticks = osWaitForever;
	osStatus_t queue_get;
	while(1){
		queue_get = osMessageQueueGet(send_task_queue, &tx_action, NULL, ticks);
		if(retry_last || queue_get == osOK ){
			retry_last = false;
			///////////////////////////////////////////////////////////////////////////////////////////
			byte = uart_byte_setup(tx_action);
			HAL_UART_Transmit_IT(&huart1, &byte,1);
			///////////////////////////////////////////////////////////////////////////////////////////
			if (tx_action == UART_DTCs_Reset_cmd ) osDelay(pdMS_TO_TICKS(200));
			if (tx_action == UART_Start_cmd && dtcs_size == 0 ) {
				ticks = osKernelGetTickFreq() * 1.5;
			}else{
				ticks = osWaitForever;
			}
		}else if (queue_get == osErrorTimeout){
			retry_last = true;
			if (huart1.RxState != HAL_UART_STATE_BUSY_RX) HAL_UART_Receive_IT(&huart1,&rx_data,1);

		}
	}
}

/*
 * Function Description: Grabs receive_task_queue and follows appropriate action from queue.
 *
 *
 * Below is the expected command order for the display(master) from the gateway(slave).
 * CMD order send  : start     , TC request, TC received, TC received,TC reset, start....
 * CMD oder receive: received  , NUM_TC    , DTC0       , DTC1       ,received, received...
 */
static void UART_RX(){
	uint8_t byte;
	int last_byte = -999;
	uart_comms_t rx_action;
	osStatus_t rx_Queue_Get;
	bool DTCs_code_rx = false; //expecting DTCs

	while(1){
		rx_Queue_Get = osMessageQueueGet(receive_task_queue, &rx_data, NULL,osWaitForever);
		if(rx_Queue_Get == osOK){
			if (!DTCs_code_rx){
				if (is_valid_frame(rx_data)){
					rx_action = ((rx_data>>3) & 0x0F); // Call back only to prevent overlapping receives or writes
				}else{
					rx_action = UART_Retry_cmd;
				}
			}else{
				rx_action = UART_DTCs_Receiving; // set after receiving received command from start command
			}

			switch (rx_action){
			case UART_Received_cmd:
				DTCs_code_rx = true;
				byte = curr_service;
				HAL_UART_Receive_IT(&huart1,&rx_data,1);
				osMessageQueuePut(send_task_queue, &byte, 0, osWaitForever);
				break;
			case UART_Retry_cmd:
				retry_last = true;
				HAL_UART_Receive_IT(&huart1,&rx_data,1);
				break;
			case UART_DTCs_Receiving:
				if (dtcs_size == 0){
					dtcs_size = rx_data;
					last_byte = -999;
					byte = UART_DTCs_Received_cmd;
					HAL_UART_Receive_IT(&huart1,&rx_data, 1);
					osMessageQueuePut(send_task_queue, &byte, 0, 0);
				}else if (last_byte == -999 && is_valid_frame(rx_data) && ((rx_data>>3) & 0x0F) == UART_DTCs_End_cmd){
					DTCs_code_rx = false;
					osSemaphoreRelease(blink_sem); //for testing: shows DTCs are ready
					DTCs_GUI_Pass(dtcs_list,dtcs_size);
				}else{
					if(last_byte >= 0){
						dtcs_conv_add((uint8_t) last_byte, rx_data); //converts and adds to global static DTCs list
						byte = UART_DTCs_Received_cmd;
						osMessageQueuePut(send_task_queue, &byte, 0, 0);
						last_byte = -999;
					}else{
						last_byte = rx_data;
					}
					HAL_UART_Receive_IT(&huart1,&rx_data, 1);
				}
				break;
			default:
				break;
			}
		}
	}

}


/**
 * Function decription: Setting current service
 */
void UART_Set_Service(uart_comms_t ser){
	uint8_t byte;
	curr_service = ser;
	vPortFree(dtcs_list);
	dtcs_list = NULL;
	dtcs_size = 0;
	cur_dtcs_list_size = 0;
	retry_last = false;

	if (ser == UART_DTCs_Reset_cmd) {
		byte = ser;
		osMessageQueuePut(send_task_queue, &byte, 0, 0);
	}else{
		HAL_UART_Receive_IT(&huart1,&rx_data, 1);
		byte = UART_Start_cmd;
		osMessageQueuePut(send_task_queue, &byte, 0, 0);
	}
}

/*
 * Function Description: Used to start and initiate UART. Also for clean up.
 *
 */
void Get_TC_USART(){

	osDelay(pdMS_TO_TICKS(150)); //small delay for GUI setup
	//setting up FreeRTOS*********************************************************************************
	send_task_queue = osMessageQueueNew(3, sizeof(uart_comms_t),NULL);
	init_error_check(send_task_queue);
	receive_task_queue = osMessageQueueNew(1, sizeof(uart_comms_t),NULL);
	init_error_check(receive_task_queue);
	receive_handle = osThreadNew(UART_RX, NULL, &receiveTask_attributes);
	init_error_check(receive_handle);
	send_handle = osThreadNew(UART_TX, NULL, &sendTask_attributes);
	init_error_check(send_handle);
	blink_handle = osThreadNew(blk_toggle_led,NULL,&blinkTask_attributes);
	init_error_check(blink_handle);

	osThreadExit();
}


