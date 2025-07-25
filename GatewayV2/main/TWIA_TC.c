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
#define TIMEOUT_PERIOD_MS       1500
#define RX_TASK_PRIO            8
#define TX_TASK_PRIO            9
#define TROUBLE_CODE_TSK_PRIO   10
#define TX_GPIO_NUM             21
#define RX_GPIO_NUM             22
#define TAG                     "Gateway TWAI"

//twai config setup
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
// only accept functional requests 0x7DF and ECU responses 0x7E8–0x7EF
static const twai_filter_config_t f_config = {
    .acceptance_code = (ID_DT_ECUs << 21),                      // 0x7DF
    .acceptance_mask = ~(((ID_ECU_Second - ID_DT_ECUs) << 21)),  // cover 0x7E8–0x7EF
    .single_filter   = false,
};
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
    .identifier = ID_DT_ECUs,
    .data_length_code = 8,
    .data = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
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
    .identifier = ID_DT_ECUs,
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
    .identifier = ID_DT_ECUs,
    .data_length_code = 8,
    .data = {MULT_FRAME_FLOW, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

static const uint8_t LIVE_PIDS[] = {
    SHOW_LIVE_DATA_REQ,     // mode byte
    0xA0,                   // ignition
    0xA1,                   // seat Adjuster
    0xA2                    // Lumbar Adjuster
};

extern volatile bool stream_on_master;

/* --------------------------- Tasks and Functions TWAI-------------------------- */
static QueueHandle_t tx_task_queue;
static SemaphoreHandle_t TC_Grabbed_sem;
static uint8_t *dtcs;
static int dtcs_bytes = 0;

//returns index of mode response
static inline uint8_t mode_identify(twai_message_t msg, uint8_t last_service){
    if (msg.data[0] <= SINGLE_FRAME){ 
        return msg.data[1];
    }else if (msg.data[0] == MULT_FRAME_FIRST){ 
        return msg.data[2];
    }
    return last_service;                                                                //note: This mean its a CF frame
}

static inline twai_message_t req_msg_create(uint8_t request){
    twai_message_t msg = TWAI_Request_msg;
    msg.data[1] = request;
    return msg;
}

static inline twai_message_t PID_MSG_REQ(uint8_t request, uint8_t PID){
    twai_message_t msg = TWAI_Request_msg;
    msg.data[1] = request;
    msg.data[2] = PID;
    return msg;
}

static void live_stream_task(void *arg);

static void live_stream_task(void *arg)
{
    tx_task_action_t req = TX_REQUEST_LD;
    while (1) {
        if (stream_on_master) {
            // queue a one‑shot live‑data request
            xQueueSend(tx_task_queue, &req, 0);
        }
        // wait 500ms between bursts
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static int DTC_Frame_Reading(twai_message_t data, int num_bytes, int next_dtc){
    tx_task_action_t tx_response;
    // ESP_LOGI(TAG,"ID: %li",data.identifier);
    // for (int i = 0; i < 8; i++){
    //     ESP_LOGI(TAG,"Data %i: %i", i , data.data[i]);
    // }
    if (data.data[0] == MULT_FRAME_FIRST){
        ESP_LOGI(TAG,"[ISO-TP] First Frame: byte1=0x%02X length=%d", data.data[0], data.data[1]);
        //setting num DTCs
        num_bytes = data.data[1];
        dtcs_bytes = num_bytes - 1;
        dtcs = pvPortMalloc(dtcs_bytes);

        //grabing first two DTCS
        for (int i = 0; i < 4; i += 2){ 
            dtcs[i] = data.data[i + 3];
            dtcs[i + 1] = data.data[i + 4];
        }

        //setting  up for next frame
        num_bytes -= 5;                                             //note: num_bytes only count data bytes and mode byte
        
        //Getting next frame 
        ESP_LOGI(TAG,"First frame of multi frame received.");
        tx_response = TX_FLOW_CONTROL_RESPONSE;
        xQueueSend(tx_task_queue, &tx_response,portMAX_DELAY);
    }else if (data.data[0] >= MULT_FRAME_CON){
        ESP_LOGI(TAG,"[ISO-TP] Consecutive Frame: SN=0x%02X", data.data[0] & 0x0F);

        //setting number of bytes expected in current frame
        int cur_bytes = (num_bytes >= 6) ? 6 : num_bytes;

        //grab DTCs
        for (int i = 0; i < cur_bytes; i += 2){
            dtcs[i + next_dtc] = data.data[i + 1];
            dtcs[i + next_dtc + 1] = data.data[i + 2];
        }
        //set num bytes 
        if (num_bytes >= 6){
            num_bytes -= 6;
        }else{
            num_bytes = 0;
        }
        ESP_LOGI(TAG,"Consecutive frame received successfully.");
    }else if (data.data[0] <= SINGLE_FRAME){ //single frame
        //grab num_bytes
        ESP_LOGI(TAG,"[ISO-TP] Single Frame: length=%d", data.data[0] & 0x0F);
        num_bytes = data.data[0] - 2;
        dtcs_bytes = num_bytes;
        dtcs = pvPortMalloc(dtcs_bytes);

        //grab DTCs
        for (int i = 0; i < num_bytes; i += 2){     
            dtcs[i] = data.data[i + 2];
            dtcs[i+ 1] = data.data[i + 3];
        }
        
        num_bytes = 0;
        ESP_LOGI(TAG,"SF received.");
    }else{
        ESP_LOGE(TAG,"Unknown frame received: %i", data.data[0]);
    }
    
    return num_bytes;
}

static int Live_Data_Get(twai_message_t data, int num_bytes, int next_dtc){
    tx_task_action_t tx_response;
    // ESP_LOGI(TAG,"ID: %li",data.identifier);
    // for (int i = 0; i < 8; i++){
    //     ESP_LOGI(TAG,"Data %i: %i", i , data.data[i]);
    // }
    if (data.data[0] == MULT_FRAME_FIRST){
        //setting num DTCs
        num_bytes = data.data[1];
        dtcs_bytes = num_bytes - 1;
        dtcs = pvPortMalloc(dtcs_bytes);

        //grabing first two DTCS
        for (int i = 0; i < 4; i += 2){ 
            dtcs[i] = data.data[i + 3];
            dtcs[i + 1] = data.data[i + 4];
        }

        //setting  up for next frame
        num_bytes -= 5;                                             //note: num_bytes only count data bytes and mode byte
        
        //Getting next frame 
        ESP_LOGI(TAG,"First frame of multi frame received.");
        tx_response = TX_FLOW_CONTROL_RESPONSE;
        xQueueSend(tx_task_queue, &tx_response,portMAX_DELAY);
    }else if (data.data[0] >= MULT_FRAME_CON){

        //setting number of bytes expected in current frame
        int cur_bytes = (num_bytes >= 6) ? 6 : num_bytes;

        //grab DTCs
        for (int i = 0; i < cur_bytes; i += 2){
            dtcs[i + next_dtc] = data.data[i + 1];
            dtcs[i + next_dtc + 1] = data.data[i + 2];
        }
        //set num bytes 
        if (num_bytes >= 6){
            num_bytes -= 6;
        }else{
            num_bytes = 0;
        }
        ESP_LOGI(TAG,"Consecutive frame received successfully.");
    }else if (data.data[0] <= SINGLE_FRAME){ //single frame
        //grab num_bytes
        num_bytes = data.data[0] - 2;
        dtcs_bytes = num_bytes;
        dtcs = pvPortMalloc(dtcs_bytes);

        //grab DTCs
        for (int i = 0; i < num_bytes; i += 2){     
            dtcs[i] = data.data[i + 2];
            dtcs[i+ 1] = data.data[i + 3];
        }
        
        num_bytes = 0;
        ESP_LOGI(TAG,"SF received.");
    }else{
        ESP_LOGE(TAG,"Unknown frame received: %i", data.data[0]);
    }
    return num_bytes;
}

static void twai_receive_task()
{
    
    uint8_t num_bytes = 0;
    uint8_t temp_bytes = 0;
    uint8_t current_service = STORED_DTCS_RESP;
    twai_message_t TX_Data;
    esp_err_t rec_successful;
    int next_dtc = 4;
    while (1) {
        rec_successful = twai_receive(&TX_Data,portMAX_DELAY);
        if (rec_successful == ESP_OK){ 
            
            if (TX_Data.identifier >= ID_ECU && TX_Data.identifier <= ID_ECU_Second){ 
                ESP_LOGI(TAG, "[RX_TASK] raw bytes: %02X %02X %02X %02X %02X %02X %02X %02X",
                     TX_Data.data[0],TX_Data.data[1],TX_Data.data[2],TX_Data.data[3],
                     TX_Data.data[4],TX_Data.data[5],TX_Data.data[6],TX_Data.data[7]);
                current_service = mode_identify(TX_Data,current_service);
                if (current_service == STORED_DTCS_RESP ||  current_service == PENDING_DTCS_RESP  || current_service == PERM_DTCS_RESP){
                    temp_bytes = num_bytes;
                    num_bytes = DTC_Frame_Reading(TX_Data,num_bytes,next_dtc);//------------------------------------------------------note: Different functions read frames differently
                    if (num_bytes == 0){
                        next_dtc = 4;
                        xSemaphoreGive(TC_Grabbed_sem);
                        //after all frames received, push to BLE UI
                        BLE_push_dtcs(dtcs, dtcs_bytes);
                    }else if (temp_bytes != 0){
                        next_dtc += temp_bytes - num_bytes;
                    }
                }else if (current_service == CLEAR_DTCS_GOOD_RESP){
                    ESP_LOGI(TAG, "DTCs reset successfully.");
                    BLE_notify_clear();//notify UI that clear completed
                    xSemaphoreGive(DTCS_Loaded_sem);                    
                }else if (current_service == CLEAR_DTCS_BAD_RESP){
                    ESP_LOGE(TAG,"FAILED to rest DTCs. Possibly unsupported or bad request.");
                }else if (current_service == SHOW_LIVE_DATA_RESP){
                    // use your existing live-data parser
                    num_bytes = Live_Data_Get(TX_Data, num_bytes, next_dtc);
                    if (num_bytes == 0) {
                        // done—push the three [PID,value] bytes to the UI
                        BLE_push_dtcs(dtcs, dtcs_bytes);
                        next_dtc = 0; // reset for next call

                        // unblock the service task so the next request can run
                        xSemaphoreGive(TC_Grabbed_sem);
                    } else {
                        next_dtc += (temp_bytes - num_bytes);
                    }
                }else if (current_service == SHOW_FREEZE_FRAME_RESP){

                }else{
                    ESP_LOGI(TAG,"TX_output rx: ID: %li, Data length: %i", TX_Data.identifier,TX_Data.data_length_code);
                    for (int i = 0; i < 8; i++){
                        ESP_LOGI(TAG,"rx Data %i: 0x%02X",i, TX_Data.data[i]);
                    }
                }
            }else{
                // ESP_LOGI(TAG,"Wrong identifier. ID: %li", TX_Data.identifier);
            }
        }
    }
}

static void twai_transmit_task()
{
    tx_task_action_t action;
    twai_message_t TX_output;
    twai_message_t msg;
    while (1) {
        xQueueReceive(tx_task_queue, &action, portMAX_DELAY);
        ESP_LOGI(TAG, "[TX_TASK] Dequeued action %d", action);
        switch(action){
            case TX_REQUEST_LD:
                // build a single-frame: [len, mode, PID1, PID2, PID3]
                msg = TWAI_Request_msg;         // your 0x01,… template
                msg.data[0] = sizeof(LIVE_PIDS); // 4 bytes: mode + 3 PIDs
                memcpy(&msg.data[1], LIVE_PIDS, sizeof(LIVE_PIDS));
                twai_transmit(&msg, portMAX_DELAY);
                ESP_LOGI(TAG, "Sent live-data request [%02X %02X %02X %02X]",
                        msg.data[0], msg.data[1], msg.data[2], msg.data[3]);
                break;
            case TX_REQUEST_STORED_DTCS:
                TX_output = req_msg_create(STORED_DTCS_REQ);
                twai_transmit(&TX_output, portMAX_DELAY);
                ESP_LOGI(TAG, "[TX] Mode 07 (stored DTC) sent: ID=0x%03X [%02X %02X …]",
                    (unsigned int)TX_output.identifier,
                    (int)TX_output.data[0], TX_output.data[1]);
                break;
            case TX_REQUEST_PENDING_DTCS:
                TX_output = req_msg_create(PENDING_DTCS_REQ);
                twai_transmit(&TX_output, portMAX_DELAY);
                ESP_LOGI(TAG, "[TX] Mode 07 (pending DTC) sent: ID=0x%03X [%02X %02X …]",
                    (unsigned int)TX_output.identifier,
                    (int)TX_output.data[0], TX_output.data[1]);
                break;
            case TX_REQUEST_PERM_DTCS:
                TX_output = req_msg_create(PERM_DTCS_REQ);
                twai_transmit(&TX_output, portMAX_DELAY);
                ESP_LOGI(TAG, "[TX] Mode 07 (permanent DTC) sent: ID=0x%03X [%02X %02X …]",
                    (unsigned int)TX_output.identifier,
                    (int)TX_output.data[0], TX_output.data[1]);
                break;
            case TX_FLOW_CONTROL_RESPONSE:
                TX_output = TWAI_Response_FC;
                twai_transmit(&TX_output, portMAX_DELAY);
                ESP_LOGI(TAG,"Sent flow control");
                break;
            case TX_RESET_DTCs:
                TX_output = TWAI_Clear_DTCS;
                twai_transmit(&TX_output, portMAX_DELAY);
                ESP_LOGI(TAG,"Sent DTCS reset request");
                break;
            default:
                break;
        }
    }
}

//used to grab trouble code from TWAI network
static void TWAI_Services()
{
    service_request_t current_serv;
    tx_task_action_t tx_action;
    int count = 0;
    while(1){
        ESP_LOGI(TAG,"[SERVICE] Waiting for next service_request");
        xQueueReceive(service_queue,&current_serv,portMAX_DELAY);
        ESP_LOGI(TAG,"[SERVICE] Got request %d", current_serv);
        switch (current_serv){
            case SERV_LD_DATA:
                tx_action = TX_REQUEST_LD;     // send the TX action
                xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
                xSemaphoreTake(TC_Grabbed_sem, portMAX_DELAY);
                break;
            case SERV_FREEZE_DATA:
                break;
            case SERV_STORED_DTCS:
                ESP_LOGI(TAG,"[SERVICE] Handling STORED_DTCS");
                tx_action = TX_REQUEST_STORED_DTCS;
                xQueueSend(tx_task_queue,&tx_action, portMAX_DELAY);
              
                // wait for up to TIMEOUT_PERIOD_MS for any SF → semaphore
                if (xSemaphoreTake(TC_Grabbed_sem,
                      pdMS_TO_TICKS(TIMEOUT_PERIOD_MS)) == pdTRUE) {
                    // got at least one DTC frame
                     ESP_LOGI(TAG,"[SERVICE] Semaphore signaled for STORED");
                } else {
                    // timeout—no pending DTCs from trainer
                    ESP_LOGI(TAG, "No stored DTC response (timeout).");
                    // make sure dtcs_bytes==0 so we push an empty list
                    dtcs_bytes = 0;
                }

                for (int i = 0; i < dtcs_bytes; i+= 2){
                    ESP_LOGI(TAG, "Trouble code 0x%02X 0x%02X", dtcs[i],dtcs[i + 1]);
                }
                TC_Code_set(dtcs,dtcs_bytes); //passing trouble_code to the main function
                ESP_LOGI(TAG,"[SERVICE] Pushing %d DTCs to both UART & BLE", dtcs_bytes/2);
                BLE_push_dtcs(dtcs, dtcs_bytes);
                ESP_LOGI(TAG,"[SERVICE] STORED_DTCS done");
                xSemaphoreGive(DTCS_Loaded_sem); //set this to pass tc to uart after first go
                break;
            case SERV_CLEAR_DTCS://UART file resets everything
                if (service_queue) {
                    xQueueReset(service_queue);
                }

                dtcs = NULL;
                dtcs_bytes = 0;
                tx_action = TX_RESET_DTCs;
                xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
                
                break;
            case SERV_PENDING_DTCS:
                ESP_LOGI(TAG,"[SERVICE] Handling PENDING_DTCS");    
                tx_action = TX_REQUEST_PENDING_DTCS;
                xQueueSend(tx_task_queue,&tx_action, portMAX_DELAY);

                // wait for up to TIMEOUT_PERIOD_MS for any SF → semaphore
                if (xSemaphoreTake(TC_Grabbed_sem,
                      pdMS_TO_TICKS(TIMEOUT_PERIOD_MS)) == pdTRUE) {
                    // got at least one DTC frame
                } else {
                    // timeout—no pending DTCs from trainer
                    ESP_LOGI(TAG, "No pending DTC response (timeout).");
                    // make sure dtcs_bytes==0 so we push an empty list
                    dtcs_bytes = 0;
                }
                count = 1;
                for (int i = 0; i < dtcs_bytes; i+= 2){
                    ESP_LOGI(TAG, "Diagnostic Trouble code %i is 0x%02X 0x%02X", count, dtcs[i],dtcs[i + 1]);
                    count ++;
                }
                TC_Code_set(dtcs,dtcs_bytes); //passing trouble_code to the main function
                BLE_push_dtcs(dtcs, dtcs_bytes);
                xSemaphoreGive(DTCS_Loaded_sem); //set this to pass tc to uart after first go
                break;
            case SERV_PERM_DTCS:
                tx_action = TX_REQUEST_PERM_DTCS;
                xQueueSend(tx_task_queue,&tx_action, portMAX_DELAY);

                // wait for up to TIMEOUT_PERIOD_MS for any SF → semaphore
                if (xSemaphoreTake(TC_Grabbed_sem,
                      pdMS_TO_TICKS(TIMEOUT_PERIOD_MS)) == pdTRUE) {
                    // got at least one DTC frame
                } else {
                    // timeout—no pending DTCs from trainer
                    ESP_LOGI(TAG, "No pending DTC response (timeout).");
                    // make sure dtcs_bytes==0 so we push an empty list
                    dtcs_bytes = 0;
                }
                count = 1;
                for (int i = 0; i < dtcs_bytes; i+= 2){
                    ESP_LOGI(TAG, "Diagnostic Trouble code %i is 0x%02X 0x%02X", count, dtcs[i],dtcs[i + 1]);
                    count ++;
                }
                TC_Code_set(dtcs,dtcs_bytes); //passing trouble_code to the main function
                BLE_push_dtcs(dtcs, dtcs_bytes);
                xSemaphoreGive(DTCS_Loaded_sem); //tells uart dtcs loaded
                break;
            default:
                break;
        }
    }
}

//grabs tc from slave/trainer
void twai_TC_Get(){
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
    xTaskCreatePinnedToCore(TWAI_Services, "trouble_code", 4096, NULL, TROUBLE_CODE_TSK_PRIO, NULL, tskNO_AFFINITY);

    xTaskCreatePinnedToCore(live_stream_task, "live_stream", 2048, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);

    vTaskDelay(pdMS_TO_TICKS(75));
    // service_request_t current_serv = SERV_LD_DATA;
    // xQueueSend(service_queue,&current_serv,portMAX_DELAY);
}