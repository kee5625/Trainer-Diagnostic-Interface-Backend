#ifndef TC
#define TC

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#define tc_size 5

typedef enum {
    SERV_PIDS           = 0, //PID bitmask
    SERV_DATA           = 1, //individual PID value
    SERV_FREEZE_DATA    = 2,
    SERV_STORED_DTCS    = 3,
    SERV_CLEAR_DTCS     = 4,
    SERV_PENDING_DTCS   = 7,
    SERV_PERM_DTCS      = 10,
    TWAI_ERROR          = 11,
    //more to be added later
} service_request_t;

extern SemaphoreHandle_t TWAI_DONE_sem;
extern QueueHandle_t service_queue;
extern uint8_t req_PID;


void Set_Req_PID(int PID);
void Set_DTCs(uint8_t *codes, int num_codes);
void Set_PID_Bitmask(uint8_t bitmask[7][4]);
void Set_PID_Value(uint8_t *data,int num_bytes);
int Set_TWAI_Serv(service_request_t req);

void DTCS_reset();


uint8_t *get_bitmask_row(int row);
uint8_t *get_dtcs();
uint8_t get_dtcs_bytes();
uint8_t get_Req_PID();


#endif