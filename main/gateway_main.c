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
/* --------------------- Definitions and static variables ------------------ */
#define TC_size 22  //trouble code size
char trouble_code_buff[TC_size];


void app_main(void)
{
    //grabbing trouble code from twia network and putting in trouble_code_buff
    twai_TC_Get();

    //running bt to send trouble code to serial port
    //bt_spp_setup();

    //start and running UART to send trouble code over uart
    uart_start();

    
}
