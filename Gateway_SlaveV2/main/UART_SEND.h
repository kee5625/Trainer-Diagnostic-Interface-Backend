#ifndef uart_send
#define uart_send

#define uart_start_pad 1
#define uart_end_pad 6

/**
 * Command expected order and packing for general request(others will be defined above their respective function).
 * See other read me for entire project for more info
 * 
 * From Display      				:     				From Gateway
 * ***************************************************************************
 * UART_Start_cmd(start) 			:     				UART_Received_cmd
 * UART_Retry_cmd(start)			:     				Resend data
 * Resend request    				:     				UART_Retry_cmd(start)
 * UART_end_of_cmd(start)			: 					UART_Received_cmd
 */


typedef enum {
	//general UART commands
    UART_Start_cmd             = 1,
	UART_Received_cmd          = 2,
	UART_Retry_cmd   		   = 3,

	//DTC service commands
    UART_DTCs_REQ_STORED_cmd   = 4,    //service/mode 3 stored dtcs
	UART_DTCs_REQ_PENDING_cmd  = 5,    //service/mode 7 pending dtcs
	UART_DTCs_REQ_PERM_cmd     = 6,    //service/mode A perminate dtcs
	UART_DTC_next_cmd          = 7,	   //tells TX for UART to send next dtc(can probably change to UART_DTC_Received_cmd)
	UART_DTC_Received_cmd      = 8,	   //1 dtc received ready for next
	UART_DTCs_End_cmd          = 9,	   //All dtcs sent(uneeded because num dtcs sent before dtcs)
    UART_DTCs_Reset_cmd        = 10,   //service/mode 4 reset/clear dtcs
	UART_DTCS_Num_cmd          = 11,   //tells UART to send number of DTCs(not needed because could just have function send number first)

	//Data service commands for Live and Freeze frame data
	UART_PIDS_LIVE    		   = 14, //service/mode 1: grabs PIDs available bit-mask for live data
	UART_PIDS_FREEZE		   = 15, //service/mode 2: grabs PIDS available bit-mask for freeze frame data
	UART_DATA_PID 			   = 16, //grab individual PID data

	//More general UART commands
	UART_SERVICE_RUNNING       = 17,   //used for switch statement to start grabbing DTCs in display code
	UART_end_of_cmd            = 18,   //used to reset UART, TWAI, everything from GUI input
	UART_SERVICE_IDLE          = 19,   //used for timeouts when idle
	UART_CMD_MAX,
}uart_comms_t;


void UART_Start(void);

#endif