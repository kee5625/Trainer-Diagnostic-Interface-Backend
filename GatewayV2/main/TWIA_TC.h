

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
    TX_REQUEST_STORED_DTCS,
    TX_REQUEST_PENDING_DTCS,
    TX_REQUEST_PERM_DTCS,
    TX_FLOW_CONTROL_RESPONSE,
    TX_TASK_EXIT,
} tx_task_action_t;

//funcitons
void twai_TC_Get();
void can_request_task(void *arg);

#endif