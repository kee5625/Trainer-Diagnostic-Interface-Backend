/**
 * Coder: Noah Batcher
 * Last updated: 6/11/2025
 * Project: Trainer Fault Code Diagnostic Gatway
 * Note: Code used to send trouble code through UART
 * 
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "TC_ref.h"
#include "UART_TC.h"

#define ECHO_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define ECHO_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)
#define UART_Priority 7 //twai ones are set to 8-10 (TWAI and UART do not run at the same time rn)
#define UART_RX_Priority 5
#define BUF_SIZE (1024)

#define UART_PORT_NUM      (CONFIG_EXAMPLE_UART_PORT_NUM)
#define ECHO_UART_BAUD_RATE     115200 
#define ECHO_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)
#define CONFIG_UART_ISR_IN_IRAM false

static const char *TAG = "UART Service";
static char trouble_code[tc_size + 2];
static QueueHandle_t uart_queue;
static QueueHandle_t uart_send_queue;

static inline uint8_t uart_byte_setup(uart_comms_t command){
		return     ((uart_start_pad & 0x01) << 7) |
				   ((command & 0x0F) << 3) |
				   ((uart_end_pad & 0x07) << 0);
}

static inline bool is_valid_frame(uint8_t byte){
	return ((byte >> 7) & uart_start_pad) && ((byte & 0x07) & uart_end_pad);
}

/**
 * Fucntion Description: Sends UART based on the xQueueSend from UART_RX() function.
 * Note: See the UART_RX() function for the expected command order.
 */
static void UART_TX(){
    uart_send_queue = xQueueCreate(1, sizeof(uart_comms_t));
    uart_comms_t action;
    uint8_t temp_byte;

    for (;;){
        xQueueReceive(uart_send_queue,&action,portMAX_DELAY); 
        uart_flush(UART_PORT_NUM);
        switch (action){
            case UART_Received_cmd:
                ESP_LOGI(TAG,"Sending received command back.");
                temp_byte = uart_byte_setup(UART_Received_cmd);
                uart_write_bytes(UART_PORT_NUM, &temp_byte, 1);
                break;
            case UART_TC_Req_cmd:
                    uart_write_bytes(UART_PORT_NUM, trouble_code, sizeof(trouble_code)-1);
                    trouble_code[5] = '\0';
                    ESP_LOGI(TAG,"Trouble code %s sent.",trouble_code);
                break;
            case UART_TC_Reset_cmd:
                ESP_LOGI(TAG,"Received rest TC command");
                temp_byte = uart_byte_setup(UART_Received_cmd);
                uart_write_bytes(UART_PORT_NUM, &temp_byte, 1);
                memset(trouble_code,0,sizeof(trouble_code));
                TC_Code_set(trouble_code);
                break;
            case UART_Retry_cmd:
                ESP_LOGI(TAG, "Sending retry commmand request.");
                temp_byte = uart_byte_setup(UART_Retry_cmd);
                uart_write_bytes(UART_PORT_NUM, &temp_byte,1);
                break;
            default:
                break;
        }
        uart_wait_tx_done(UART_PORT_NUM, portMAX_DELAY);
    }
}


/**
 * Function Description: Receive inputs from UART and fill queue for uart_send_queue in the UART_TX() function.
 * Notes: Below is the expected command order the Gateway(slave) should receive from the Display(master)
 * CMD order receive: Start   , TC request, TC received, TC reset, start...
 * CMD order Send   : received, TC        ,            , received, received...
 */
static void UART_RX(){
    uart_event_t rx_action;
    uart_comms_t action;
    uint8_t rx_data;
    while(1){
        xQueueReceive(uart_queue,&rx_action,portMAX_DELAY);
        switch (rx_action.type){
            case UART_DATA:
                for(;;){
                    uart_read_bytes(UART_PORT_NUM, &rx_data, 1, portMAX_DELAY);
                    // checking if data is valid
                    if (((rx_data >> 7) & uart_start_pad) && ((rx_data & 0x07) & uart_end_pad)){
                        ESP_LOGI(TAG,"Command valid.");//************************************************************************************************************** */
                        break;
                    }else{
                        action = UART_Retry_cmd;
                        xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                        ESP_LOGW(TAG, "Invalid command.\n Command: %02X", rx_data);
                    }
                }
                 
                //checking command
                switch ((rx_data >> 3) & 0x0F){
                    case UART_Start_cmd:
                        ESP_LOGI(TAG,"Received start command.");
                        action = UART_Received_cmd;
                        xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                        break;
                    case UART_TC_Req_cmd:
                        memcpy(trouble_code,TC_Code_Get(),sizeof(trouble_code));
                        if (trouble_code[0] == '\0'){
                            ESP_LOGI(TAG,"TC not loaded yet but requested.");
                            vTaskDelay(pdMS_TO_TICKS(500));
                            action = UART_Retry_cmd;
                            xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                        }else{
                            ESP_LOGI(TAG,"Received TC request.");
                            action = UART_TC_Req_cmd;
                            xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                        }
                        break;
                    case UART_TC_Received_cmd:
                        ESP_LOGI(TAG,"TC sent successfully.");
                        break;
                    case UART_TC_Reset_cmd:
                        ESP_LOGI(TAG,"Loading new trouble code.");
                        action = UART_TC_Reset_cmd;
                        xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                        vTaskDelay(pdMS_TO_TICKS(300));
                        new_tc_tasks(); //grab new code for reset
                        break;
                    default:
                        break;
                }
                break;
            //error handeling for rx line.
            case UART_FIFO_OVF:
            case UART_BUFFER_FULL:
            case UART_BREAK:
            case UART_PARITY_ERR:
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "%i", rx_action.type);
                uart_flush_input(UART_PORT_NUM);
                xQueueReset(uart_queue);
                action = UART_Retry_cmd;
                xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                break;
            default:
                break; 
        }   
    }
    vTaskDelete(NULL);
}

void UART_INIT(char tc_pass[tc_size + 2])
{   

    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    int intr_alloc_flags = 0; //sets interrupt if 1

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE, BUF_SIZE, 1, &uart_queue, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));
    memcpy(trouble_code,tc_pass,sizeof(trouble_code));
    xTaskCreate(UART_TX, "UART_TX_task", ECHO_TASK_STACK_SIZE, NULL, UART_Priority, NULL);
    xTaskCreate(UART_RX, "UART_RX", ECHO_TASK_STACK_SIZE, NULL, UART_RX_Priority, NULL);
}
