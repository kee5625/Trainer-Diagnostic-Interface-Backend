#ifndef uart_send
#define uart_send

#define uart_start_pad 1
#define uart_end_pad 6

typedef enum {
    UART_Start_cmd             = 1,
	UART_Received_cmd          = 2,
	UART_Retry_cmd   		   = 3,
    UART_DTCs_REQ_STORED_cmd   = 4,    //stored dtcs
	UART_DTCs_REQ_PENDING_cmd  = 5,    //pending dtcs
	UART_DTCs_REQ_PERM_cmd     = 6,    //perminate dtcs
	UART_DTCs_REQ_LD_cmd       = 7,	  //Live data
	UART_DTCs_REQ_FFD_cmd 	   = 8,    //Freeze frame data
    UART_DTCs_Received_cmd     = 9,
	UART_DTCs_Receiving        = 10,   //used for switch statement to start grabbing DTCs in display code
	UART_DTCs_next_cmd         = 11,
	UART_DTCs_End_cmd          = 12,
    UART_DTCs_Reset_cmd        = 13,
	UART_end_of_cmd            = 14,
    UART_stop_cmd              = 15,
    UART_DTCs_Num_cmd          = 16,
	UART_CMD_MAX,
}uart_comms_t;


void UART_Start(void);

#endif