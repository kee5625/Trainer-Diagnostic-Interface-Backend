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
    SERV_LD_DATA       = 1,
    SERV_FREEZE_DATA    = 2,
    SERV_STORED_DTCS    = 3,
    SERV_CLEAR_DTCS     = 4,
    SERV_CUR_DATA       = 5,
    SERV_PENDING_DTCS   = 7,
    SERV_PERM_DTCS      = 10,
    //more to be added later
} service_request_t;

extern SemaphoreHandle_t TC_Recieved_sem;
extern SemaphoreHandle_t TWAI_GRAB_TC_sem;
extern SemaphoreHandle_t DTCS_Loaded_sem;
extern QueueHandle_t service_queue;

void TC_Code_set(uint8_t *codes, int num_codes);
void DTCS_reset();
void set_serv(service_request_t req);
uint8_t *get_dtcs();
uint8_t get_dtcs_bytes();


#endif