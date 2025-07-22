

#ifndef TWIA_TC
#define TWIA_TC

#include "driver/twai.h"
#include "TWIA_TC.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <inttypes.h>


//TWAI action type setup
typedef enum {
    TX_REQUEST_LD                       = 1,
    TX_REQUEST_FFD                      = 2,
    TX_REQUEST_STORED_DTCS              = 3,
    TX_REQUEST_PENDING_DTCS             = 4,
    TX_REQUEST_PERM_DTCS                = 5,
    TX_RESET_DTCs                       = 6,
    TX_FLOW_CONTROL_RESPONSE            = 7,
    TX_TASK_EXIT                        = 8,
} tx_task_action_t;

//funcitons
void twai_TC_Get();


#endif