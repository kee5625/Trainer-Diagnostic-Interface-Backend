#ifndef uart_TC
#define uart_TC

#include <inttypes.h>
#include "TC_ref.h"

#define uart_start_pad 1
#define uart_end_pad 6

typedef enum {
	//general UART commands
    UART_Start_cmd             = 1,
	UART_Received_cmd          = 2,
	UART_Retry_cmd   		   = 3,

	//DTC service commands
    UART_DTCs_REQ_STORED_cmd   = 4,    //stored dtcs
	UART_DTCs_REQ_PENDING_cmd  = 5,    //pending dtcs
	UART_DTCs_REQ_PERM_cmd     = 6,    //perminate dtcs
	UART_DTC_next_cmd          = 7,
	UART_DTC_Received_cmd      = 8,
	UART_DTCs_End_cmd          = 9,
    UART_DTCs_Reset_cmd        = 10,
	UART_DTCS_Num_cmd          = 11,

	//Data service commands for Live and Freeze frame data
	UART_PIDS    		       = 14, //grab PIDs bit-mask of available PIDs
	UART_DATA_PID 			   = 15, //grab individual PID

	//More general UART commands
	UART_SERVICE_RUNNING       = 16,   //used for switch statement to start grabbing DTCs in display code
	UART_end_of_cmd            = 17,
    UART_stop_cmd              = 18,
	UART_CMD_MAX,
}uart_comms_t;

void UART_INIT();
void UART_PID_VALUE(uint8_t *data,int num_bytes);

#endif