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

  typedef struct {
        int service;
        int pid;
    }Service_Request_t;


extern UART_HandleTypeDef huart1;

void UART_INIT();


#endif /* APPLICATION_USER_USART_USART_H_ */
