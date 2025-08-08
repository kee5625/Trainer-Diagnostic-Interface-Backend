/**
 * Coder: Noah Batcher
 * Last updated: 6/11/2025
 * Project: Trainer Fault Code Diagnostic Gatway
 * Note: Code designed to send trouble code to devices over bluetooth to a serial port.
 * 
 * 
 */
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"
#include "BT_SPP_TC.h"
#include "TC_ref.h"

#include "time.h"
#include "sys/time.h"


#define SPP_TAG "SPP_Gateway"
#define SPP_SERVER_NAME "Gateway"
#define SPP_SHOW_DATA 0
#define SPP_SHOW_SPEED 1
#define SPP_SHOW_MODE SPP_SHOW_SPEED 
#define BT_SPP_CB_PRIO  5
#define BT_GAP_CB_PRIO  6

static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const bool esp_spp_enable_l2cap_ertm = true;
static uint32_t btHandle = 0;
static char trouble_code[tc_size + 2];

static void spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {

    switch (event) {
        case ESP_SPP_INIT_EVT:
            ESP_LOGI(SPP_TAG, "SPP initialized");
            esp_bt_gap_set_device_name(SPP_SERVER_NAME);
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            break;
        case ESP_SPP_START_EVT:
            ESP_LOGI(SPP_TAG, "SPP server started waiting for connetion");
            if (param->start.status == ESP_SPP_SUCCESS){
                btHandle = param->start.handle;
            }
            break;
        case ESP_SPP_SRV_OPEN_EVT:
            //give user message to provide input for fault code
            uint8_t *temp = (uint8_t *)"Enter anything anything for fault code.";
            esp_spp_write(btHandle,strlen((char *)temp),temp);
            break;
        case ESP_SPP_DATA_IND_EVT:
            if (strncmp((char *)param->data_ind.data, "ping",4)== 0){
                printf("%s Pinged...\n", SPP_TAG);
            }else{
                //sending trouble code to serial port
                esp_spp_write(btHandle,sizeof(trouble_code),(uint8_t *) trouble_code);
            }
            fflush(stdout);
            break;
        case ESP_SPP_WRITE_EVT:
            while(param->write.cong);//waiting for write congestion to go away
            break;
        case ESP_SPP_CONG_EVT:
            while (param->cong.cong); //waiting for congestion
            break;
        default:
            break;
    }

}

static void bt_gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {

    switch (event)
    {
    case ESP_BT_GAP_AUTH_CMPL_EVT:
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS){
            ESP_LOGI(SPP_TAG, "Authentication success: %s", param->auth_cmpl.device_name);
        }else{
            ESP_LOGW(SPP_TAG, "Authentication failed, status: %d", param->auth_cmpl.stat);
        }
        break;
    case ESP_BT_GAP_PIN_REQ_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
        if (param->pin_req.min_16_digit) {
            ESP_LOGI(SPP_TAG, "Input pin code: 0000 0000 0000 0000");
            esp_bt_pin_code_t pin_code = {0};

            esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
        } else {
            ESP_LOGI(SPP_TAG, "Input pin code: 1234");
            esp_bt_pin_code_t pin_code;
            pin_code[0] = '1';
            pin_code[1] = '2';
            pin_code[2] = '3';
            pin_code[3] = '4';
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
        }
        break;

#if (CONFIG_EXAMPLE_SSP_ENABLED == true)
    case ESP_BT_GAP_CFM_REQ_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %06"PRIu32, param->cfm_req.num_val);
        ESP_LOGW(SPP_TAG, "To confirm the value, type `spp ok;`");
        break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%06"PRIu32, param->key_notif.passkey);
        ESP_LOGW(SPP_TAG, "Waiting response...");
        break;
    case ESP_BT_GAP_KEY_REQ_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
        ESP_LOGW(SPP_TAG, "To input the key, type `spp key xxxxxx;`");
        break;
#endif

    case ESP_BT_GAP_MODE_CHG_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_MODE_CHG_EVT mode:%d", param->mode_chg.mode);
        break;
    default:
        break;
    }
}

void bt_spp_setup(char tc[tc_size + 2]){
    memcpy(trouble_code,tc,sizeof(trouble_code));
    /**
     * NVS flash setup
     */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));
   
    /**
     * Controller setup
     */
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    ESP_ERROR_CHECK(ret);
    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    ESP_ERROR_CHECK(ret);

    /**
    * initialze Bluedroid 
    */
    esp_bluedroid_config_t bluedriod_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    ret = esp_bluedroid_init_with_cfg(&bluedriod_cfg);
    ESP_ERROR_CHECK(ret);

    ret = esp_bluedroid_enable();
    ESP_ERROR_CHECK(ret);
    

    
    /**
    * gap callback setup
    */
    ret = esp_bt_gap_register_callback(bt_gap_callback);
    ESP_ERROR_CHECK(ret);
    /**
    * setting pin code 
    */
    esp_bt_pin_code_t pin_code;
    memcpy(pin_code,"1234",4);
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED; //variable doesn't work
    esp_bt_gap_set_pin(pin_type,4,pin_code);
    /**
    * initialize SPP
    */
    ret = esp_spp_register_callback(spp_callback); 
    ESP_ERROR_CHECK(ret);

    esp_spp_cfg_t bt_spp_cfg = {
        .mode = esp_spp_mode,
        .enable_l2cap_ertm = esp_spp_enable_l2cap_ertm,
        .tx_buffer_size = 0, /* Only used for ESP_SPP_MODE_VFS mode */
    };
    ret = esp_spp_enhanced_init(&bt_spp_cfg);
    ESP_ERROR_CHECK(ret);

    /**
    * Starting SPP server
    */
    ret = esp_spp_start_srv(ESP_SPP_SEC_AUTHENTICATE, ESP_SPP_ROLE_SLAVE, 0, SPP_SERVER_NAME);
    ESP_ERROR_CHECK(ret);
    vTaskDelay(portMAX_DELAY);
}
