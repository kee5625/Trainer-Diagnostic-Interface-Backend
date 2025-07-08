/*
 * USART.c
 *
 *  Created on: Jun 12, 2025
 *      Author: nbatcher
 */
#include "stdio.h"
#include "stdlib.h"
#include "main.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "cmsis_os.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_uart.h"
#include "USART.h"
#include "blinking.h"
#include "..\..\STM32CubeIDE\Application\User\TouchGFX\App\TC_Bridge.hpp"

#define tc_size 5
#define uart_start_pad 1
#define uart_end_pad 6

//global static
static char trouble_code[6];
static uint8_t rx_data;
static bool retry_last = false;
static bool TC_code_rx = false; //if true then TC is expected as next input.
static osMessageQueueId_t  send_task_queue;
static osMessageQueueId_t  receive_task_queue;

//thread setup
osThreadId_t send_handle;
const osThreadAttr_t sendTask_attributes = {
  .name = "sendTask",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityLow,
};
osThreadId_t receive_handle;
const osThreadAttr_t receiveTask_attributes = {
  .name = "receiveTask",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityLow1,
};
osThreadId_t blink_handle;
const osThreadAttr_t blinkTask_attributes = {
  .name = "blinkTask",
  .stack_size = 256,
  .priority = (osPriority_t) osPriorityBelowNormal2,
};

//padding added for error checking
static inline uint8_t uart_byte_setup(uart_comms_t command){
		return     ((uart_start_pad & 0x01) << 7) |
				   ((command & 0x0F) << 3) |
				   ((uart_end_pad & 0x07) << 0);
}

static inline bool is_valid_frame(uint8_t byte){
	return ((byte >> 7) & uart_start_pad) && ((byte & 0x07) & uart_end_pad);
}

void init_error_check(void *ptr){
	if (ptr == NULL){
		Error_Handler();
	}
}

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
		if (TC_code_rx){
			HAL_UART_Receive_IT(huart,(uint8_t *)trouble_code,tc_size + 1);
		}else{
			HAL_UART_Receive_IT(huart, &rx_data, 1);
		}
    }
}
/*
 * Function Description: Sends action passed to send_task_queue
 * Note: see UART_RX() for expected command order.
 *
 */
static void UART_TX(){
	uart_comms_t tx_action;
	int wait_time = 1000;
	uint8_t byte;
	while(1){
		if(retry_last || osMessageQueueGet(send_task_queue, &tx_action, NULL, osKernelGetTickFreq()) == osOK){
				retry_last = false;
				///////////////////////////////////////////////////////////////////////////////////////////
				byte = uart_byte_setup(tx_action);
				HAL_UART_Transmit_IT(&huart1, &byte,1);
				///////////////////////////////////////////////////////////////////////////////////////////
				if (tx_action == TC_Reset_cmd ){
					osDelay(pdMS_TO_TICKS(5000));
				}
				osDelay(pdMS_TO_TICKS(wait_time));
		}
	}
}

/*
 * Function Description: Grabs receive_task_queue and follows appropriate action from queue.
 *
 *
 * Below is the expected command order for the display(master) from the gateway(slave).
 * CMD order send  : start     , TC request, TC received, TC reset, start....
 * CMD oder receive: received, TC        ,            , received, received...
 */
static void UART_RX(){
	uint8_t byte;
	uart_comms_t rx_action;
	osStatus_t rx_Queue_Get;
	uint32_t ticks = osKernelGetTickFreq() * 4;

	while(1){
		rx_Queue_Get = osMessageQueueGet(receive_task_queue, &rx_action, NULL,ticks);
		if(rx_Queue_Get == osOK){
			if (!TC_code_rx){
				if (is_valid_frame(rx_data)){
					rx_action = ((rx_data>>3) & 0x0F); // Call back only to prevent overlapping receives or writes
				}else{
					rx_action = retry_cmd;
				}
			}else{
				rx_action =TC_Receiving; // default condition of switch
			}

			switch (rx_action){
			case received_cmd:
				if (ticks == osWaitForever){
					ticks = osKernelGetTickFreq() * 4;
				}else{
					ticks = osWaitForever;
					TC_code_rx = true;
					byte = TC_Req_cmd;
					osMessageQueuePut(send_task_queue, &byte, 0, osWaitForever);
					HAL_UART_Receive_IT(&huart1,(uint8_t *)trouble_code,tc_size + 1);
				}
				break;
			case retry_cmd:
				retry_last = true;
				break;
			case Read_live_cmd:
				break;
			case TC_Receiving:
				if (isalpha(trouble_code[0]) && (trouble_code[0] == 'P' || trouble_code[0] == 'C' || trouble_code[0] == 'B' || trouble_code[0] == 'U')){
					osSemaphoreRelease(blink_sem);
					TC_code_rx = false;
					byte = TC_Received_cmd;
					osMessageQueuePut(send_task_queue, &byte, 0, 0);
					TC_GUI_Pass(trouble_code);
					//RX rearmed by reset UART_REST_TC()
				}else{
					byte = TC_Req_cmd;
					osMessageQueuePut(send_task_queue, &byte, 0, 0);
					HAL_UART_Receive_IT(&huart1,(uint8_t *)trouble_code,tc_size + 1);
				}
				break;
			default:
				break;
			}
		}else if (rx_Queue_Get == osErrorTimeout ){
			byte = start_cmd;
			if (osMessageQueuePut(send_task_queue, &byte, 0, osKernelGetTickFreq() / 5) == osOK) {
				HAL_UART_Receive_IT(&huart1,&rx_data, 1);
			}
		}

	}
}

/*
 * Function Description: Called by GUI to start reset TC procedure.
 *
 */
void UART_REST_TC(){
	uint8_t byte;
	memcpy(trouble_code,"XXXXX\0", sizeof(trouble_code));
	TC_GUI_Pass(trouble_code); //not needed because the screen can't switch without new code**********************************************************************************************************
	byte = TC_Reset_cmd;
	osMessageQueuePut(send_task_queue, &byte, 0, 0);
	HAL_UART_Receive_IT(&huart1,&rx_data, 1);
}

/*
 * Function Description: Used to start and initiate UART. Also for clean up.
 *
 */
void Get_TC_USART(){

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

  //setting interrupts**********************************************************************************
  uint8_t byte;
  byte = start_cmd;
  osMessageQueuePut(send_task_queue, &byte, 0, 0);
  HAL_UART_Receive_IT(&huart1,&rx_data, 1);

  osThreadExit();
}


