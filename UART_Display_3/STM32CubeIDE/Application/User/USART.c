/*
 * USART.c
 *
 *  Created on: Jun 12, 2025
 *      Author: nbatcher
 */
#include "stdio.h"
#include "stdlib.h"
#include "main.h"
#include "string.h"
#include "stdbool.h"
#include <ctype.h>
#include "USART.h"
#include "cmsis_os.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_uart.h"
#include "blinking.h"
#include "..\..\STM32CubeIDE\Application\User\TouchGFX\App\TC_Bridge.hpp"

#define tc_size 5
#define buff_size 32


char trouble_code[6];
char data[6];

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

static osMessageQueueId_t  send_task_queue;
static osMessageQueueId_t  receive_task_queue;
static osSemaphoreId_t  receive_code_sem;
Action_phase_t Action_phase;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	if(huart->Instance == USART1){
		if (Action_phase == Get_TC_Phase){
			rx_task_action_t rx_action = RX_RECEIVE_DATA;
			osMessageQueuePut(receive_task_queue,&rx_action, 0,0);
		}else if (Action_phase == Clean_Up_Phase){
			rx_task_action_t rx_action = RX_TASK_EXIT;
			osMessageQueuePut(receive_task_queue,&rx_action, 0,0);
		}else{
			//no action for read in rest_tc_phase
		}
	}


}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == USART1){
		if (Action_phase == Get_TC_Phase){
			tx_task_action_t tx_action = TX_SEND_START_CMD;
			osMessageQueuePut(send_task_queue,&tx_action, 0,0);
		}else if (Action_phase == Reset_TC_Phase){
			tx_task_action_t tx_action = TX_Reset_TC;
			osMessageQueuePut(send_task_queue, &tx_action,0,0);
		}else if (Action_phase == Clean_Up_Phase){
			tx_task_action_t tx_action = TX_TASK_EXIT;
			osMessageQueuePut(send_task_queue,&tx_action, 0,0);
		}
	}
}


void send_uart(){
	osDelay(50);//small delay for USART initialization
	tx_task_action_t action;
	while(1){
		if(osMessageQueueGet(send_task_queue, &action, NULL, osWaitForever) == osOK){
			if (action == TX_SEND_START_CMD) {
				HAL_UART_Transmit_IT(&huart1, (const uint8_t *)"Send trouble code\n",strlen("Send trouble code\n"));
				osDelay(pdMS_TO_TICKS(5000));
			}else if (action == TX_Reset_TC){
				HAL_UART_Transmit_IT(&huart1, (const uint8_t *)"Reset TC\n",strlen("Reset TC\n"));
				osDelay(pdMS_TO_TICKS(1500));
			}else if (action == TX_TASK_EXIT){
				break;
			}
		}
	}
	osMessageQueueDelete(send_task_queue);
	osThreadExit();
}

void receive_uart(){
	osDelay(50); //small delay for USART to initialize
	rx_task_action_t action;
	while(1){
		if(osMessageQueueGet(receive_task_queue, &action, NULL,osWaitForever) == osOK){
			if (action == RX_RECEIVE_DATA){
				//setting trouble_codes
				if (isalpha(data[0])&& data[tc_size] == '\n' && (data[0] == 'P' || data[0] == 'C' || data[0] == 'B' || data[0] == 'U')){
					osSemaphoreRelease(blink_sem);
					memcpy(trouble_code,data,sizeof(data));
					TC_Received(trouble_code); //pass trouble code to GUI
					break;
				}
			}else if (action == RX_TASK_EXIT){
				break;
			}else if (action == RX_TC_Cleared){
				Action_phase = Clean_Up_Phase;
				break;
			}else{
				HAL_UART_Receive_IT(&huart1,(uint8_t *)data,tc_size + 2);
			}
		}
	}
	//terminating thread
	osSemaphoreRelease(receive_code_sem);
	osMessageQueueDelete(receive_task_queue);
	osThreadExit();

}

void init_error_check(void *ptr){
	if (ptr == NULL){
		Error_Handler();
	}
}

void UART_REST_TC(){
	//waiting till thread is deleted to start again
	while (osThreadGetState(send_handle) != osThreadTerminated){
		osDelay(10);
	}
	osDelay(50); //waiting extra time for setup

	//starting queue and thread
	send_task_queue = osMessageQueueNew(1, sizeof(tx_task_action_t),NULL);
	init_error_check(send_task_queue);
	send_handle = osThreadNew(send_uart, NULL, &sendTask_attributes);
	init_error_check(send_handle);
	//start callback
	HAL_UART_Transmit_IT(&huart1, (const uint8_t *)"Reset TC\n",strlen("Reset TC\n"));
}

void Get_TC_USART()
{

  //setting up FreeRTOS*********************************************************************************
  /**
   * Setup and error checking. To load next trouble code call function again.
   */
  Action_phase = Get_TC_Phase;
  send_task_queue = osMessageQueueNew(1, sizeof(tx_task_action_t),NULL);
  init_error_check(send_task_queue);
  receive_task_queue = osMessageQueueNew(1, sizeof(rx_task_action_t),NULL);
  init_error_check(receive_task_queue);
  receive_code_sem = osSemaphoreNew(1,0,NULL);
  init_error_check(receive_code_sem);
  send_handle = osThreadNew(send_uart, NULL, &sendTask_attributes);
  init_error_check(send_handle);
  receive_handle = osThreadNew(receive_uart, NULL, &receiveTask_attributes);
  init_error_check(receive_handle);
  blink_handle = osThreadNew(blk_toggle_led,NULL,&blinkTask_attributes);
  init_error_check(blink_handle);

  //setting interrupts**********************************************************************************
  HAL_UART_Transmit_IT(&huart1, (uint8_t *)"Send trouble code.",strlen("Send trouble code."));
  HAL_UART_Receive_IT(&huart1,(uint8_t *)data, tc_size + 2);

  //waiting for trouble code to be received*************************************************************
  osSemaphoreAcquire(receive_code_sem, HAL_MAX_DELAY);

  //clean up after code received************************************************************************
  Action_phase = Clean_Up_Phase;
  osSemaphoreRelease(blink_sem);
  osDelay(pdMS_TO_TICKS(1000)); //waiting for tasks to exit
  osSemaphoreDelete(receive_code_sem);
  HAL_UART_Abort_IT(&huart1);
  //turning off led if left on from blink_handle
  if (HAL_GPIO_ReadPin(GPIOI,GPIO_PIN_13) == GPIO_PIN_RESET){
  	  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_13, GPIO_PIN_SET);
  }
  osThreadExit();
}


