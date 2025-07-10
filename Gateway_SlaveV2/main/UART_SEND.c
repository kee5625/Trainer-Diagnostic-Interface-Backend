/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "string.h"
#include <esp_err.h>
#include "UART_SEND.h"

/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */

#define ECHO_TEST_TXD (17)
#define ECHO_TEST_RXD (5)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      (2)
#define ECHO_UART_BAUD_RATE     (115200)
#define ECHO_TASK_STACK_SIZE    (3072)

static const char *TAG = "UART TEST";
static QueueHandle_t uart_queue;
static SemaphoreHandle_t TC_rec_sem;
#define BUF_SIZE (1024)

//Slave only for monitoring uart
// static void uart_write_tx(){
//     uint8_t *tx_data = (uint8_t *) malloc(BUF_SIZE);
//     uart_event_t event;

//     if (xQueueReceive(uart_queue, (void *) &event, portMAX_DELAY)){
//         switch (event.type) {
//             case UART_DATA:
//                 // Data received; read from UART
//                 int len = uart_read_bytes(ECHO_UART_PORT_NUM, tx_data, event.size, portMAX_DELAY);
//                 tx_data[len] = '\0';
//                 switch (tx_data){
//                     case TC_Reset_cmd:

//                         break;
//                     default:
                        
//                         break;
//                 }
//                 break;
//             default:
//                 break;
//         }
//     }
// }

static void uart_monitor_rx(){
    uint8_t rx_data;
    char trouble_code[7];
    int count = 0;
    uart_event_t event;
    for (;;){
        if (xQueueReceive(uart_queue, (void *) &event, portMAX_DELAY)){
            switch (event.type) {
                case UART_DATA:
                    for(;;){
                        // Data received; read from UART
                        uart_read_bytes(ECHO_UART_PORT_NUM, &rx_data, 1, portMAX_DELAY);
                        ESP_LOGI(TAG,"%X", rx_data);
                        switch ((rx_data >>3) & 0x0F){
                            case UART_Start_cmd:
                                ESP_LOGI(TAG,"Start command received.");
                                break;
                            case UART_Received_cmd:
                                ESP_LOGI(TAG,"Received command received.");
                                break;
                            case UART_TC_Reset_cmd:
                                break;
                            case UART_end_of_cmd:
                                break;
                            case UART_Read_live_cmd:
                                break;
                            case UART_TC_Received_cmd:
                                ESP_LOGI(TAG,"TC received cmd sent.");
                                break;
                            default:
                                trouble_code[count] = (char)rx_data;
                                count ++;
                                if ( rx_data == '\n'){
                                    trouble_code[5] = '\0';
                                    ESP_LOGI(TAG,"Trouble Code: %s", trouble_code);
                                    count = 0;
                                    break;
                                }
                        }
                        break;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

static void new_tc_task()
{   
    TC_rec_sem = xSemaphoreCreateBinary();
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE, BUF_SIZE, 1, &uart_queue, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t send_comm;
    while (1) {
        // Write data back to the UART
        send_comm = UART_Start_cmd;
        for(;;){
            uart_write_bytes(ECHO_UART_PORT_NUM, &send_comm, sizeof(send_comm));
        }
        vTaskDelay(100);
    }
}

void UART_Start(void)
{
    xTaskCreate(new_tc_task, "new_TC_task", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
    xTaskCreate(uart_monitor_rx, "rx_task", ECHO_TASK_STACK_SIZE, NULL, 12, NULL);
}
