#ifndef uart_send
#define uart_send

#define uart_start_pad 1
#define uart_end_pad 6

typedef enum {
    start_cmd = 1,
	received_cmd,
	retry_cmd,
    TC_Req_cmd,
    TC_Received_cmd,
    TC_Reset_cmd,
    Read_live_cmd,
	end_of_cmd,
    stop_cmd,
	Exit_Task_cmd,
}uart_comms_t;


void UART_Start(void);

#endif