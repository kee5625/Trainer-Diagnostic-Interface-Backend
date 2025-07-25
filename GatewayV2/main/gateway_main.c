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

#include "TWIA_TC.h"
#include "TC_ref.h"
#include "ble.h"
/* --------------------- Definitions and static variables ------------------ */
#define IF_BIT_SET(byte,bit)  ((byte) & (1<< (bit)))
#define IF_BIT_RESET(byte,bit) (!((byte) & (1<< (bit))))

volatile bool stream_on_master = false;

static uint8_t *dtcs;
static uint8_t dtcs_bytes;
SemaphoreHandle_t TC_Recieved_sem;
SemaphoreHandle_t TWAI_GRAB_TC_sem;
SemaphoreHandle_t DTCS_Loaded_sem;
QueueHandle_t service_queue;


void DTCS_reset(){
    dtcs = NULL;
    dtcs_bytes = 0;
}

void TC_Code_set(uint8_t *codes, int codes_bytes){
    dtcs = pvPortMalloc(codes_bytes);
    dtcs_bytes = codes_bytes;
    if (codes != NULL){
        memcpy(dtcs,codes,codes_bytes);
    }else{
        dtcs = NULL;
        dtcs_bytes = 0;
    }
}

void set_serv(service_request_t req){
    
    switch(req){
        case SERV_LD_DATA:
        case SERV_FREEZE_DATA:
        case SERV_STORED_DTCS:
        case SERV_CLEAR_DTCS: /*fall through*/
            DTCS_reset();
        case SERV_PENDING_DTCS:
        case SERV_PERM_DTCS:
            xQueueSend(service_queue, &req, portMAX_DELAY);
            xSemaphoreTake(DTCS_Loaded_sem, portMAX_DELAY);
            
        default:
            break;
    }
}

uint8_t *get_dtcs(){ //Made to have 1 D pointer to all values that can send through UART
    return (uint8_t *)dtcs;
}

uint8_t get_dtcs_bytes(){
    return dtcs_bytes;
}

void app_main(void)
{  
    ESP_ERROR_CHECK(nvs_flash_init());

    BLE_init();

    service_queue = xQueueCreate(3, sizeof(service_request_t));
    DTCS_Loaded_sem = xSemaphoreCreateBinary();
    twai_TC_Get();

    //UART_INIT();
}