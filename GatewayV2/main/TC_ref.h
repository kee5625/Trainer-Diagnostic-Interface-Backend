#ifndef TC
#define TC

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#define tc_size 5

#define ERROR_TIMEOUT   -1

typedef enum {
    SERV_PIDS_LIVE      = 0, //PID bitmask live data
    SERV_PIDS_FREEZE    = 1, //PID bitmask freeze frame data
    SERV_DATA           = 2, //individual PID value
    SERV_STORED_DTCS    = 4,
    SERV_CLEAR_DTCS     = 5,
    SERV_PENDING_DTCS   = 6,
    SERV_PERM_DTCS      = 7,
    TWAI_ERROR          = 10,
    //more to be added later
} service_request_t;

extern SemaphoreHandle_t TWAI_DONE_sem;
extern QueueHandle_t service_queue;

void Set_Req_PID(int PID);
void Set_DTCs(uint8_t *codes, int num_codes);
void Set_PID_Bitmask(uint8_t bitmask[7][4]);
void Set_PID_Value(uint8_t *data,int num_bytes);
void Reset_TWAI_QUEUE();
int Set_TWAI_Serv(service_request_t req);                   //returns -1 if error

uint8_t *get_bitmask_row(int row);
uint8_t *get_dtcs();
uint8_t get_dtcs_bytes();
uint8_t get_Req_PID();
uint8_t get_Cur_Serv();


#endif