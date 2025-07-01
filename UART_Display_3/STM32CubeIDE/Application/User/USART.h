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

typedef enum {
    TX_SEND_PINGS,
    TX_SEND_START_CMD,
	TX_Reset_TC,
    TX_TASK_EXIT,
} tx_task_action_t;

typedef enum {
    RX_RECEIVE_PING_RESP,
    RX_RECEIVE_DATA,
	RX_TC_Cleared,
    RX_TASK_EXIT,
} rx_task_action_t;

typedef enum {
	Get_TC_Phase,
	Reset_TC_Phase,
	Clean_Up_Phase,
}Action_phase_t;

extern UART_HandleTypeDef huart1;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void send_uart();
void receive_uart();
void init_error_check(void *ptr);

void Get_TC_USART();


#endif /* APPLICATION_USER_USART_USART_H_ */
