/**
 * Coder: Noah Batcher
 * Last updated: 6/11/2025
 * Project: Trainer trouble Code Diagnostic Gatway
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
#include  "math.h"

#include "TWIA_TC.h"
#include "TC_ref.h"
#include "TWAI_OBD.h"
#include "ble.h"

//twai driver setup
#define PING_PERIOD_MS          250
#define NO_OF_DATA_MSGS         1
#define NO_OF_ITERS             1
#define ITER_DELAY_MS           1000
#define RX_TASK_PRIO            8
#define TX_TASK_PRIO            9
#define TROUBLE_CODE_TSK_PRIO   10
#define TX_GPIO_NUM             21
#define RX_GPIO_NUM             22
#define TAG                     "GateWayV2"

/* ---------- logging helpers (common) ---------- */
#define LOG_FRAME(dir, msg)                                                      \
    ESP_LOGI(TAG,                                                                \
        dir " id=0x%03X dlc=%d "                                                 \
        "%02X %02X %02X %02X %02X %02X %02X %02X",                               \
        (unsigned int)(msg).identifier, (msg).data_length_code,                  \
        (msg).data[0], (msg).data[1], (msg).data[2], (msg).data[3],              \
        (msg).data[4], (msg).data[5], (msg).data[6], (msg).data[7])

#define LOG_BIN(label, val)  ESP_LOGI(TAG, label " = %d (0x%02X)", (val), (val))



//twai config setup
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);

//**********************************Trouble Code */
static const twai_message_t TWAI_Request_msg = {
    // Message type and format settings
    .extd = 0,              // Standard Format message (11-bit ID)
    .rtr = 0,               // Send a data frame
    .ss = 0,                // Not single shot
    .self = 0,              // Not a self reception request
    .dlc_non_comp = 0,      // DLC is less than 8
    // Message ID and payload
    .identifier = ID_DT,
    .data_length_code = 8,
    .data = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    //*******Num data bytes, Mode 03 (request DTC's), padding (0)...... */
};

static const twai_message_t TWAI_Clear_DTCS = {
    // Message type and format settings
    .extd = 0,              // Standard Format message (11-bit ID)
    .rtr = 0,               // Send a data frame
    .ss = 0,                // Not single shot
    .self = 0,              // Not a self reception request
    .dlc_non_comp = 0,      // DLC is less than 8
    // Message ID and payload
    .identifier = ID_DT,
    .data_length_code = 8,
    .data = {0x01, CLEAR_DTCS_REQ, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    //*******Num data bytes, Mode 03 (request DTC's), padding (0)...... */
};

//**********************************Flow Control */
static const twai_message_t TWAI_Response_FC = { //only after FF unless ECU sending too fast
    // Message type and format settings
    .extd = 0,              // Standard Format message (11-bit ID)
    .rtr = 0,               // Send a data frame
    .ss = 0,                // Not single shot
    .self = 0,              // Not a self reception request
    .dlc_non_comp = 0,      // DLC is less than 8
    // Message ID and payload
    .identifier = ID_DT,
    .data_length_code = 8,
    .data = {MULT_FRAME_FLOW, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};


/* --------------------------- Tasks and Functions TWAI-------------------------- */
static QueueHandle_t tx_task_queue;
static SemaphoreHandle_t TC_Grabbed_sem;
static uint8_t *dtcs;
static int dtcs_bytes = 0;

//returns index of mode response
static inline uint8_t mode_find(twai_message_t msg, uint8_t last_service){
    if (msg.data[0] < 0x10){ //single frame 
        return msg.data[1];
    }else if (msg.data[0] == 0x10){
        return msg.data[2];
    }
    return last_service;
}

static inline twai_message_t req_msg_create(uint8_t request){
    twai_message_t msg = TWAI_Request_msg;
    msg.data[1] = request;
    return msg;
}



static void twai_receive_task()
{
    tx_task_action_t tx_response;
    uint8_t num_bytes = 0;
    uint8_t num_frames = 0;
    uint8_t current_service = STORED_DTCS_RESP;
    twai_message_t TC_received;
    int next_dtc = 4;
    while (1) {
        twai_receive(&TC_received,portMAX_DELAY);
        LOG_FRAME("RX ", TC_received);
        ESP_LOGI(TAG,"RX id=0x%03X  dlc=%d  %02X %02X %02X %02X %02X %02X %02X %02X",
            (unsigned int)TC_received.identifier, TC_received.data_length_code,
            TC_received.data[0],TC_received.data[1],TC_received.data[2],TC_received.data[3],
            TC_received.data[4],TC_received.data[5],TC_received.data[6],TC_received.data[7]);
        if (TC_received.identifier == ID_ECU || TC_received.identifier == ID_ECU_Second){
            current_service = mode_find(TC_received,current_service);
            if (current_service == STORED_DTCS_RESP ||  current_service == PENDING_DTCS_RESP  || current_service == PERM_DTCS_RESP){
                if (TC_received.data[0] == MULT_FRAME_FIRST){
                    num_bytes = TC_received.data[1];
                    dtcs_bytes = num_bytes - 1;
                    ESP_LOGI(TAG, "%i", dtcs_bytes);
                    dtcs = malloc(dtcs_bytes);
                    for (int i = 0; i < 4; i += 2){ //grab first two DTCS
                        dtcs[i] = TC_received.data[i + 3];
                        dtcs[i + 1] = TC_received.data[i + 4];
                    }
                    //setting  up for next frame
                    num_bytes -= 5;
                    num_frames = ceil( (double) num_bytes / 6.0); //starting down 4 bytes from FF and only 6 because DTCs cannot be split across frames
                    ESP_LOGI(TAG,"Number bytes: %i and Number frames: %i",num_bytes, num_frames);
                    //Getting next frame 
                    ESP_LOGI(TAG,"First frame of multi frame received.");
                    tx_response = TX_FLOW_CONTROL_RESPONSE;
                    
                    xQueueSend(tx_task_queue, &tx_response,portMAX_DELAY);
                }else if (TC_received.data[0] >= MULT_FRAME_CON){
                    ESP_LOGI(TAG, "Consecutive frame %i received.", TC_received.data[0] - MULT_FRAME_CON);
                    int cur_bytes = (num_bytes >= 6) ? 6 : num_bytes;
                    ESP_LOGI(TAG, "current bytes: %i total bytes: %i ", cur_bytes, num_bytes);
                    for (int i = 0; i < cur_bytes; i += 2){
                        dtcs[i + next_dtc] = TC_received.data[i + 1];
                        dtcs[i + next_dtc + 1] = TC_received.data[i + 2];
                    }
                    if (num_bytes >= 6){
                        next_dtc += 6;
                        num_bytes -= 6;
                    }else{
                        next_dtc += num_bytes;
                    }
                    ESP_LOGI(TAG,"Consecutive frame received successfully.");
                    num_frames --;
                    if (num_frames == 0){
                        num_bytes = 0;
                        next_dtc = 4;
                        xSemaphoreGive(TC_Grabbed_sem);
                    }
                }else if (TC_received.data[0] <= SINGLE_FRAME){ //single frame
                    num_bytes = TC_received.data[0] - 2;
                    dtcs_bytes = num_bytes;
                    dtcs = malloc(dtcs_bytes);
                    if (TC_received.data[1] == STORED_DTCS_RESP){
                        for (int i = 0; i < num_bytes; i += 2){     
                            dtcs[i] = TC_received.data[i + 2];
                            dtcs[i+ 1] = TC_received.data[i + 3];
                        }
                    }
                    num_bytes = 0;
                    ESP_LOGI(TAG,"SF received.");
                    xSemaphoreGive(TC_Grabbed_sem);
                }else{
                    ESP_LOGE(TAG,"Unknown frame received: %i", TC_received.data[0]);
                }
            }else if (current_service == CLEAR_DTCS_GOOD_RESP){
                BLE_notify_clear();
                ESP_LOGI(TAG, "DTCs reset successfully.");
            }else if (current_service == CLEAR_DTCS_BAD_RESP){
                ESP_LOGE(TAG,"FAILED to rest DTCs. Possibly unsupported or bad request.");
            }
        }else{
            ESP_LOGI(TAG,"Wrong identifier.");
        }
    }
}

static void twai_transmit_task()
{
    tx_task_action_t action;
    twai_message_t TX_output;
    while (1) {
        xQueueReceive(tx_task_queue, &action, portMAX_DELAY);
        ESP_LOGI(TAG, "Q←TX dequeue action=%d", action);
        switch(action){
            case TX_REQUEST_STORED_DTCS:
                TX_output = req_msg_create(STORED_DTCS_REQ);
                twai_transmit(&TX_output, portMAX_DELAY);
                ESP_LOGI(TAG,"Sent TC request ");
                break;
            case TX_REQUEST_PENDING_DTCS:
                TX_output = req_msg_create(PENDING_DTCS_REQ);
                esp_err_t e = twai_transmit(&TX_output, portMAX_DELAY);
                LOG_FRAME("TX ", TX_output);
                ESP_LOGI(TAG,"TX id=0x%03X %s  %02X %02X %02X %02X %02X %02X %02X %02X",
                    (unsigned int)TX_output.identifier,
                    esp_err_to_name(e),
                    TX_output.data[0],TX_output.data[1],TX_output.data[2],TX_output.data[3],
                    TX_output.data[4],TX_output.data[5],TX_output.data[6],TX_output.data[7]);
                break;
            case TX_REQUEST_PERM_DTCS:
                TX_output = req_msg_create(PERM_DTCS_REQ);
                twai_transmit(&TX_output, portMAX_DELAY);
                LOG_FRAME("TX ", TX_output);
                ESP_LOGI(TAG,"Sent perminate dtcs request");
                break;
            case TX_FLOW_CONTROL_RESPONSE:
                TX_output = TWAI_Response_FC;
                twai_transmit(&TX_output, portMAX_DELAY);
                LOG_FRAME("TX ", TX_output);
                ESP_LOGI(TAG,"Sent flow control");
                break;
            case SERV_CLEAR_DTCS:
                TX_output = TWAI_Clear_DTCS; 
                twai_transmit(&TX_output, portMAX_DELAY);
                LOG_FRAME("TX ", TX_output);
                ESP_LOGI(TAG,"Sent DTCS reset request");
                break;
            default:
                break;
        }
    }
    vTaskDelete(NULL);
}

//used to grab trouble code from TWAI network
static void Get_DTCS()
{
    service_request_t current_serv;
    tx_task_action_t tx_action;
    int count = 0;
    while(1){
        xQueueReceive(service_queue,&current_serv,portMAX_DELAY);
        switch (current_serv){
            case SERV_CUR_DATA:
                break;
            case SERV_FREEZE_DATA:
                break;
            case SERV_STORED_DTCS:
                tx_action = TX_REQUEST_STORED_DTCS;
                ESP_LOGI(TAG, "Q→TX enqueue action=%d", tx_action);
                xQueueSend(tx_task_queue,&tx_action, portMAX_DELAY);
              
                xSemaphoreTake(TC_Grabbed_sem,portMAX_DELAY);
                for (int i = 0; i < dtcs_bytes ; i+= 2){
                    ESP_LOGI(TAG, "Trouble code 0x%02X 0x%02X", dtcs[i],dtcs[i + 1]);
                }
                TC_Code_set(dtcs,dtcs_bytes); //passing trouble_code to the main function
                BLE_push_dtcs(dtcs, dtcs_bytes);
                xSemaphoreGive(TC_Recieved_sem); //set this to pass tc to uart after first go
                break;
            case SERV_CLEAR_DTCS://UART file resets everything
                dtcs = NULL;
                dtcs_bytes = 0;
                tx_action = SERV_CLEAR_DTCS;
                ESP_LOGI(TAG, "Q→TX enqueue action=%d", tx_action);
                xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
                break;
            case SERV_PENDING_DTCS:
                tx_action = TX_REQUEST_PENDING_DTCS;
                ESP_LOGI(TAG, "Q→TX enqueue action=%d", tx_action);
                xQueueSend(tx_task_queue,&tx_action, portMAX_DELAY);

                xSemaphoreTake(TC_Grabbed_sem,portMAX_DELAY);
                count = 1;
                for (int i = 0; i < dtcs_bytes; i+= 2){
                    ESP_LOGI(TAG, "Diagnostic Trouble code %i is 0x%02X 0x%02X", count, dtcs[i],dtcs[i + 1]);
                    count ++;
                }
                TC_Code_set(dtcs,dtcs_bytes); //passing trouble_code to the main function
                BLE_push_dtcs(dtcs, dtcs_bytes);
                xSemaphoreGive(TC_Recieved_sem); //set this to pass tc to uart after first go
                vTaskDelay(pdMS_TO_TICKS(250));
                
                break;
            case SERV_PERM_DTCS:
                tx_action = TX_REQUEST_PERM_DTCS;
                ESP_LOGI(TAG, "Q→TX enqueue action=%d", tx_action);
                xQueueSend(tx_task_queue,&tx_action, portMAX_DELAY);

                xSemaphoreTake(TC_Grabbed_sem,portMAX_DELAY);
                count = 1;
                for (int i = 0; i < dtcs_bytes; i+= 2){
                    ESP_LOGI(TAG, "Diagnostic Trouble code %i is 0x%02X 0x%02X", count, dtcs[i],dtcs[i + 1]);
                    count ++;
                }
                TC_Code_set(dtcs,dtcs_bytes); //passing trouble_code to the main function
                BLE_push_dtcs(dtcs, dtcs_bytes);
                xSemaphoreGive(TC_Recieved_sem); //set this to pass tc to uart after first go
                break;
            default:
                break;
        }
    }
}

//grabs tc from slave/trainer
void twai_TC_Get(){
    bool debug = false;
    //Create tasks, queues, and semaphores
    tx_task_queue = xQueueCreate(1, sizeof(tx_task_action_t));
    TC_Grabbed_sem = xSemaphoreCreateBinary();


    //Install TWAI driver
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(TAG, "Driver installed");

    ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(TAG,"TWAI started.");

    xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(twai_transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(Get_DTCS, "trouble_code", 4096, NULL, TROUBLE_CODE_TSK_PRIO, NULL, tskNO_AFFINITY);

    if(debug){
        vTaskDelay(pdMS_TO_TICKS(50));
        service_request_t serv = SERV_PENDING_DTCS;
        xQueueSend(service_queue, &serv, portMAX_DELAY);
    }
    
}