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

#include "C:\ESP-IDF\Gateway_Slave\main\trouble_codes.c"
#include "BT_SPP_TC.h"
#include "TWIA_TC.h"
#include "UART_TC.h"
#include "TC_ref.h"
/* --------------------- Definitions and static variables ------------------ */
#define IF_BIT_SET(byte,bit)  ((byte) & (1<< (bit)))
#define IF_BIT_RESET(byte,bit) (!((byte) & (1<< (bit))))


static uint8_t (*dtcs)[2];
static uint8_t num_dtcs;
SemaphoreHandle_t TC_Recieved_sem;
SemaphoreHandle_t TWAI_GRAB_TC_sem;
QueueHandle_t service_queue;


void DTCS_reset(){
    dtcs = NULL;
    num_dtcs = 0;
}

void TC_Code_set(uint8_t *codes, int num_codes){
    dtcs = malloc(num_codes);
    num_dtcs = num_codes;
    if (codes != NULL){
        memcpy(dtcs,codes,num_codes);
    }else{
        dtcs = NULL;
    }
}

uint8_t *get_dtcs_flat(){ //Made to have 1 D pointer to all values that can send through UART
    return (uint8_t *)dtcs;
}

uint8_t get_num_dtcs(){
    return num_dtcs;
}

void app_main(void)
{  
    service_queue = xQueueCreate(3, sizeof(service_request_t));
    TWAI_GRAB_TC_sem = xSemaphoreCreateBinary();
    TC_Recieved_sem = xSemaphoreCreateBinary();
    twai_TC_Get();
    xSemaphoreTake(TC_Recieved_sem, portMAX_DELAY);
    //start and running UART to send trouble code over uart
    UART_INIT();
}
