#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"
//#include "UART_SEND.h"
#include "TWAI_OBD.h"

#define RX_TASK_PRIO                    8       //Receiving task priority
#define TX_TASK_PRIO                    9       //Sending task priority
#define CTRL_TSK_PRIO                   10      //Control task priority
#define TX_GPIO_NUM                     21
#define RX_GPIO_NUM                     22
#define TAG                             "ECM/Trainer"

typedef enum {
    TX_REQUEST_TC,
    TX_FLOW_CONTROL_RESPONSE,
    TX_TASK_EXIT,
} tx_task_action_t;

typedef enum {
    RX_RECEIVE_TC,
    RX_RECEIVE_LIVE_DATA,
    RX_TASK_EXIT,
} rx_task_action_t;

static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS(); //change! to 125k - 1M was at 25k
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

static uint8_t stored_dtcs[6] = {0x03,0x04,0x02,0x00,0x04,0x00};
static uint8_t pending_dtcs[12] = {0x03,0x04,0x02,0x00,0x04,0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00};
static uint8_t perminate_dtcs[4] = {0x09, 0x00, 0x0A,0x00};
static uint8_t live_data[6];
static const uint8_t TEST_IGNITIONS[]    = { 0, 1 };
static const uint8_t TEST_SEAT_ADJ[]     = { 0, 2, 4, 1, 3 };
static const uint8_t TEST_LUMBAR_ADJ[]   = { 0, 1, 2, 3, 4 };

static int CF_num = 0; 
static int frames_Before_FC = 0;
static QueueHandle_t tx_task_queue;
static SemaphoreHandle_t TC_sent_sem;
static SemaphoreHandle_t FC_Frame_sem;
static SemaphoreHandle_t start_sem;

static const twai_message_t TWAI_MSG = {
    // Message type and format settings
    .extd = 0,              // Standard Format message (11-bit ID)
    .rtr = 0,               // Send a data frame
    .ss = 0,                // Not single shot
    .self = 0,              // Not a self reception request
    .dlc_non_comp = 0,      // DLC is less than 8
    // Message ID and payload
    .identifier = ID_ECU,
    .data_length_code = 8,
    .data = {0x99,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    //data overriden by TWAI_setup(); 
};

/* --------------------------- Tasks and Functions -------------------------- */
static inline twai_message_t TWAI_Clear_DTCS(){ //good and bad response not implemented
    twai_message_t msg = TWAI_MSG;
    msg.data[0] = 0x01;
    msg.data[1] = CLEAR_DTCS_GOOD_RESP;
    for (int i = 2; i < 6; i++){
        msg.data[i] = 0x00;
    }
    return msg;
}
static uint8_t read_ignition(void)     { return 1;   /* ON=1, OFF=0 */ }
static uint8_t read_seat_adjuster(void) { return 2;   /* UP=2 */ }
static uint8_t read_lumbar_adjuster(void){ return 3;   /* LEFT=3 */ }

static void refresh_live_data(void) {
    static size_t idx = 0;
    // cycle idx through 0…n−1
    idx = (idx + 1) % 5;  

    live_data[0] = 0xA0;
    live_data[1] = TEST_IGNITIONS[idx % (sizeof(TEST_IGNITIONS)/1)];

    live_data[2] = 0xA1;
    live_data[3] = TEST_SEAT_ADJ[idx];

    live_data[4] = 0xA2;
    live_data[5] = TEST_LUMBAR_ADJ[(idx+2) % (sizeof(TEST_LUMBAR_ADJ)/1)];
}

/**
 * Function Description: Passed the desired request from TWAI_OBD.h this will create a corret frame for it. To send a multi frame message call
 * this function for every frame. 
 */
static twai_message_t TWAI_setup(uint8_t response){
    twai_message_t message = TWAI_MSG; 
    int bytes = 0;
    int start = 0;
    int temp = 0; //only used for math of consecutive frames
    int con_frame = 4;
    uint8_t *dtcs_ptr = NULL;
    //setting which set of dtcs to send, responseonse, and finding size of dtcs list
    if (response == STORED_DTCS_REQ){
        response = STORED_DTCS_RESP;
        bytes = sizeof(stored_dtcs);
        dtcs_ptr = stored_dtcs;
    }else if (response == PENDING_DTCS_REQ){
        bytes = sizeof(pending_dtcs);
        response = PENDING_DTCS_RESP;
        dtcs_ptr = pending_dtcs;
    }else if (response == PERM_DTCS_REQ){
        bytes = sizeof(perminate_dtcs);
        response = PERM_DTCS_RESP;
        dtcs_ptr = perminate_dtcs;
    }else if (response == CLEAR_DTCS_REQ){
        ESP_LOGI(TAG,"%i", response);
        message = TWAI_Clear_DTCS();
        return message;
    }
    else if (response == SHOW_LIVE_DATA_REQ) {
        response = SHOW_LIVE_DATA_RESP;
        refresh_live_data();
        bytes = sizeof(live_data);
        dtcs_ptr = live_data;
    }

    //setting frame type
    if (bytes <= 6){ //single frame
        start = 2;
        con_frame = 0;
        message.data[0] = bytes + 2;
        message.data[1] = response;
    }else if (CF_num == 0){ //first multi frame 
        message.data[0] = MULT_FRAME_FIRST;
        message.data[1] = bytes + 1; //add one for mode byte
        message.data[2] = response;
        message.data[7] = 0x00; //padding only for dtcs
        bytes = 4;
        start = 3;
        con_frame = 0;
    }else{ //next multi frame 
        bytes = (bytes - (CF_num * 6 - 2) >= 6) ? 6 : bytes - (CF_num  * 6 - 2);
        start = 1;
        message.data[0] = MULT_FRAME_CON + CF_num;
        message.data[7] = 0x00; //padding only for dtcs
    }
    
    //setting data (dtcs)
    if (dtcs_ptr != NULL){
        temp = (CF_num != 0) ? (CF_num * 6) - 2 : 0; //math for consecutive frames only
        for (int i = start; i <= bytes + start - 1; i+= 2){
            message.data[i] = *(dtcs_ptr + (i - start) + temp);
            message.data[i + 1] = *(dtcs_ptr + (i - start) + temp + 1);
        }
    }else{
        ESP_LOGE(TAG, "Unknown response! %i", response);
    }
    CF_num ++;
    return message;
}





static void twai_receive_task(void *arg)
{   
    twai_message_t tx_action;
    twai_message_t rx_action;
    uint8_t mode_req;
    uint8_t last_mode_req;
    int num_bytes;
    xSemaphoreTake(start_sem,portMAX_DELAY);
    while (1) {
        if (twai_receive(&rx_action,portMAX_DELAY) == ESP_OK){
            num_bytes = rx_action.data[0]; //number of bytes
            if (num_bytes > 0 && num_bytes <= SINGLE_FRAME){ // 1 byte is most request but some will need 2 bytes
                mode_req = rx_action.data[1]; //mode request
            }else if (num_bytes == MULT_FRAME_FLOW){
                mode_req = (uint8_t) MULT_FRAME_FLOW;
            }else{  
                mode_req = 999999; //unsupported
            }

            switch(mode_req){
                case STORED_DTCS_REQ:
                    frames_Before_FC = 0;
                    CF_num = 0;
                    /* enqueue FIRST frame */
                    tx_action = TWAI_setup(mode_req);
                    xQueueSend(tx_task_queue,&tx_action,portMAX_DELAY);
                    ESP_LOGI(TAG,"Received stored dtcs request");
                    /* enqueue EVERY remaining CF */
                    while((CF_num - 1) * 6 < sizeof(stored_dtcs)){
                        tx_action = TWAI_setup(mode_req);
                        ESP_LOGI(TAG,"Enqueueing frame %02X", tx_action.data[0]);
                        xQueueSend(tx_task_queue,&tx_action,portMAX_DELAY);
                    }

                    break;
                case CLEAR_DTCS_REQ:
                    frames_Before_FC = 0;
                    CF_num = 0;
                    tx_action = TWAI_setup(mode_req);
                    ESP_LOGI(TAG,"Received clear dtcs request %i", tx_action.data[1]);
                    xQueueSend(tx_task_queue,&tx_action,portMAX_DELAY);
                    break;
                case PENDING_DTCS_REQ:
                    frames_Before_FC = 0;
                    CF_num = 0;
                    tx_action = TWAI_setup(mode_req);
                    ESP_LOGI(TAG,"Received pending dtcs request");
                    xQueueSend(tx_task_queue,&tx_action,portMAX_DELAY);
                    for (int i = 4; i < sizeof(pending_dtcs); i += 6){
                        tx_action = TWAI_setup(mode_req);
                        xQueueSend(tx_task_queue,&tx_action,portMAX_DELAY);
                    }
                    break;
                case PERM_DTCS_REQ:
                    frames_Before_FC = 0;
                    CF_num = 0;
                    tx_action = TWAI_setup(mode_req);
                    ESP_LOGI(TAG,"Received perminate dtcs request");
                    xQueueSend(tx_task_queue,&tx_action,portMAX_DELAY);
                    while((CF_num - 1) * 6 < sizeof(perminate_dtcs)){
                        tx_action = TWAI_setup(mode_req);
                        ESP_LOGI(TAG,"Enqueueing frame %02X", tx_action.data[0]);
                        xQueueSend(tx_task_queue,&tx_action,portMAX_DELAY);
                    }
                    break;
                case MULT_FRAME_FLOW:
                    ESP_LOGI(TAG,"Received flow control frame");
                    //rx_action.data[0] == 0x00 means keep sending
                    if (rx_action.data[0] == 0x01){ //wait
                        //wait for next FC to be sent
                        break;
                    }else if (rx_action.data[0] == 0x02){ //overflow/abort
                        ESP_LOGI(TAG,"Request error out.");
                        break;
                    }
                    frames_Before_FC = rx_action.data[1];
                    ESP_LOGI(TAG, "Flow control frame received, ready to continue.");
                    xSemaphoreGive(FC_Frame_sem);
                    break;
                case SHOW_LIVE_DATA_REQ:
                    frames_Before_FC = 0;
                    CF_num = 0;
                    refresh_live_data();
                    /* enqueue first multi-frame (or single, if <6 bytes) */
                    tx_action = TWAI_setup(mode_req);
                    xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
                    /* enquue additional CFs*/
                    while((CF_num - 1) * 6 < sizeof(live_data)){
                        tx_action = TWAI_setup(mode_req);
                        xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
                    }
                    ESP_LOGI(TAG, "Responding to live data request");
                    break;
                default:
                    ESP_LOGI(TAG, "UNKNOWN COMMAND: %i %i",rx_action.data[0], rx_action.data[1]);
                    //error frame because unsuported frame recieved 
                    break;
            }
            last_mode_req = mode_req;
        }
    }
}

static void twai_transmit_task(void *arg)
{   
    int start = 2;
    twai_message_t action;
    while (1) {
        xQueueReceive(tx_task_queue, &action, portMAX_DELAY);
        ESP_LOGI(TAG,"Transmitting");
        twai_transmit(&action, portMAX_DELAY);
        if (action.data[0] == MULT_FRAME_FIRST){
            ESP_LOGI(TAG,"First frame of multi-frame message sent.");
            start = 3;
        }else if (action.data[0] >= MULT_FRAME_CON){
            ESP_LOGI(TAG,"Consecutive frame number %i sent.", action.data[0] - MULT_FRAME_CON);
            start = 1;
        }else if (action.data[1] == CLEAR_DTCS_GOOD_RESP){
            ESP_LOGI(TAG,"Sent good DTCs clear response.");
        }else if (action.data[0] <= SINGLE_FRAME){
            ESP_LOGI(TAG,"Single frame sent.");
            start = 2;
        }
        ESP_LOGI(TAG,"First byte: 0x%02X",action.data[0]);
        for (int i = start; i <= 6; i += 2){
            ESP_LOGI(TAG, "Trouble code 0x%02X 0x%02X", action.data[i],action.data[i + 1]);
        }
        if (action.data[0] == MULT_FRAME_FIRST || ((CF_num != 0) && (frames_Before_FC != 0) && (CF_num % frames_Before_FC == 0))){
            xSemaphoreTake(FC_Frame_sem,portMAX_DELAY);
        }
    }
}

void app_main(void)
{    
    //Install TWAI driver, trigger tasks to start
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(TAG, "Driver installed");


    ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(TAG,"TWAI started.");


    //Create semaphores and tasks
    tx_task_queue = xQueueCreate(20, sizeof(twai_message_t));
    TC_sent_sem  = xSemaphoreCreateBinary();
    FC_Frame_sem = xSemaphoreCreateBinary();
    start_sem = xSemaphoreCreateBinary();
    xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(twai_transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
   
    //UART_Start();
    xSemaphoreGive(start_sem);
}