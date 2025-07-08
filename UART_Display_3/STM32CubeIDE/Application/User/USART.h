/*
 * USART.h
 *
 *  Created on: Jun 12, 2025
 *      Author: nbatcher
 */

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_uart.h"
#include "cmsis_os.h"
#include "stdbool.h"


#ifndef APPLICATION_USER_USART_USART_H_
#define APPLICATION_USER_USART_USART_H_

extern UART_HandleTypeDef huart1;

typedef enum {
    start_cmd = 1,
	received_cmd,
	retry_cmd,
    TC_Req_cmd,
    TC_Received_cmd,
	TC_Receiving,
    TC_Reset_cmd,
    Read_live_cmd,
	end_of_cmd,
    stop_cmd,
	Exit_Task_cmd,
}uart_comms_t;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void init_error_check(void *ptr);
void UART_REST_TC();
void Get_TC_USART();


#endif /* APPLICATION_USER_USART_USART_H_ */
