/*
 * USART.h
 *
 *  Created on: Jun 12, 2025
 *      Author: nbatcher
 */

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_uart.h"
#include "cmsis_os.h"


#ifndef APPLICATION_USER_USART_USART_H_
#define APPLICATION_USER_USART_USART_H_

extern osSemaphoreId_t blink_sem;
extern UART_HandleTypeDef huart1;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void send_uart();
void receive_uart();
void USART_Init_Start();



#endif /* APPLICATION_USER_USART_USART_H_ */
