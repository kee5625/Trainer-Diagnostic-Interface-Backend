/**
 * Coder: Noah Batcher
 * Last updated: 6/11/2025
 * Project: Trainer trouble Code Diagnostic Gatway
 * Note: Gateway designed to interact with trainer/car ECU to request DTCs, reset DTCs, and read live data
 * DTC (diagnostic trouble code) = TC (trouble code)
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


//twai driver setup
#define TIMEOUT_PERIOD_MS       1500
#define RX_TASK_PRIO            8
#define TX_TASK_PRIO            9
#define TROUBLE_CODE_TSK_PRIO   5
#define TX_GPIO_NUM             14
#define RX_GPIO_NUM             15
#define TAG                     "Gateway TWAI"

//twai config setup
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
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
    .flags = TWAI_MSG_FLAG_NONE,
    // Message ID and payload
    .identifier = ID_DT,
    .data_length_code = 8,
    .data = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    
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
static SemaphoreHandle_t TWAI_COMPLETE; //marks when TWAI done with curr_service
static SemaphoreHandle_t BIT_MASK_ROW_GRABED; //only for getting bitmask
static uint8_t *dtcs = NULL;
static int curr_BYTES = 0;
static uint8_t PIDs_Supported[7][4]; //filled with 0's in init (0 = unsupported, 1 = supported)
static uint8_t *PID_VALUE;
static uint8_t PID_VALUE_BYTES = 0;
static bool resetflag = false;




//*****************************************************Helper functions********************************************************/

static inline void send_FC_Frame(){
    twai_message_t TX_output = TWAI_Response_FC;
    twai_transmit(&TX_output, portMAX_DELAY);
    ESP_LOGI(TAG,"Sent flow control");
}

//returns index of mode response
static inline uint8_t mode_identify(twai_message_t msg, uint8_t last_service){
    if (msg.data[0] <= SINGLE_FRAME){ //For DTC modes
        return msg.data[1];

    }else if (msg.data[0] == MULT_FRAME_FIRST){ //For DTC modes
        return msg.data[2];

    }else if (msg.data[0] >= 0x41){ //For data modes
        return msg.data[0];

    }

    return last_service;  //note: This mean its a CF frame
}

static inline twai_message_t DTC_MSG_Req(uint8_t request){
    twai_message_t msg = TWAI_Request_msg;
    msg.data[1] = request;
    return msg;
}

static inline twai_message_t PID_MSG_REQ(uint8_t PID, uint8_t modeByte){
    twai_message_t msg = TWAI_Request_msg;
    msg.data[0] = 0x02;
    msg.data[1] = modeByte;
    msg.data[2] = PID;
    return msg;
}


//********************************************Functions for services/modes*********************************************************************************/

/**
 * Function Description: Comptelete all frame reading for all DTC modes (Pending, Stored, and Perminate).
 * 
 */
static int DTC_Frame_Reading(twai_message_t data, int num_bytes, int next_dtc){
    tx_task_action_t tx_response;

    if (data.data[0] == MULT_FRAME_FIRST){
        //setting num DTCs
        num_bytes = data.data[1];
        curr_BYTES = num_bytes - 1;
        dtcs = pvPortMalloc(curr_BYTES);

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
        curr_BYTES = num_bytes;
        dtcs = pvPortMalloc(curr_BYTES);

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

/**
 * Funciton Description: Complete all live data reqeust (available bit-mask and Value).
 * return 1 for bitmask row 0 for data pid -1 for error and when restflag = true
 */
static int Live_Data_Get(twai_message_t data){
    tx_task_action_t tx_response;
    int byte_count = 0;
    bool IS_BIT_MASK = data.data[2] % 0x20 == 0; //availability bit-masks not other data bit-masks for now 


    memset(PIDs_Supported, 0, sizeof(PIDs_Supported)); //reset bitmask
     

    while (1){

       ESP_LOGI(TAG,"In loop: %i", data.data[1]);                        //here not checking if it's the right response for the request create check in main that is set by UART
        if (IS_BIT_MASK || (data.data[0] < SINGLE_FRAME && (data.data[1] == SHOW_LIVE_DATA_RESP || data.data[1] == SHOW_FREEZE_FRAME_RESP) && (data.data[2] == get_Req_PID())) ){ //bit-mask frame or single frame
            if (IS_BIT_MASK){                                            //Available PID bit-mask only for now 
                int num_bitmask = (data.data[2] / 0x20); 
               
                for(int i = 0; i < 4; i++){                           
                    PIDs_Supported[num_bitmask][i] = data.data[i + 3]; 
                }
                return 1;//bitmask return

            }else{//PID data 
                PID_VALUE_BYTES = data.data[0] - 2;
                PID_VALUE = (uint8_t *)pvPortMalloc(PID_VALUE_BYTES);
                for (int i = 0; i < PID_VALUE_BYTES; i++){
                    PID_VALUE[i] = data.data[i + 3];
                }
            }

            return 0; //return for pid data 
        }else if (data.data[0] == MULT_FRAME_FIRST && (data.data[2] == SHOW_LIVE_DATA_RESP || data.data[2] == SHOW_FREEZE_FRAME_RESP) && get_Req_PID() == data.data[3]){ //below this is data responses 
            PID_VALUE = (uint8_t *)pvPortMalloc(data.data[1]);
            PID_VALUE_BYTES = data.data[1] - 2;
            for (int i = 0; i < 4; i++){
                PID_VALUE[i] = data.data[i + 4];
            }

            byte_count += 6; //mode and PID echo counted
            send_FC_Frame();

        }else if (data.data[0] >= MULT_FRAME_CON){
            for (int i = 0; i < PID_VALUE_BYTES - byte_count; i ++){
                PID_VALUE[byte_count] = data.data[i + 1];
                byte_count ++;
            }

            if (byte_count == PID_VALUE_BYTES){
                return 0; //return for pid data
            }

        }
        
        data.identifier = 0x00; //to get into while loop
        while (data.identifier < ID_ECU || data.identifier > ID_ECU_Second){
            ESP_LOGI(TAG,"Grabbing next message");

            twai_receive(&data,pdMS_TO_TICKS(1000));
            if (resetflag){
                return ERROR_TIMEOUT; //return for reset
            }   
        }
    }

}


//******************************************************General TWAI functions*****************************************************************/

static void twai_receive_task()
{
    tx_task_action_t tx_response;
    uint8_t num_bytes = 0;
    uint8_t temp_bytes = 0;
    uint8_t current_service = STORED_DTCS_RESP;
    twai_message_t RX_Data;
    esp_err_t rec_successful;
    int dataType = 0;
    int next_dtc = 4;

    while (1) {
        
        rec_successful = twai_receive(&RX_Data,pdMS_TO_TICKS(500));

        if (rec_successful == ESP_OK){

            if (RX_Data.identifier >= ID_ECU && RX_Data.identifier <= ID_ECU_Second){ 
                current_service = mode_identify(RX_Data,current_service);

                if (current_service == STORED_DTCS_RESP ||  current_service == PENDING_DTCS_RESP  || current_service == PERM_DTCS_RESP){
                    temp_bytes = num_bytes;
                    num_bytes = DTC_Frame_Reading(RX_Data,num_bytes,next_dtc);//------------------------------------------------------note: Different modes/services read frames differently
                    
                    if (num_bytes == 0){
                        next_dtc = 4;
                        ESP_LOGI(TAG,"DONE with dtcs");
                        xSemaphoreGive(TWAI_COMPLETE);
                    }else if (temp_bytes != 0){
                        next_dtc += temp_bytes - num_bytes;
                    }

                }else if (current_service == CLEAR_DTCS_GOOD_RESP){
                    ESP_LOGI(TAG,"DONE with clear dtcs");
                    xSemaphoreGive(TWAI_COMPLETE);

                }else if (current_service == CLEAR_DTCS_BAD_RESP){
                    

                }else if (current_service == SHOW_LIVE_DATA_RESP){
                    for (int i = 0; i < 8; i ++){
                        ESP_LOGI(TAG, "data %i: 0x%02X", i, RX_Data.data[i]);
                    }

                    dataType = Live_Data_Get(RX_Data);

                    if (dataType == 1){ //grabbing bitmask
                        xSemaphoreGive(BIT_MASK_ROW_GRABED);

                    }else if (dataType == 0){ //grabbing PID data
                        xSemaphoreGive(TWAI_COMPLETE);

                    }else if (dataType == ERROR_TIMEOUT){//error resetting
                        xSemaphoreGive(BIT_MASK_ROW_GRABED);
                        xSemaphoreGive(TWAI_COMPLETE);

                    }

                }else if (current_service == SHOW_FREEZE_FRAME_RESP){
                    for (int i = 0; i < 8; i ++){
                        ESP_LOGI(TAG, "data %i: 0x%02X", i, RX_Data.data[i]);
                    }

                    dataType = Live_Data_Get(RX_Data);

                    if (dataType == 1){ //grabbing bitmask
                        ESP_LOGI(TAG,"done with bitmask row");
                        xSemaphoreGive(BIT_MASK_ROW_GRABED);

                    }else if (dataType == 0){ //grabbing PID data
                        ESP_LOGI(TAG,"done with data grab");
                        xSemaphoreGive(TWAI_COMPLETE);

                    }else if (dataType == ERROR_TIMEOUT){//error resetting
                        ESP_LOGI(TAG,"TIMOUT sending semaphores");
                        xSemaphoreGive(BIT_MASK_ROW_GRABED);
                        xSemaphoreGive(TWAI_COMPLETE);

                    }
                }else{
                    ESP_LOGI(TAG,"TX_output rx: ID: %li, Data length: %i", RX_Data.identifier,RX_Data.data_length_code);
                    for (int i = 0; i < 8; i++){
                        ESP_LOGI(TAG,"rx Data %i: 0x%02X",i, RX_Data.data[i]);
                    }
                }
            }else{
                // ESP_LOGI(TAG,"Wrong identifier. ID: %li", RX_Data.identifier);
            }

        }else if (rec_successful == ESP_ERR_TIMEOUT){
            if (resetflag){
                ESP_LOGI(TAG,"RX reset time");
                xSemaphoreGive(TWAI_COMPLETE);
                xSemaphoreGive(BIT_MASK_ROW_GRABED); //give to service queue to give to reset function
            }
        }
    }
}

static void twai_transmit_task()
{
    tx_task_action_t action;
    twai_message_t TX_output;
    uint8_t Cur_Serv = 0;

    while (1) {
        ESP_LOGI(TAG,"Waiting for transmit*******************************************************************************");
        xQueueReceive(tx_task_queue, &action, portMAX_DELAY);
        switch(action){

        case TX_REQUEST_PIDS_Live:                                              //grabs full bit-mask for live data
        case TX_REQUEST_PIDS_Freeze: 

            if (action == TX_REQUEST_PIDS_Live){
                Cur_Serv = SHOW_LIVE_DATA_REQ;
            }else{
                Cur_Serv = SHOW_FREEZE_FRAME_REQ;
            }
    

            for (int i = 0x00; i < MAX_BITMASK_NUM; i += 0x20){                 //MAX_BITMASK_NUM set in TWAI_TC.h
                TX_output = PID_MSG_REQ( i, Cur_Serv);
                twai_transmit(&TX_output, portMAX_DELAY);
                xSemaphoreTake(BIT_MASK_ROW_GRABED, portMAX_DELAY);
                ESP_LOGI(TAG,"Cur_Serv %i",Cur_Serv);
                if (resetflag) break;
            }

            Set_PID_Bitmask(PIDs_Supported);                                   
            xSemaphoreGive(TWAI_COMPLETE);                                      
            break;

        case TX_REQUEST_DATA:
            TX_output = PID_MSG_REQ(get_Req_PID(), Cur_Serv);                   //grab current request PID from main 
            twai_transmit(&TX_output, portMAX_DELAY);
            break;

        case TX_REQUEST_STORED_DTCS:
            TX_output = DTC_MSG_Req(STORED_DTCS_REQ);
            twai_transmit(&TX_output, portMAX_DELAY);
            ESP_LOGI(TAG,"Sent stored DTCs request ");
            break;

        case TX_REQUEST_PENDING_DTCS:
            TX_output = DTC_MSG_Req(PENDING_DTCS_REQ);
            twai_transmit(&TX_output, portMAX_DELAY);
            ESP_LOGI(TAG,"Sent pending dtcs request");
            break;

        case TX_REQUEST_PERM_DTCS:
            TX_output = DTC_MSG_Req(PERM_DTCS_REQ);
            twai_transmit(&TX_output, portMAX_DELAY);
            ESP_LOGI(TAG,"Sent perminate dtcs request");
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

void TWAI_RESET(service_request_t req){
    ESP_LOGI(TAG,"Reseting.");

    //clear queues
    xQueueReset(tx_task_queue);
    xQueueReset(service_queue);

    resetflag = true;
    xSemaphoreTake(TWAI_DONE_sem, portMAX_DELAY); //current service done and TWAI turned off 
    resetflag = false;

    xSemaphoreTake(TWAI_COMPLETE, 0);        //taking pending semaphores
    xSemaphoreTake(BIT_MASK_ROW_GRABED, 0);

    if (req != TWAI_ERROR){
        //sending command again
        ESP_LOGI(TAG,"Reset done.");
        xQueueSend(service_queue, &req, portMAX_DELAY); //start command in queue
    }else{
        xQueueReset(tx_task_queue);
        xSemaphoreGive(TWAI_COMPLETE);
    }
     
}

//service_queue for external use to contorl TWIA
static void TWAI_Services()
{
    service_request_t current_serv;
    tx_task_action_t tx_action;
    int count = 0;
    twai_status_info_t status;
    
    while(1){
        xQueueReceive(service_queue,&current_serv,portMAX_DELAY);

        twai_get_status_info(&status);
        if (status.state == TWAI_STATE_STOPPED){
            ESP_LOGI(TAG,"TWAI started");
            twai_start();
        }

        switch (current_serv){
            case SERV_PIDS_LIVE:
                tx_action = TX_REQUEST_PIDS_Live;
                xQueueSend(tx_task_queue,&tx_action, portMAX_DELAY);
                ESP_LOGI(TAG,"SENT PID request");

                xSemaphoreTake(TWAI_COMPLETE,portMAX_DELAY);
                break;
            
            case SERV_PIDS_FREEZE:
                tx_action = TX_REQUEST_PIDS_Freeze;
                xQueueSend(tx_task_queue,&tx_action, portMAX_DELAY);
                ESP_LOGI(TAG,"SENT PID request");

                xSemaphoreTake(TWAI_COMPLETE,portMAX_DELAY);
                break;

            case SERV_DATA:
                tx_action = TX_REQUEST_DATA;
                xQueueSend(tx_task_queue,&tx_action, portMAX_DELAY);
                xSemaphoreTake(TWAI_COMPLETE,portMAX_DELAY);
                Set_PID_Value(PID_VALUE,PID_VALUE_BYTES);
                break;

            case SERV_FREEZE_DATA:

                break;

            case SERV_STORED_DTCS:
                tx_action = TX_REQUEST_STORED_DTCS;
                xQueueSend(tx_task_queue,&tx_action, portMAX_DELAY);
              
                xSemaphoreTake(TWAI_COMPLETE,portMAX_DELAY);

                Set_DTCs(dtcs,curr_BYTES); //passing trouble_code to the main function

                /**
                 * Reset dtcs after main gets deep copy
                 */
                vPortFree(dtcs);
                dtcs = NULL;
                curr_BYTES = 0;
                break;

            case SERV_CLEAR_DTCS://UART file resets everything
                dtcs = NULL;
                curr_BYTES = 0;
                tx_action = TX_RESET_DTCs;
                xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);

                xSemaphoreTake(TWAI_COMPLETE,portMAX_DELAY);
                break;

            case SERV_PENDING_DTCS:
                tx_action = TX_REQUEST_PENDING_DTCS;
                xQueueSend(tx_task_queue,&tx_action, portMAX_DELAY);

                xSemaphoreTake(TWAI_COMPLETE,portMAX_DELAY);
                count = 1;

                Set_DTCs(dtcs,curr_BYTES); //passing trouble_code to the main function

                /**
                 * Reset dtcs after main gets deep copy
                 */
                vPortFree(dtcs);
                dtcs = NULL;
                curr_BYTES = 0;
                break;

            case SERV_PERM_DTCS:
                tx_action = TX_REQUEST_PERM_DTCS;
                xQueueSend(tx_task_queue,&tx_action, portMAX_DELAY);

                xSemaphoreTake(TWAI_COMPLETE,portMAX_DELAY);
                count = 1;

                Set_DTCs(dtcs,curr_BYTES); //passing trouble_code to the main function

                /**
                 * Reset dtcs after main gets deep copy
                 */
                vPortFree(dtcs);
                dtcs = NULL;
                curr_BYTES = 0;
                break;

            default:
                break;
        }
         
        twai_get_status_info(&status);
        if (status.state == TWAI_STATE_RUNNING){
            twai_stop();
            ESP_LOGI(TAG,"TWAI stopped");
        }

        xSemaphoreGive(TWAI_DONE_sem); //tells uart TWAI is done
    }

}

//grabs tc from slave/trainer
void TWAI_INIT(){
    //Create tasks, queues, and semaphores
    tx_task_queue = xQueueCreate(1, sizeof(tx_task_action_t));
    TWAI_COMPLETE = xSemaphoreCreateBinary();
    BIT_MASK_ROW_GRABED = xSemaphoreCreateBinary();

    //Install TWAI driver
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(TAG, "Driver installed");

    xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(twai_transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(TWAI_Services, "trouble_code", 4096, NULL, TROUBLE_CODE_TSK_PRIO, NULL, tskNO_AFFINITY);

    vTaskDelay(pdMS_TO_TICKS(75));
}