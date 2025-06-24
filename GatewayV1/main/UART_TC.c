/**
 * Coder: Noah Batcher
 * Last updated: 6/11/2025
 * Project: Trainer Fault Code Diagnostic Gatway
 * Note: Code used to send trouble code through UART
 * 
 */

#include <stdio.h>
#include "string.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "UART_TC.h"

#define ECHO_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define ECHO_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)
#define UART_Priority 5 //twai ones are set to 8-10 (TWAI and UART do not run at the same time)
#define BUF_SIZE (1024)

#define UART_PORT_NUM      (CONFIG_EXAMPLE_UART_PORT_NUM)
#define ECHO_UART_BAUD_RATE     9600//(CONFIG_EXAMPLE_UART_BAUD_RATE)
#define ECHO_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)
#define CONFIG_UART_ISR_IN_IRAM false



QueueHandle_t uart_queue;
static const char *TAG = "UART TEST";

//sends fault code over uart
static void uart_send_tc(){
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    int intr_alloc_flags = 0; //sets interrupt if 1

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE, BUF_SIZE, 10, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    for(;;){
        uart_write_bytes(UART_PORT_NUM, trouble_code_buff, strlen(trouble_code_buff));
        uart_wait_tx_done(UART_PORT_NUM, 1000 / portTICK_PERIOD_MS); //waiting 0-10s for send to go through
        vTaskDelay(pdMS_TO_TICKS(500));
        ESP_LOGI(TAG,"Sending trouble code %s.", trouble_code_buff);
    }
}

void uart_start(void)
{   
    xTaskCreate(uart_send_tc, "uart_send_tc_task", ECHO_TASK_STACK_SIZE, NULL, UART_Priority, NULL);
}
