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
    UART_Start_cmd          = 1,
	UART_Received_cmd       = 2,
	UART_Retry_cmd   		= 3,
    UART_TC_Req_cmd         = 4,
    UART_TC_Received_cmd    = 5,
	UART_TC_Receiving       = 6,
	UART_TCs_End_cmd        = 7,
    UART_TC_Reset_cmd       = 8,
    UART_Read_live_cmd      = 9,
	UART_end_of_cmd         = 10,
    UART_stop_cmd           = 11,
	UART_CMD_MAX,
}uart_comms_t;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void init_error_check(void *ptr);
void UART_REST_TC();
void Get_TC_USART();


#endif /* APPLICATION_USER_USART_USART_H_ */
