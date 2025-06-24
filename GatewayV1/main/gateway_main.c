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

/*  One-byte queue: every UI “Read DTC” click pushes a byte here  */
QueueHandle_t req_q;

/* ---------- TWAI driver configuration (same as in TWIA_TC.c) ---------- */
static const twai_general_config_t g_config =
TWAI_GENERAL_CONFIG_DEFAULT(21, 22, TWAI_MODE_NORMAL);
static const twai_timing_config_t  t_config = TWAI_TIMING_CONFIG_25KBITS();
static const twai_filter_config_t  f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();


void app_main(void)
{
    //ble_connect();
    //grabbing trouble code from twia network and putting in trouble_code_buff
    //twai_TC_Get();

    //running bt to send trouble code to serial port
    //bt_spp_setup();

    //start and running UART to send trouble code over uart
    //uart_start();

    /* -------- initialise CAN driver once and keep it running ---------- */
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_ERROR_CHECK(twai_start());

    /*  queue and tasks */
    req_q = xQueueCreate(4, sizeof(uint8_t));
    xTaskCreatePinnedToCore(can_request_task, "CAN_req", 4096, NULL, 9, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(ui_task,         "UI",      4096, NULL, 8, NULL, tskNO_AFFINITY);

}
