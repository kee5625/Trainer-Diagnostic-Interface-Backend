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

static char trouble_code[tc_size + 2];
static char TC_code[7];
SemaphoreHandle_t TC_Recieved_sem;

//convert uint8_t[2] buff for trouble code to char [7]
char* TC_buff_conv_char(uint8_t TC_buff[2]){
    const char prefix_lookup[4] = {'P', 'C', 'B', 'U'};

    //converting TC letter
    TC_code[0] = prefix_lookup[(TC_buff[0] >> 6) & 0x03];    

    //converting TC number within category
    TC_code[1] = ((TC_buff[0] >> 4) & 0x03) + '0'; 
    TC_code[2] = ((char)TC_buff[0] & 0x0F) + '0';
    TC_code[3] = ((char)(TC_buff[1] >> 4) & 0x0F) + '0';
    TC_code[4] = ((char)TC_buff[1] & 0x0F) + '0';
    TC_code[5] = '\n';
    ESP_LOGI("main", "TC = %s", TC_code);
    return TC_code;
}

void TC_Code_set(char TC_code[tc_size + 2]){
    memcpy(trouble_code, TC_code,sizeof(trouble_code));
}

char* TC_Code_Get(){
    return trouble_code;
}

void new_tc_tasks(){
    //grabbing trouble code from twia network and putting in trouble_code_buff
    twai_TC_Get();
    
    //running bt to send trouble code to serial port
    //bt_spp_setup();
}

void app_main(void)
{  
    TC_Recieved_sem = xSemaphoreCreateBinary();
    twai_TC_Get();
    xSemaphoreTake(TC_Recieved_sem, portMAX_DELAY);
    //start and running UART to send trouble code over uart
    UART_INIT(trouble_code);
}
