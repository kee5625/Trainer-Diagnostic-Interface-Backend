/*
 * USART.c
 *
 *  Created on: Jun 12, 2025
 *      Author: nbatcher
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
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
#include "PIDs_Library.hpp"

//padding for uart commands
#define uart_start_pad 1
#define uart_end_pad 6

//other macros
#define IF_BIT_SET(byte,bit)  ((byte) & (1<< (bit)))
#define IF_BIT_RESET(byte,bit) (!((byte) & (1<< (bit))))
#define HEXCHAR(n) ((n) < 10 ? ('0' + (n)) : ('A' + (n) - 10))

//general globals
static uint8_t rx_byte;
static bool retry_last = false;    				//used for retry command
uart_comms_t cur_service = UART_SERVICE_IDLE;	//marks current service being request(GUI sets)
static bool resetFlag = false;

//DTCs globals
static char **dtcs_list = NULL; 	   		    //two d array of DTCs
static int dtcs_size = -1;		   				//total size of DTC list
static int cur_dtcs_list_size = 0; 				//tracks current size for re-alloc for DTCs list

//PID data globals
static uint8_t cur_pid = -1;
static uint8_t pid_bitmask[7][4];
static uint8_t *PID_VALUE;
static int pid_bytes = -1; 						//number of bytes expected for cur_pid

//RTOS globals
static osSemaphoreId_t SERV_DONE_sem;
static osMessageQueueId_t send_task_queue;
static osMessageQueueId_t receive_task_queue;
static osMessageQueueId_t control_queue;
osMutexId DTCList_MUT;                                               //used by everything to control access to DTCList


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
osThreadId_t control_handle;
const osThreadAttr_t controlTask_attributes = {
  .name = "controlTask",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityLow,
};
osThreadId_t blink_handle;
const osThreadAttr_t blinkTask_attributes = {
  .name = "blinkTask",
  .stack_size = 256,
  .priority = (osPriority_t) osPriorityBelowNormal,
};


/**
 * --------------------------------------------------------------Helper functions---------------------------------------------------------------------------------
 */

static inline void UART_Reset(){

	if (huart1.gState != HAL_UART_STATE_READY || huart1.RxState != HAL_UART_STATE_READY)
	{
		HAL_UART_Abort(&huart1);

		__HAL_UART_CLEAR_FLAG(&huart1, UART_CLEAR_OREF);
		__HAL_UART_CLEAR_FLAG(&huart1, UART_CLEAR_PEF);
		__HAL_UART_CLEAR_FLAG(&huart1, UART_CLEAR_FEF);
		__HAL_UART_CLEAR_FLAG(&huart1, UART_CLEAR_NEF);
	}

	if (huart1.RxState == HAL_UART_STATE_READY)  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void init_error_check(void *ptr){
	if (ptr == NULL){
		Error_Handler();
	}
}

static inline void ptrFree(void **ptr){
	if(ptr && *ptr){
		vPortFree(*ptr);
		*ptr = NULL;
	}

}

//custom re-alloc for RTOS
void *freertos_realloc(void* old_ptr, size_t old_size, size_t new_size){
	void* new_ptr = pvPortMalloc(new_size);
	if (!new_ptr) return NULL;

	if (old_ptr && old_size > 0) {
		memcpy(new_ptr, old_ptr, (old_size < new_size) ? old_size : new_size);
		ptrFree((void **) &old_ptr);
	}else{
		memset(new_ptr, 0, new_size);
	}
	return new_ptr;
}

/**
 * Function Description: Converts read in bytes into trouble codes and then adds them
 * to the static global dtcs_list
 */
static void DTC_Decode(uint8_t first_byte, uint8_t second_byte) {
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

    memcpy(dtcs_list[cur_dtcs_list_size], dtcs_con, 5); // null term already in init
    if (dtcs_list[cur_dtcs_list_size]){
    	cur_dtcs_list_size ++;
    }

}


/**
 * --------------------------------------------------------------mode/service functions---------------------------------------------------------------------------------
 */


/**
 * UART Protocol: DTC (Diagnostic Trouble Codes) Communication
 * ===========================================================
 *
 * Notes:
 * - Start commands and Receive commands are handled in the `rx` function.
 * - DTCs can be Pending, Stored, or Permanent depending on the request.
 *
 * ------------------------------------------------------------------------
 * From Gateway (to Display)
 * ------------------------------------------------------------------------
 * Description:
 *   Sends DTC data to the display after receiving a request.
 *
 * Byte packing:
 *   Byte 0     : Receive Command (e.g., 0xBB)
 *   Byte 1     : Total Number of DTCs for the category
 *   Byte 2     : DTC 0 (first half DTC)
 *   Byte 3     : DTC 0 (second half DTC)
 *   Byte 4     : DTC 1 (first half DTC)
 *   Byte 5     : DTC 1 (second half DTC)
 *   ...
 *   Byte N     : DTC N (first half DTC)
 *   Byte N+1   : DTC N (second half DTC)
 *   Final Byte : END Command (e.g., 0xEE)
 *
 *
 * DTC Encoding: 00      00   0111 / 0000 0011
 *              ^letter ^num ^num   ^num ^num
 *              P        0   B      0    3
 *	See https://en.wikipedia.org/wiki/OBD-II_PIDs for further explanation
 * 	of PID encoding.
 *
 * ------------------------------------------------------------------------
 * From Display (to ECU / Trainer)
 * ------------------------------------------------------------------------
 * Description:
 *   Sends a request for DTCs in a specific category.
 *
 * Byte Packing:
 * Byte 0               : Start Command (e.g., 0xAA)
 * Byte 1               : DTC Command Type:
 *                          - 0x04 = Stored
 *                          - 0x05 = Pending
 *                          - 0x06 = Permanent
 * Byte 2+ Num_bytes    : DTC Received cmd
 *
 *
 *
 */

static void Read_Codes(){
	uint8_t byte;
	int last_byte;

	while (1){

		if (dtcs_size == -1){

			dtcs_size = rx_byte;

			if (dtcs_size > 0){
				dtcs_list = (char **)pvPortMalloc(dtcs_size * sizeof(char *));
				if(!dtcs_list) return;

				for (int i = 0; i < dtcs_size; i ++){
					dtcs_list[i] = (char *)pvPortMalloc(6);
					if (!dtcs_list[i]) return;

					dtcs_list[i][5] = '\0';
				}
			}

			last_byte = -999;

			byte = UART_DTC_Received_cmd;
			HAL_UART_Receive_IT(&huart1,&rx_byte, 1);
			osMessageQueuePut(send_task_queue, &byte, 0, 0);

		}else if (last_byte == -999 && rx_byte == UART_DTCs_End_cmd){

			osSemaphoreRelease(blink_sem); //for testing: shows DTCs are ready
			DTCs_GUI_Pass(dtcs_list,dtcs_size);
			last_byte = -1;
			return;

		}else{

			if(last_byte >= 0){
				DTC_Decode((uint8_t) last_byte, rx_byte); //converts raw bytes to DTC and adds to global static dtcs_list
				byte = UART_DTC_Received_cmd;
				osMessageQueuePut(send_task_queue, &byte, 0, 0);
				last_byte = -999;

			}else{
				last_byte = rx_byte;

			}

			HAL_UART_Receive_IT(&huart1,&rx_byte, 1);

		}

		while(osMessageQueueGet(receive_task_queue, &rx_byte, NULL,pdMS_TO_TICKS(500)) != osOK){

			if (resetFlag) {
				break;
			}

			if (huart1.RxState == HAL_UART_STATE_READY)  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
		}

		if (resetFlag) {
			return;
		}

	}

}

/**
 * This funciton gets available PIDs bit-mask only. get_PID_Value gets the individual PID value
 *
 * UART Protocol Overview
 * ======================
 *
 * From Gateway (to Display):
 *   Byte 0   : Receive Command
 *   Byte 1-28: PIDs Supported Bitmask [7][4] (28 bytes)
 *   Byte 29  : Checksum (display repeat req if failed)
 *   Byte 30+ : Repeated blocks of:
 *              - Byte N    : Num of bytes for PID data
 *              - Byte N+1  : Data for PID
 *              - Byte N+2  : Checksum (display repeat req if failed)
 *
 * From Display (to Gateway):
 *   Byte 0   : Start Command (e.g. 0x20)
 *   Byte 1   : UART_PIDS Request Flag
 *   Byte 2+  : Sequence of PIDs requested (1 byte each)
 *   Final    : 0x20 sent
 */
static void Get_PID_BitMask(){
	int row = 0;
	int col = 0;
	uint8_t tx_byte = 0;
	uint8_t checksum = 0;
	bool checksum_failed = false;

	while (1){

		if (resetFlag) {
			osSemaphoreRelease(SERV_DONE_sem); //ready for next service
			return;
		}

		if (row != 7){
			pid_bitmask[row][col] = rx_byte;
			checksum += rx_byte;
		}

		//setting up for next call
		if (col == 3){
			row ++;
			col = 0;

		}else if (row == 7){
			checksum = ~ checksum;
			if (checksum == rx_byte) {
				break;

			}

			checksum_failed = true;
			row = 0;
			col = 0;
			checksum = 0;

		}else{
			col += 1;
		}

		//if failed the while loop below will start command chain again
		if(checksum_failed){
			tx_byte = 0x20; //exit PID data grab task
			osMessageQueuePut(send_task_queue, &tx_byte, 0, 0);
			osDelay(pdMS_TO_TICKS(25));
			tx_byte = UART_Start_cmd; //start over grabbing bit-mask
			osMessageQueuePut(send_task_queue, &tx_byte, 0, 0);
			osDelay(pdMS_TO_TICKS(25));
		}

		if (huart1.RxState == HAL_UART_STATE_READY)  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
		while((osMessageQueueGet(receive_task_queue, &rx_byte, NULL,pdMS_TO_TICKS(500)) != osOK) && (rx_byte != UART_Received_cmd && checksum_failed)){
			if (resetFlag) {
				break;
			}

			if (huart1.RxState == HAL_UART_STATE_READY)  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
			if (checksum_failed){
				HAL_UART_Transmit_IT(&huart1,&tx_byte,1); //re-sending start command
			}
			osDelay(pdMS_TO_TICKS(50));
		}

		if (checksum_failed){ //send request after sending start command and receiving received command
			tx_byte = cur_service;
			HAL_UART_Transmit_IT(&huart1,&tx_byte,1); //retry request
		}
	}

	GUI_Set_PIDs(cur_pid,NULL,pid_bitmask, 0);
	vTaskDelay(pdMS_TO_TICKS(10));
}

/**
 * Available PIDs bit-mask grabbed by Get_PID_BitMask() above and then this function waits for PIDs through UART to update PID data.
 * Display starts with start command
 *
 * UART Protocol Overview
 * ======================
 *
 * From Gateway (to Display):
 *   Byte 0   : Receive Command
 *   Byte 1-28: PIDs Supported Bitmask [7][4] (28 bytes)
 *   Byte 29  : Checksum (display repeat req if failed)
 *   Byte 30+ : Repeated blocks of:
 *              - Byte N    : Num of bytes for PID data
 *              - Byte N+1  : Data for PID
 *              - Byte N+2  : Checksum (display repeat req if failed)
 *
 * From Display (to Gateway):
 *   Byte 0   : Start Command (e.g. 0x20)
 *   Byte 1   : UART_PIDS Request
 *   Byte 2+  : Sequence of PIDs requested (1 byte each)
 *   Final    : 0x20 sent
 */
static void get_PID_Value(uint8_t data){
	uint8_t checksum = 0;
	int i = -1; //index
	int err_count = 0;

	while(1){

		if (resetFlag) {
			osSemaphoreRelease(SERV_DONE_sem); //ready for next service
			return;
		}

		if (cur_pid == 0x20){ //exit condition: PID = 0x20 and UART_Received_cmd read by UART
			if (rx_byte == UART_Received_cmd){
				retry_last = false;
				osSemaphoreRelease(SERV_DONE_sem);
				return;
			}
		}

		if (i == -1){
			if (data > 0 && pid_bytes != data){ //if the sizes are the same just overwrite pointer
				ptrFree((void **) &PID_VALUE);
				PID_VALUE = (uint8_t *)pvPortMalloc(data);
				if (!PID_VALUE) Error_Handler();        //kills UART thread

				pid_bytes = data; //set global size
			}

		}else if (i < pid_bytes){
			PID_VALUE[i] = data;
			checksum += data;

		}else{ //checksum checking and dealing with restart or skiping
			checksum = ~checksum;

			if (checksum == data){															//done, correct checksum
				break;

			}else if (err_count >= 1){          											//skipping
				ptrFree((void**)&PID_VALUE);
				PID_VALUE = (uint8_t *)pvPortMalloc(1);
				if (!PID_VALUE) Error_Handler();	//kills UART thread

				PID_VALUE[0] = 0;
				pid_bytes = 1;
				break;

			}else{																			//retrying
				err_count ++;
				checksum = 0;
				i = -2;
				ptrFree((void**)&PID_VALUE);
				HAL_UART_Transmit_IT(&huart1,&cur_pid,1);//request current PID data again

			}
		}

		i ++;

		if (huart1.RxState == HAL_UART_STATE_READY)  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
		while(osMessageQueueGet(receive_task_queue, &data, NULL,pdMS_TO_TICKS(500)) != osOK){

			if (resetFlag) {
				break;
			}

			if (huart1.RxState == HAL_UART_STATE_READY)  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
			osThreadYield();
		}

	}

	GUI_Set_PIDs(cur_pid,PID_VALUE,NULL, pid_bytes);
	vTaskDelay(pdMS_TO_TICKS(10));
}

/**
 * --------------------------------------------------------------Callback functions for UART---------------------------------------------------------------------------------
 */

/**
 * UART Callback functions
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == USART1){
	}

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1){
		osMessageQueuePut(receive_task_queue,&rx_byte, 0,0);
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


/**
 * --------------------------------------------------------------UART functions---------------------------------------------------------------------------------
 */

/**
 * Queues up UART actions from GUI thread
 */
void UART_Set_Service(uart_comms_t ser, int pid){
	Service_Request_t tempReq;

	tempReq.service = ser; //mode and service are interchangeable terms
	tempReq.pid = pid;

	if (ser == UART_end_of_cmd || (ser == UART_DATA_PID && pid == 0x20)){
		osMessageQueueReset(control_queue);
		resetFlag = true; //end current tasks
	}

	if (osMessageQueuePut(control_queue, &tempReq, 0, 0) != osOK){

	}
}

//used to queue up action sequences on UART (mainly for PID request)
static void UART_CONTROl(){
	uint8_t byte;
	Service_Request_t ServReq;

	while (1){

		if (osMessageQueueGet(control_queue,&ServReq,NULL,osWaitForever) == osOK){
			resetFlag = false;
			retry_last = false;

			//clear dtcs_list if not NULL already
			if (dtcs_list != NULL){
				for (int i = 0; i < dtcs_size; i ++){
					ptrFree((void **) &dtcs_list[i]); //free and NULL pointer
				}
				ptrFree((void **) &dtcs_list);
			}

			pid_bytes = -1;            // = number of bytes in pid value
			dtcs_size = -1;
			cur_dtcs_list_size = 0;
			cur_service = ServReq.service;
			cur_pid = ServReq.pid;     //PID 0x20 = exit PID data sequence

			if (cur_service == UART_end_of_cmd || (cur_service == UART_DATA_PID && cur_pid == 0x20)){ //exiting task

				//resetting queues
				osMessageQueueReset(send_task_queue);
				osMessageQueueReset(receive_task_queue);

				if (huart1.gState != HAL_UART_STATE_READY || huart1.RxState != HAL_UART_STATE_READY){
					HAL_UART_Abort(&huart1);

					if (cur_service != UART_DATA_PID){
						HAL_UART_Receive_IT(&huart1,&rx_byte, 1); 											//receive receive command
						byte = UART_end_of_cmd;
						osMessageQueuePut(send_task_queue, &byte, 0, 0);
					}
				}

				if (cur_pid == 0x20){ 																		//for PID data exit
					HAL_UART_Receive_IT(&huart1,&rx_byte, 1); 												//receive receive command
					byte = UART_DATA_PID; 																	//TX will grab cur_pid
					osMessageQueuePut(send_task_queue, &byte, 0, 0); 										//send PID

				}

			}else if (cur_service == UART_DTCs_Reset_cmd || cur_service == UART_DATA_PID) { //reseting DTCs
				retry_last = false;
				HAL_UART_Receive_IT(&huart1,&rx_byte, 1);
				byte = cur_service;
				osMessageQueuePut(send_task_queue, &byte, 0, 0);

			}else{	//all other services
				UART_Reset();
				osDelay(pdMS_TO_TICKS(15));
				byte = UART_Start_cmd;
				osMessageQueuePut(send_task_queue, &byte, 0, 0);

			}

			//checking if UART is running and reset happening
			if (cur_service ==  UART_end_of_cmd && huart1.gState == HAL_UART_STATE_READY && huart1.RxState == HAL_UART_STATE_READY ){
				osSemaphoreRelease(SERV_DONE_sem);

			}

			osSemaphoreAcquire(SERV_DONE_sem, portMAX_DELAY); //wait till service is done

		}

		cur_service = UART_SERVICE_IDLE;

	}

}

/*
 * Note: see UART_RX() for expected command order.
 *
 */
static void UART_TX(){
	uart_comms_t tx_action;
	uint8_t byte;
	osStatus_t queue_get;
	int timeouts = 0;
	while(1){

		queue_get = osMessageQueueGet(send_task_queue, &tx_action, NULL, pdMS_TO_TICKS(550));

		if( retry_last || queue_get == osOK ){

			if (tx_action != UART_DATA_PID){
				byte = tx_action;

			}else{
				byte = cur_pid;

			}

			if (resetFlag){
				retry_last = false;
				continue;
			}

			HAL_UART_Transmit_IT(&huart1, &byte,1);

		}else if (queue_get == osErrorTimeout){
			if (resetFlag){
				retry_last = false;
			}

		}


		//deals with timeout
		if (retry_last){
			timeouts ++;

			if (timeouts <= 3){
				UART_Reset();

			}else if (timeouts > 3){
				retry_last = false;
				osSemaphoreRelease(SERV_DONE_sem);

			}

			if (huart1.RxState != HAL_UART_STATE_BUSY_RX) HAL_UART_Receive_IT(&huart1,&byte,1);

		}else if (timeouts != 0) {
			timeouts = 0;
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
	osStatus_t rx_Queue_Get;
	bool Serv_Bytes = false; //expecting bytes for Service (non command)

	while(1){
		rx_Queue_Get = osMessageQueueGet(receive_task_queue, &rx_byte, NULL,pdMS_TO_TICKS(500));
		retry_last = false;

		if(rx_Queue_Get == osOK && rx_byte != UART_end_of_cmd ){ //end command sent by display to display no padding

			if (cur_service == UART_DATA_PID){ //PIDs coming without commands
				get_PID_Value(rx_byte);
				osSemaphoreRelease(SERV_DONE_sem); //ready for next service
				continue;

			}else if (Serv_Bytes && (cur_service == UART_PIDS_LIVE || cur_service == UART_PIDS_FREEZE)){
				Get_PID_BitMask();
				Serv_Bytes = false;
				osSemaphoreRelease(SERV_DONE_sem); //ready for next service
				continue;

			}else if (Serv_Bytes && (cur_service == UART_DTCs_REQ_STORED_cmd
				|| cur_service == UART_DTCs_REQ_PENDING_cmd || cur_service == UART_DTCs_REQ_PERM_cmd)){

				Read_Codes();
				Serv_Bytes = false;
				osSemaphoreRelease(SERV_DONE_sem); //ready for next service

			}else if (rx_byte == UART_Received_cmd){
				UART_Reset(); //stop outgoing start commands

				if (cur_service == UART_DTCs_Reset_cmd) {
					Serv_Bytes = false;
					retry_last = false;
					osSemaphoreRelease(SERV_DONE_sem);
					continue;

				}else if (cur_service == UART_end_of_cmd){
					osSemaphoreRelease(SERV_DONE_sem); //ready for next service
					continue;

				}else{
					byte = cur_service;
				}

				Serv_Bytes = true;								//code is in this read function
				HAL_UART_Receive_IT(&huart1,&rx_byte,1);
				osMessageQueuePut(send_task_queue, &byte, 0, osWaitForever);

			}



		}else if (rx_Queue_Get == osErrorTimeout) { //Timeout*****************************************************

			if (resetFlag){
				retry_last = false;
				Serv_Bytes = false;
				osSemaphoreRelease(SERV_DONE_sem);
			}else if (cur_service != UART_SERVICE_IDLE){
				retry_last = true;

			}

		}

	}

}

/*
 * Function Description: Used to start and initiate UART.
 *
 */
void UART_INIT(){

	osDelay(pdMS_TO_TICKS(150)); //small delay for GUI setup
	//setting up FreeRTOS*********************************************************************************
	SERV_DONE_sem = osSemaphoreNew(1,0,NULL);

	send_task_queue = osMessageQueueNew(3, sizeof(uart_comms_t),NULL);
	init_error_check(send_task_queue);

	receive_task_queue = osMessageQueueNew(1, sizeof(uart_comms_t),NULL);
	init_error_check(receive_task_queue);

	control_queue = osMessageQueueNew(25, sizeof(Service_Request_t),NULL);
	init_error_check(control_queue);

	receive_handle = osThreadNew(UART_RX, NULL, &receiveTask_attributes);
	init_error_check(receive_handle);

	send_handle = osThreadNew(UART_TX, NULL, &sendTask_attributes);
	init_error_check(send_handle);

	control_handle = osThreadNew(UART_CONTROl, NULL, &controlTask_attributes);
	init_error_check(control_handle);

	blink_handle = osThreadNew(blk_toggle_led,NULL,&blinkTask_attributes); //testing only (blink.c and blink.h are both for testing)
	init_error_check(blink_handle);

	DTCList_MUT = osMutexNew(NULL);

	osThreadExit();
}


