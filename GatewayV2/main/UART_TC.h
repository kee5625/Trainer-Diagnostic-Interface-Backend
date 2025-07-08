#ifndef uart_TC
#define uart_TC

#include <inttypes.h>
#include "TC_ref.h"

#define uart_start_pad 1
#define uart_end_pad 6

typedef enum {
    start_cmd = 1,
	received_cmd,
	retry_cmd,
    TC_Req_cmd,
    TC_Received_cmd,
	TC_Receiving,
    TC_Reset_cmd,
    Read_live_cmd,
	end_of_cmd,
    stop_cmd,
	Exit_Task_cmd,
}uart_comms_t;


void UART_INIT(char tc_pass[tc_size + 2]);

#endif