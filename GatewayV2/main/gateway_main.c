/**
 * Coder: Noah Batcher
 * Last updated: 6/11/2025
 * Project: Trainer Fault Code Diagnostic Gatway
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "time.h"
#include "sys/time.h"

#include "BT_SPP_TC.h"
#include "TWIA_TC.h"
#include "UART_TC.h"
#include "TC_ref.h"
/* --------------------- Definitions and static variables ------------------ */
#define IF_BIT_SET(byte,bit)  ((byte) & (1<< (bit)))
#define IF_BIT_RESET(byte,bit) (!((byte) & (1<< (bit))))

//static copies
static uint8_t *dtcs;
static uint8_t dtcs_bytes;
static uint8_t supported_Bitmask[7][4];

uint8_t req_PID = 0;

SemaphoreHandle_t TWAI_DONE_sem;
QueueHandle_t service_queue;


void DTCS_reset(){
    dtcs = NULL;
    dtcs_bytes = 0;
}

void Set_DTCs(uint8_t *codes, int codes_bytes){
    dtcs = pvPortMalloc(codes_bytes);
    dtcs_bytes = codes_bytes;
    if (codes != NULL){
        memcpy(dtcs,codes,codes_bytes);
    }else{
        dtcs = NULL;
        dtcs_bytes = 0;
    }
}

//
void Set_Req_PID(int PID){
    req_PID = PID;
}

void Set_PID_Bitmask(uint8_t bitmask[7][4]){
    memcpy(supported_Bitmask, bitmask, sizeof(supported_Bitmask));
}

void Set_PID_Value(uint8_t *data,int num_bytes){
    UART_PID_VALUE(data,num_bytes);
}

//controlls TWAI based on req
void Set_TWAI_Serv(service_request_t req){
    
    switch(req){
        case SERV_PIDS:
        case SERV_DATA:
        case SERV_FREEZE_DATA:
        case SERV_STORED_DTCS:
        case SERV_CLEAR_DTCS: //intentional fall through
            DTCS_reset();
        case SERV_PENDING_DTCS:
        case SERV_PERM_DTCS:
            xQueueSend(service_queue, &req, portMAX_DELAY);
            xSemaphoreTake(TWAI_DONE_sem, portMAX_DELAY);
        
        default:
            break;
    }
}


uint8_t *get_bitmask_row(int row){
    return supported_Bitmask[row]; //single row of bitmask
}

uint8_t *get_dtcs(){ //Made to have 1 D pointer to all values that can send through UART
    return (uint8_t *)dtcs;
}

uint8_t get_dtcs_bytes(){
    return dtcs_bytes; //total number of bytes for all dtcs (1 dtcs = 2 bytes)
}

uint8_t get_Req_PID(){
    return req_PID;
}

void app_main(void)
{  
    service_queue = xQueueCreate(3, sizeof(service_request_t));
    TWAI_DONE_sem = xSemaphoreCreateBinary();
    TWAI_INIT();
    UART_INIT();
}
