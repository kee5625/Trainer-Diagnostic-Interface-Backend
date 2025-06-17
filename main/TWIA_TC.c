/**
 * Coder: Noah Batcher
 * Last updated: 6/11/2025
 * Project: Trainer Fault Code Diagnostic Gatway
 * Note: Code designed to recieve trouble code from TWAI slave on TWAI bus.
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
#include "time.h"
#include "sys/time.h"



//TWAI
#define PING_PERIOD_MS          250
#define NO_OF_DATA_MSGS         1
#define NO_OF_ITERS             2
#define ITER_DELAY_MS           1000
#define RX_TASK_PRIO            8
#define TX_TASK_PRIO            9
#define FAULT_CODE_TSK_PRIO     10
#define TX_GPIO_NUM             14
#define RX_GPIO_NUM             15
#define EXAMPLE_TAG             "TWAI Master"

#define ID_MASTER_STOP_CMD      0x0A0
#define ID_MASTER_START_CMD     0x0A1
#define ID_MASTER_PING          0x0A2
#define ID_SLAVE_STOP_RESP      0x0B0
#define ID_SLAVE_DATA           0x0B1
#define ID_SLAVE_PING_RESP      0x0B2

extern char trouble_code_buff[22];

//TWAI
typedef enum {
    TX_SEND_PINGS,
    TX_SEND_START_CMD,
    TX_SEND_STOP_CMD,
    TX_TASK_EXIT,
} tx_task_action_t;

typedef enum {
    RX_RECEIVE_PING_RESP,
    RX_RECEIVE_DATA,
    RX_RECEIVE_STOP_RESP,
    RX_TASK_EXIT,
} rx_task_action_t;


static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_25KBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);

static const twai_message_t ping_message = {
    // Message type and format settings
    .extd = 0,              // Standard Format message (11-bit ID)
    .rtr = 0,               // Send a data frame
    .ss = 1,                // Is single shot (won't retry on error or NACK)
    .self = 0,              // Not a self reception request
    .dlc_non_comp = 0,      // DLC is less than 8
    // Message ID and payload
    .identifier = ID_MASTER_PING,
    .data_length_code = 0,
    .data = {0},
};

static const twai_message_t start_message = {
    // Message type and format settings
    .extd = 0,              // Standard Format message (11-bit ID)
    .rtr = 0,               // Send a data frame
    .ss = 0,                // Not single shot
    .self = 0,              // Not a self reception request
    .dlc_non_comp = 0,      // DLC is less than 8
    // Message ID and payload
    .identifier = ID_MASTER_START_CMD,
    .data_length_code = 0,
    .data = {0},
};

static const twai_message_t stop_message = {
    // Message type and format settings
    .extd = 0,              // Standard Format message (11-bit ID)
    .rtr = 0,               // Send a data frame
    .ss = 0,                // Not single shot
    .self = 0,              // Not a self reception request
    .dlc_non_comp = 0,      // DLC is less than 8
    // Message ID and payload
    .identifier = ID_MASTER_STOP_CMD,
    .data_length_code = 0,
    .data = {0},
};

static QueueHandle_t tx_task_queue;
static QueueHandle_t rx_task_queue;
static SemaphoreHandle_t stop_ping_sem;
//static SemaphoreHandle_t ctrl_task_sem;
static SemaphoreHandle_t fault_code_task_sem;
static SemaphoreHandle_t done_sem;

/* --------------------------- Tasks and Functions TWAI-------------------------- */

static void twai_receive_task(void *arg)
{
    while (1) {
        rx_task_action_t action;
        xQueueReceive(rx_task_queue, &action, portMAX_DELAY);

        if (action == RX_RECEIVE_PING_RESP) {
            //Listen for ping response from slave
            while (1) {
                twai_message_t rx_msg;
                twai_receive(&rx_msg, portMAX_DELAY);
                if (rx_msg.identifier == ID_SLAVE_PING_RESP) {
                    xSemaphoreGive(stop_ping_sem);
                    xSemaphoreGive(fault_code_task_sem);
                    break;
                }
            }
        } else if (action == RX_RECEIVE_DATA) {
            //Receive data messages from slave
            uint32_t data_msgs_rec = 0;
            while (data_msgs_rec < NO_OF_DATA_MSGS) {
                twai_message_t rx_msg;
                twai_receive(&rx_msg, portMAX_DELAY);
                if (rx_msg.identifier == ID_SLAVE_DATA) {
                    uint32_t data = 0;
                    for (int i = 0; i < rx_msg.data_length_code; i++) {
                        data |= (rx_msg.data[i] << (i * 8));
                    }
                    ESP_LOGI(EXAMPLE_TAG, "Received data value %"PRIu32, data);

                    if (strcmp(trouble_code_buff,"") == 0){
                        snprintf(trouble_code_buff,sizeof(trouble_code_buff) - 4,"Fault code: %c",(char)data);
                    }else{
                        char temp[18];
                        memcpy(temp,trouble_code_buff,sizeof(temp));
                        snprintf(trouble_code_buff,sizeof(trouble_code_buff),"%s%04li",temp,data);
                    }
                    data_msgs_rec ++;
                }
            }
            xSemaphoreGive(fault_code_task_sem);
        } else if (action == RX_RECEIVE_STOP_RESP) { 
            //Listen for stop response from slave
            while (1) {
                twai_message_t rx_msg;
                twai_receive(&rx_msg, portMAX_DELAY);
                if (rx_msg.identifier == ID_SLAVE_STOP_RESP) {
                    xSemaphoreGive(fault_code_task_sem);
                    break;
                }
            }
        } else if (action == RX_TASK_EXIT) {
            break;
        }
    }
    vTaskDelete(NULL);
}

static void twai_transmit_task(void *arg)
{
    while (1) {
        tx_task_action_t action;
        xQueueReceive(tx_task_queue, &action, portMAX_DELAY);

        if (action == TX_SEND_PINGS) {
            //Repeatedly transmit pings
            ESP_LOGI(EXAMPLE_TAG, "Transmitting ping");
            while (xSemaphoreTake(stop_ping_sem, 0) != pdTRUE) {
                twai_transmit(&ping_message, portMAX_DELAY);
                vTaskDelay(pdMS_TO_TICKS(PING_PERIOD_MS));
            }
        } else if (action == TX_SEND_START_CMD) {
            //Transmit start command to slave
            twai_transmit(&start_message, portMAX_DELAY);
            ESP_LOGI(EXAMPLE_TAG, "Transmitted start command");
        } else if (action == TX_SEND_STOP_CMD) {
            //Transmit stop command to slave
            twai_transmit(&stop_message, portMAX_DELAY);
            ESP_LOGI(EXAMPLE_TAG, "Transmitted stop command");
        } else if (action == TX_TASK_EXIT) {
            break;
        }
    }
    vTaskDelete(NULL);
}

//used to grab fault code from TWAI network
static void trouble_code_buff_task(void *arg)
{
    xSemaphoreTake(fault_code_task_sem, portMAX_DELAY);
    tx_task_action_t tx_action;
    rx_task_action_t rx_action;
    
    //runs twice to first get the fault code letter then number
    for (int i = 0; i < NO_OF_ITERS; i ++){
        ESP_ERROR_CHECK(twai_start());
        ESP_LOGI(EXAMPLE_TAG, "Driver started");

        //Start transmitting pings, and listen for ping response
        tx_action = TX_SEND_PINGS;
        rx_action = RX_RECEIVE_PING_RESP;
        xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
        xQueueSend(rx_task_queue, &rx_action, portMAX_DELAY);

        //Send Start command to slave, and receive data messages
        ESP_LOGI(EXAMPLE_TAG, "Ping received");

        xSemaphoreTake(fault_code_task_sem, portMAX_DELAY);
        tx_action = TX_SEND_START_CMD;
        rx_action = RX_RECEIVE_DATA;
        xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
        xQueueSend(rx_task_queue, &rx_action, portMAX_DELAY);

        //Send Stop command to slave when enough data messages have been received. Wait for stop response
        xSemaphoreTake(fault_code_task_sem, portMAX_DELAY);
        tx_action = TX_SEND_STOP_CMD;
        rx_action = RX_RECEIVE_STOP_RESP;
        xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
        xQueueSend(rx_task_queue, &rx_action, portMAX_DELAY);

        xSemaphoreTake(fault_code_task_sem, portMAX_DELAY);
        ESP_ERROR_CHECK(twai_stop());
        ESP_LOGI(EXAMPLE_TAG, "Driver stopped");
        vTaskDelay(pdMS_TO_TICKS(ITER_DELAY_MS));
    }

        //Stop TX and RX tasks
        tx_action = TX_TASK_EXIT;
        rx_action = RX_TASK_EXIT;
        xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
        xQueueSend(rx_task_queue, &rx_action, portMAX_DELAY);

    //Delete fault code task
    xSemaphoreGive(done_sem);
    vTaskDelete(NULL);
}

//grabs fc from slave/trainer
void twai_TC_Get(){
    //Create tasks, queues, and semaphores
    rx_task_queue = xQueueCreate(1, sizeof(rx_task_action_t));
    tx_task_queue = xQueueCreate(1, sizeof(tx_task_action_t));
    fault_code_task_sem = xSemaphoreCreateBinary();
    stop_ping_sem = xSemaphoreCreateBinary();
    done_sem = xSemaphoreCreateBinary();
    xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(twai_transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(trouble_code_buff_task, "Fault_Code", 4096, NULL, FAULT_CODE_TSK_PRIO, NULL, tskNO_AFFINITY);

    //Install TWAI driver
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(EXAMPLE_TAG, "Driver installed");
    xSemaphoreGive(fault_code_task_sem);              //start fault code reading task
    xSemaphoreTake(done_sem, portMAX_DELAY);    //Wait for completion

    //Uninstall TWAI driver
    ESP_ERROR_CHECK(twai_driver_uninstall());
    ESP_LOGI(EXAMPLE_TAG, "Driver uninstalled");

    //Cleanup
    vQueueDelete(rx_task_queue);
    vQueueDelete(tx_task_queue);
    vSemaphoreDelete(fault_code_task_sem);
    vSemaphoreDelete(stop_ping_sem);
    vSemaphoreDelete(done_sem);
}