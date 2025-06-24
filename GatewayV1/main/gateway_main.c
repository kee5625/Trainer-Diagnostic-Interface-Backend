/**
 * Coder: Noah Batcher
 * Last updated: 6/11/2025
 * Project: Trainer Fault Code Diagnostic Gatway.
 * Note: 
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
#include "C:\Users\krachamolla\Trainer Diagnostic interface\Gateway_Slave\main\trouble_codes.c"
#include "BT_SPP_TC.h"
#include "TWIA_TC.h"
//#include "UART_TC.h"
#include "ble.c"
/* --------------------- Definitions and static variables ------------------ */
#define TC_size 22  //trouble code size
char trouble_code_buff[TC_size];


void app_main(void)
{
    //ble_connect();
    //grabbing trouble code from twia network and putting in trouble_code_buff
    twai_TC_Get();

    //running bt to send trouble code to serial port
    //bt_spp_setup();

    //start and running UART to send trouble code over uart
    //uart_start();

    // twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT(21, 22, TWAI_MODE_NORMAL);
    // twai_timing_config_t  t = TWAI_TIMING_CONFIG_25KBITS();
    // twai_filter_config_t  f = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // twai_driver_install(&g,&t,&f);
    // twai_start();

    // twai_message_t ping = {
    //     .identifier = 0x0A2,
    //     .data_length_code = 0,
    //     .self = 1
    // };

    // while (1) {
    //     twai_transmit(&ping, portMAX_DELAY);              // send
    //     twai_message_t rx;
    //     esp_err_t res = twai_receive(&rx, pdMS_TO_TICKS(10));   // expect own frame back
    //     printf("Loopback result: %s\n", res == ESP_OK ? "OK" : "FAIL");
    //     vTaskDelay(pdMS_TO_TICKS(500));
    // }
    

}
