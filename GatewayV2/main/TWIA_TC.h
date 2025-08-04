

#ifndef TWIA_TC
#define TWIA_TC

#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <inttypes.h>
#include "TC_ref.h"


//TWAI action type setup
typedef enum {
    TX_REQUEST_PIDS                     = 0,
    TX_REQUEST_DATA                     = 1,
    TX_REQUEST_FFD                      = 2,
    TX_REQUEST_STORED_DTCS              = 3,
    TX_REQUEST_PENDING_DTCS             = 4,
    TX_REQUEST_PERM_DTCS                = 5,
    TX_RESET_DTCs                       = 6,
    TX_FLOW_CONTROL_RESPONSE            = 7,
    TX_TASK_EXIT                        = 8,
} tx_task_action_t;

//funcitons
void TWAI_INIT();
void TWAI_RESET(service_request_t req);

uint8_t* get_DTCs_buffer(void);
uint8_t get_DTCs_length(void);
uint8_t* get_bitmask_row(int row);
uint8_t* get_live_data_buffer(void);
uint8_t get_live_data_length(void);

#endif