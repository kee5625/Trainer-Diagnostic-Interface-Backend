

#ifndef TWIA_TC
#define TWIA_TC

#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <inttypes.h>
#include "TC_ref.h"


#define MAX_BITMASK_NUM                 0x20 //0xC8 is the max value from wiki (increment in 0x20)

//TWAI action type setup
typedef enum {
    TX_REQUEST_PIDS_Live                = 0, //could be change to use TWAI_OBD.h commands
    TX_REQUEST_PIDS_Freeze              = 1,
    TX_REQUEST_DATA                     = 2,
    TX_REQUEST_FFD                      = 3,
    TX_REQUEST_STORED_DTCS              = 4,
    TX_REQUEST_PENDING_DTCS             = 5,
    TX_REQUEST_PERM_DTCS                = 6,
    TX_RESET_DTCs                       = 7,
    TX_FLOW_CONTROL_RESPONSE            = 8,
    TX_TASK_EXIT                        = 9,
} tx_task_action_t;

//funcitons
void TWAI_INIT();
void TWAI_RESET(service_request_t req);

#endif