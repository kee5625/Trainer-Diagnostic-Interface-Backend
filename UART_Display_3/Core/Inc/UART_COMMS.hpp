/*
 * UART_COMMS.hpp
 *
 *  Created on: Jul 17, 2025
 *      Author: nbatcher
 */

#ifndef UART_COMMS_HPP_
#define UART_COMMS_HPP_

#ifdef __cplusplus
extern "C" {
#endif

// Example: Diagnostic command codes
typedef enum {
    UART_Start_cmd             = 1,
	UART_Received_cmd          = 2,
	UART_Retry_cmd   		   = 3,
    UART_DTCs_REQ_STORED_cmd   = 4,    //stored dtcs
	UART_DTCs_REQ_PENDING_cmd  = 5,    //pending dtcs
	UART_DTCs_REQ_PERM_cmd     = 6,    //perminate dtcs
	UART_DTCs_REQ_LD_cmd       = 7,	   //Live data
	UART_DTCs_REQ_FFD_cmd 	   = 8,    //Freeze frame data
    UART_DTC_Received_cmd     = 9,
	UART_SERVICE_RUNNING       = 10,   //used for switch statement to start grabbing DTCs in display code
	UART_DTC_next_cmd         = 11,
	UART_DTCs_End_cmd          = 12,
    UART_DTCs_Reset_cmd        = 13,
	UART_end_of_cmd            = 14,
    UART_stop_cmd              = 15,
    UART_DTCs_Num_cmd          = 16,
	UART_CMD_MAX,
}uart_comms_t;

#ifdef __cplusplus
}
#endif

#endif
