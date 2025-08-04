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
static uint8_t *PID_VALUE;
static int PID_NUM_BYTES = 0;
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

void UART_PID_VALUE(uint8_t *data,int num_bytes){
    PID_VALUE = data;
    PID_NUM_BYTES = num_bytes;

}

/**
 * UART Protocol: DTC (Diagnostic Trouble Codes) Communication
 * ===========================================================
 * 
 * Notes:
 * - Start commands and Receive commands are handled in the `rx` function.
 * - DTCs can be Pending, Stored, or Permanent depending on the request.
 * 
 * ------------------------------------------------------------------------
 * From Gateway (to Display)
 * ------------------------------------------------------------------------
 * Description:
 *   Sends DTC data to the display after receiving a request.
 *
 * Byte packing:
 *   Byte 0     : Receive Command (e.g., 0xBB)
 *   Byte 1     : Total Number of DTCs for the category
 *   Byte 2     : DTC 0 (first half DTC)
 *   Byte 3     : DTC 0 (second half DTC)
 *   Byte 4     : DTC 1 (first half DTC)
 *   Byte 5     : DTC 1 (second half DTC)
 *   ...
 *   Byte N     : DTC N (first half DTC)
 *   Byte N+1   : DTC N (second half DTC)
 *   Final Byte : END Command (e.g., 0xEE)
 *  
 * bit packing (commands):
 *   Bit 0     : Start Command 1
 *   Bit 1     : UART command see UART_TC.h
 *   Bit 2     : ^
 *   Bit 3     : ^
 *   Bit 4     : ^
 *   Bit 5     : 1 (uart_end_pad)
 *   Bit 6     : 1 (uart_end_pad)
 *   Bit 7     : 0 (uart_end_pad)
 * 
 * 
 * Encoded DTC: 00      00   0111 / 0000 0011
 *              ^letter ^num ^num   ^num ^num
 *              P        0   B      0    3
 *  
 *
 * ------------------------------------------------------------------------
 * From Display (to ECU / Trainer)
 * ------------------------------------------------------------------------
 * Description:
 *   Sends a request for DTCs in a specific category.
 * 
 * Byte Packing:
 * Byte 0               : Start Command (e.g., 0xAA)
 * Byte 1               : DTC Command Type:
 *                          - 0x04 = Stored
 *                          - 0x05 = Pending
 *                          - 0x06 = Permanent
 * Byte 2+ Num_bytes    : DTC Received cmd
 * 
 * 
 * Bit packing (commands):
 *   Bit 0     : Start Command 1
 *   Bit 1     : UART command see UART_TC.h
 *   Bit 2     : ^
 *   Bit 3     : ^
 *   Bit 4     : ^
 *   Bit 5     : 1 (uart_end_pad)
 *   Bit 6     : 1 (uart_end_pad)
 *   Bit 7     : 0 (uart_end_pad)
 *  
 */
static void Read_Codes(uint8_t command){
    uart_comms_t action;
    int timeout = 0;

    timeout = Set_TWAI_Serv(command); 

    if (timeout == -1) return; //TWAI timeout error

    dtcs = get_dtcs();
    num_bytes_dtcs = get_dtcs_bytes();  
    ESP_LOGI(TAG,"Sending number of DCTS: %i", num_bytes_dtcs / 2);
    ESP_LOGI(TAG,"Stored DTCs request received.");

    action = UART_DTCS_Num_cmd;
    xQueueSend(uart_send_queue, &action,portMAX_DELAY);
    
    timeout = 0;
    while (1){
        timeout = uart_read_bytes(UART_PORT_NUM, &command, 1, pdMS_TO_TICKS(10000)); //10s timeout
        if (timeout != -1){
            if (!is_valid_frame(command)) {
                action = UART_Retry_cmd;
                xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                continue;

            }

            command = (command >> 3) & 0x0F; //grabs command no padding

            if(command == UART_DTC_Received_cmd){
                ESP_LOGI(TAG,"Receive command!!!!!!!!!!!!!");
                if (num_bytes_dtcs > dtcs_sent){
                    action = UART_DTC_next_cmd;

                }else{
                    dtcs_sent = 0; //incremented by TX thread
                    action = UART_DTCs_End_cmd;
                    xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                    break;

                }

                xQueueSend(uart_send_queue, &action, portMAX_DELAY);
            }

        }else { //timeout 
            break; //end command should come.
        }

    }

}


/**
 * Gets available PIDs bit-mask and waits for PIDs through UART to update PID data. Display starts with start command
 * 
 * UART Protocol Overview
 * ======================
 * 
 * From Gateway (to Display):
 *   Byte 0   : Receive Command
 *   Byte 1-28: PIDs Supported Bitmask [7][4] (28 bytes)
 *   Byte 29  : Checksum (display repeat req if failed)
 *   Byte 30+ : Repeated blocks of:
 *              - Byte N    : Num of bytes for PID data
 *              - Byte N+1  : Data for PID
 *              - Byte N+2  : Checksum (display repeat req if failed)
 * 
 * From Display (to Gateway):
 *   Byte 0   : Start Command (e.g. 0x20)
 *   Byte 1   : UART_PIDS Request Flag
 *   Byte 2+  : Sequence of PIDs requested (1 byte each)
 *   Final    : 0x20 sent
 */
static void PIDs_GRAB_LIVE_DATA(){
    uint8_t rx_data = 0x00;
    uart_event_t event;

    Set_TWAI_Serv(SERV_PIDS); //grabs bit-mask
    ESP_LOGI(TAG,"PIDs bitmask grabbed.");

    uint8_t * temp_row;
    uint8_t checksum = 0;

    //sending PIDs mask
    for (int i = 0; i < 7; i ++){
        temp_row = get_bitmask_row(i);
        for (int k = 0; k < 4; k ++){
            uart_write_bytes(UART_PORT_NUM,&temp_row[k] ,1);
            uart_wait_tx_done(UART_PORT_NUM, portMAX_DELAY);
            vTaskDelay(pdMS_TO_TICKS(25));
            checksum += temp_row[k];
        }
    }
    checksum = ~checksum;
    uart_write_bytes(UART_PORT_NUM,&checksum,1);
    ESP_LOGI(TAG,"checksum writen. %i", checksum);

    /**
     * Grabbing PIDs data... PID 0x20 = exit command
     * 
     * Note:PID 0x20 is a bitmask request and is already grabbed with the bitmask.
     */
    while (1){
        ESP_LOGI(TAG,"Waiting on PID...");

        xQueueReceive(uart_queue,&event,portMAX_DELAY);
        if (event.type != UART_DATA) continue; //skip errors and retry might need fixing here

        uart_read_bytes(UART_PORT_NUM, &rx_data, 1, portMAX_DELAY); //grab rx buffer

        if (rx_data == 0x20) break; //exit

        Set_Req_PID(rx_data); //set pid in main
        ESP_LOGI(TAG,"Grabbing next PID 0x%02X", rx_data);
        Set_TWAI_Serv(SERV_DATA); //Thread blocked until TWAI grabs data

       
        uint8_t checksum = 0;
        if (PID_NUM_BYTES == 0) break; //0 data
        uart_write_bytes(UART_PORT_NUM, &PID_NUM_BYTES,1); //send num_bytes
        uart_wait_tx_done(UART_PORT_NUM, portMAX_DELAY);

        for (int i = 0; i < PID_NUM_BYTES; i++){
            uint8_t temp = PID_VALUE[i];
            uart_write_bytes(UART_PORT_NUM, &temp,1);
            uart_wait_tx_done(UART_PORT_NUM, portMAX_DELAY);
            checksum += PID_VALUE[i];
        }

        checksum = ~checksum;//1s compliment
        ESP_LOGI(TAG,"FINISHED and sent checksum for PID data. %i", checksum);
        uart_write_bytes(UART_PORT_NUM, &checksum, 1);
        uart_wait_tx_done(UART_PORT_NUM, portMAX_DELAY);
    }
    ESP_LOGI(TAG,"PID service complete.");
}

/**
 * Fucntion Description: Queue up and send beginning request. 
 * Note: Only for PID data this function will be responsible for all sending.
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
                if (num_bytes_dtcs == 0) break; //don't send if twai failure or no codes
                uart_write_bytes(UART_PORT_NUM, &dtcs[dtcs_sent], 1);
                uart_wait_tx_done(UART_PORT_NUM, portMAX_DELAY);
                uart_write_bytes(UART_PORT_NUM, &dtcs[dtcs_sent + 1], 1);
                uart_wait_tx_done(UART_PORT_NUM, portMAX_DELAY);
                ESP_LOGI(TAG,"Trouble code bytes sent 0x%02X 0x%02X, dtcs_sent index: %i",dtcs[dtcs_sent], dtcs[dtcs_sent + 1],dtcs_sent);
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

            case UART_DTCS_Num_cmd:
                ESP_LOGI(TAG,"NUM dtcs sent %i", num_bytes_dtcs / 2);
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
 *  
 */
static void UART_RX(){
    uart_event_t rx_action;
    uart_comms_t action;
    uint8_t rx_data;
    uint8_t command;
    vTaskDelay(pdMS_TO_TICKS(50));
    while(1){
        xQueueReceive(uart_queue,&rx_action,portMAX_DELAY);
        switch (rx_action.type){
            case UART_DATA: //UART DATA Case
                for(;;){
                    uart_read_bytes(UART_PORT_NUM, &rx_data, 1, portMAX_DELAY);
                    ESP_LOGI(TAG,"Grabbed data %i", rx_data);
                    
                    // checking if data is valid
                    if (is_valid_frame(rx_data)){
                        break;
                    }else{
                        action = UART_Retry_cmd;
                        xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                        ESP_LOGW(TAG, "Invalid command.\n Command: %02X", rx_data);
                    }
                }

                command = (rx_data >> 3) & 0x0F;
                //switch statement for commmands inside UART_DATA case switch (see above)
                switch (command){
                case UART_Start_cmd:
                    ESP_LOGI(TAG,"Received start command.");
                    dtcs_sent = 0;
                    action = UART_Received_cmd;
                    xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                    break;

                case UART_DTCs_REQ_STORED_cmd:
                    Read_Codes(SERV_STORED_DTCS);
                    break;
                case UART_DTCs_REQ_PENDING_cmd:
                    Read_Codes(SERV_PENDING_DTCS);
                    break;
                case UART_DTCs_REQ_PERM_cmd:
                    Read_Codes(SERV_PERM_DTCS);
                    break;

                case UART_DTCs_Reset_cmd:
                    ESP_LOGI(TAG,"Reseting DTCs.");

                    //resetting trouble_code
                    dtcs = NULL;
                    num_bytes_dtcs = 0;
                    vTaskDelay(pdMS_TO_TICKS(100));
                    Set_TWAI_Serv(SERV_CLEAR_DTCS);
                    action = UART_Received_cmd;
                    xQueueSend(uart_send_queue, &action, portMAX_DELAY);

                    break;

                case UART_PIDS:
                    PIDs_GRAB_LIVE_DATA(); //does both grabbing available PID bit-mask and PID data
                    break;
                
                case UART_end_of_cmd:
                    break;

                default:
                    ESP_LOGI(TAG,"Received end command stoping and clearing queues.");

                    //UART hardware reset
                    uart_flush_input(UART_NUM_1);
                    uart_wait_tx_done(UART_NUM_1, pdMS_TO_TICKS(100));

                    //Queue clear
                    xQueueReset(uart_send_queue);
                    xQueueReset(uart_queue);

                    //received command sent back
                    action = UART_Received_cmd;
                    xQueueSend(uart_send_queue, &action, portMAX_DELAY);
                    break;
                }

                break; //for UART data type switch

            //error handeling for rx line.
            case UART_FIFO_OVF:
            case UART_BUFFER_FULL:
            case UART_BREAK:
            case UART_PARITY_ERR:
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, " Error: %i", rx_action.type);
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
