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
#define UART_Priority 7 //twai ones are set to 8-10 
#define UART_RX_Priority 5
#define BUF_SIZE (1024)

#define UART_PORT_NUM      (CONFIG_EXAMPLE_UART_PORT_NUM)
#define ECHO_UART_BAUD_RATE     115200 
#define ECHO_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)
#define CONFIG_UART_ISR_IN_IRAM false

static const char *TAG = "UART Service";
static uint8_t *dtcs; //1-d array of dtcs
static uint8_t num_bytes_dtcs;
static int dtcs_sent = 0;
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
    vTaskDelay(pdMS_TO_TICKS(50));
    for (;;){
        xQueueReceive(uart_send_queue,&action,portMAX_DELAY); 
        uart_flush(UART_PORT_NUM);
        switch (action){
            case UART_Received_cmd:
                ESP_LOGI(TAG,"Sending received command back.");
                temp_byte = uart_byte_setup(UART_Received_cmd);
                uart_write_bytes(UART_PORT_NUM, &temp_byte, 1);
                break;
            case UART_DTC_next_cmd:
                //sending two bytes for one dtcs
                if (num_bytes_dtcs == 0) break; //don't send if twai failure
                uart_flush(UART_PORT_NUM);
                uart_write_bytes(UART_PORT_NUM, &dtcs[dtcs_sent], 1);
                uart_wait_tx_done(UART_PORT_NUM, portMAX_DELAY);
                uart_write_bytes(UART_PORT_NUM, &dtcs[dtcs_sent + 1], 1);
                uart_wait_tx_done(UART_PORT_NUM, portMAX_DELAY);
                ESP_LOGI(TAG,"Trouble code bytes sent 0x%02X 0x%02X",dtcs[dtcs_sent], dtcs[dtcs_sent + 1]);
                dtcs_sent += 2;
                break;
            case UART_DTCs_Reset_cmd:
                ESP_LOGI(TAG,"Received rest TC command");
                temp_byte = uart_byte_setup(UART_Received_cmd);
                uart_write_bytes(UART_PORT_NUM, &temp_byte, 1);
                break;
            case UART_Retry_cmd:
                ESP_LOGI(TAG, "Sending retry commmand request.");
                temp_byte = uart_byte_setup(UART_Retry_cmd);
                uart_write_bytes(UART_PORT_NUM, &temp_byte,1);
                break;
            case UART_DTCs_End_cmd:
                ESP_LOGI(TAG,"All dtcs codes sent");
                temp_byte = uart_byte_setup(UART_DTCs_End_cmd);
                uart_write_bytes(UART_PORT_NUM, &temp_byte,1);
                break;
            case UART_DTCs_Num_cmd:
                ESP_LOGI(TAG,"Sending number of DCTS: %i", num_bytes_dtcs / 2);
                temp_byte = num_bytes_dtcs / 2;
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
 * CMD order receive: Start   , num TC req, TC request, TC received, TC received, TC received, TC reset, start...
 * CMD order Send   : received, num TC    ,TC         , Next TC    , Next TC    , TC end comm, received, received...
 */
static void UART_RX(){
    uart_event_t rx_action;
    uart_comms_t action;
    uint8_t rx_data;
    vTaskDelay(pdMS_TO_TICKS(50));
    while(1){
        xQueueReceive(uart_queue,&rx_action,portMAX_DELAY);
        switch (rx_action.type){
            case UART_DATA:
                for(;;){
                    uart_read_bytes(UART_PORT_NUM, &rx_data, 1, portMAX_DELAY);
                    // checking if data is valid
                    if (((rx_data >> 7) & uart_start_pad) && ((rx_data & 0x07) & uart_end_pad)){
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
                        dtcs_sent = 0;
                        action = UART_Received_cmd;
                        xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                        break;
                    case UART_DTCs_REQ_STORED_cmd:
                        set_serv(SERV_STORED_DTCS);

                        dtcs = get_dtcs();
                        num_bytes_dtcs = get_dtcs_bytes();  
                    
                        ESP_LOGI(TAG,"Stored DTCs request received.");
                        action = UART_DTCs_Num_cmd;
                        xQueueSend(uart_send_queue, &action,portMAX_DELAY);
                        action = UART_DTC_next_cmd;
                        xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                        break;
                    case UART_DTCs_REQ_PENDING_cmd:
                        set_serv(SERV_PENDING_DTCS);
            
                        dtcs = get_dtcs();
                        num_bytes_dtcs = get_dtcs_bytes();  

                        ESP_LOGI(TAG,"Pending DTCs request received.");
                        action = UART_DTCs_Num_cmd;
                        xQueueSend(uart_send_queue, &action,portMAX_DELAY);
                        action = UART_DTC_next_cmd;
                        xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                        break;
                    case UART_DTCs_REQ_PERM_cmd:
                        set_serv(SERV_PERM_DTCS);
                        
                        dtcs = get_dtcs();
                        num_bytes_dtcs = get_dtcs_bytes();  

                        ESP_LOGI(TAG,"Perminate DTCs request received.");
                        action = UART_DTCs_Num_cmd;
                        xQueueSend(uart_send_queue, &action,portMAX_DELAY);
                        action = UART_DTC_next_cmd;
                        xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                        break;
                    case UART_DTC_Received_cmd:
                        ESP_LOGI(TAG,"DTC sent successfully.");
                        if (num_bytes_dtcs >= dtcs_sent + 2){
                            action = UART_DTC_next_cmd;
                        }else{
                            dtcs_sent = 0;
                            action = UART_DTCs_End_cmd;
                        }
                        xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                        break;
                    case UART_DTCs_Reset_cmd:
                        ESP_LOGI(TAG,"Reseting DTCs.");
                        action = UART_Received_cmd;
                        xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                        //resetting trouble_code
                        dtcs = NULL;
                        num_bytes_dtcs = 0;
                        vTaskDelay(pdMS_TO_TICKS(100));
                        set_serv(SERV_CLEAR_DTCS);
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
}

void UART_INIT()
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
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE, 0, 1, &uart_queue, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));
    xTaskCreate(UART_TX, "UART_TX_task", ECHO_TASK_STACK_SIZE, NULL, UART_Priority, NULL);
    xTaskCreate(UART_RX, "UART_RX", ECHO_TASK_STACK_SIZE, NULL, UART_RX_Priority, NULL);
}
