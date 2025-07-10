#ifndef uart_send
#define uart_send

#define uart_start_pad 1
#define uart_end_pad 6

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


void UART_Start(void);

#endif