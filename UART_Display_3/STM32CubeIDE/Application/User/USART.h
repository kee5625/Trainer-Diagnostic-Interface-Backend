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

extern bool isCleanUp;
extern UART_HandleTypeDef huart1;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void send_uart();
void receive_uart();
void init_error_check(void *ptr);
void USART_Init_Start();



#endif /* APPLICATION_USER_USART_USART_H_ */
