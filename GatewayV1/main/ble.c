#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "driver/gpio.h"

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------
static const char *TAG = "PowerSeat";   //Log tag
// Custom UUIDs (16‑bit for brevity, upgrade to 128‑bit if needed)
#define TRAINER_SERVICE_UUID        0xABCD    //primary service
#define TRAINER_UUID_TX             0xAB01    // TX Request (Write / WNR)
#define TRAINER_UUID_RX             0xAB02    // RX Response(Notify / Read)
#define TRAINER_UUID_CTRL           0xAB03    // Control char (Read / Write)

// Service handle count: svc + 3x(char + cccd) = 8
#define TRAINER_NUM_HANDLES 8


// Atribute Handles
static uint8_t hdl_tx = 0;
static uint8_t hdl_rx = 0;
static uint8_t hdl_ctrl = 0;

// Characteristic definitions
static const esp_gatt_char_prop_t PROP_TX = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
static const esp_gatt_char_prop_t PROP_RX = ESP_GATT_CHAR_PROP_BIT_READ;
static const esp_gatt_char_prop_t PROP_CTRL = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE;

static const uint8_t ctrl_init_val[] = { 0x01 };    //protocol v1;
static esp_attr_value_t ctrl_attr_val = {
    .attr_max_len = 20,
    .attr_len = sizeof(ctrl_init_val),
    .attr_value = (uint8_t *)ctrl_init_val,
};

// PROFILE STRUCT
typedef struct {
    esp_gatts_cb_t  gatts_cb;
    uint16_t        gatts_if;
    uint16_t        app_id;
    uint16_t        conn_id;
    uint16_t        service_hdl;
    esp_gatt_srvc_id_t service_id;
} trainer_profile_t;

static void trainer_profile_cb(esp_gatts_cb_event_t, esp_gatt_if_t, esp_ble_gatts_cb_param_t *);

static trainer_profile_t profile = {
    .gatts_cb = trainer_profile_cb,
    .gatts_if = ESP_GATT_IF_NONE,
    .app_id   = 0,
};

// -----------------------------------------------------------------------------
// Helper macros
// -----------------------------------------------------------------------------
#define GATT_CHAR_PROP_RD_WR  (ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE)

// -----------------------------------------------------------------------------
// GAP (advertising)
// -----------------------------------------------------------------------------

static uint8_t adv_service_uuid128[16] = {
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, (TRAINER_SERVICE_UUID & 0xFF), (TRAINER_SERVICE_UUID >> 8), 0x00, 0x00,
};

static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static void gap_event_handler(esp_gap_ble_cb_event_t evt, esp_ble_gap_cb_param_t *param)
{
    switch(evt){
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            esp_ble_gap_start_advertising(&adv_params);
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            ESP_LOGI(TAG, "Advertising %s", param->adv_start_cmpl.status==ESP_BT_STATUS_SUCCESS?"started":"failed");
            break;
        default:
            break;
    }
}


// -----------------------------------------------------------------------------
// GATT Profile Callback
// -----------------------------------------------------------------------------
static void trainer_profile_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param){
    if (event == ESP_GATTS_WRITE_EVT) {
        ESP_LOGI(TAG, "WRITE_EVT handle=0x%04X, len=%u, need_rsp=%u",
                param->write.handle, param->write.len, param->write.need_rsp);
    }
    
    switch(event){
    // ───── Registration ─────
    case ESP_GATTS_REG_EVT: {
        ESP_LOGI(TAG, "GATTS_REG_EVT, app_id %d", param->reg.app_id);
        profile.gatts_if = gatts_if;

        profile.service_id.is_primary = true;
        profile.service_id.id.inst_id = 0;
        profile.service_id.id.uuid.len = ESP_UUID_LEN_16;
        profile.service_id.id.uuid.uuid.uuid16 = TRAINER_SERVICE_UUID;

        // Device name comes from menuconfig or NVS
        esp_ble_gap_set_device_name(CONFIG_TRAINER_DEVICE_NAME);
        esp_ble_gap_config_adv_data(&adv_data);

        esp_ble_gatts_create_service(gatts_if, &profile.service_id, TRAINER_NUM_HANDLES);
        break; }

    // ───── Service created ─────
    case ESP_GATTS_CREATE_EVT: {
        profile.service_hdl = param->create.service_handle;
        esp_ble_gatts_start_service(profile.service_hdl);

        // TX Characteristic (client→ESP)
        esp_bt_uuid_t tx_uuid = { .len = ESP_UUID_LEN_16, .uuid.uuid16 = TRAINER_UUID_TX };
        esp_ble_gatts_add_char(profile.service_hdl, &tx_uuid,
                               ESP_GATT_PERM_WRITE,
                               PROP_TX, NULL, NULL);
        // RX Characteristic (ESP→client)
        esp_bt_uuid_t rx_uuid = { .len = ESP_UUID_LEN_16, .uuid.uuid16 = TRAINER_UUID_RX };
        esp_ble_gatts_add_char(profile.service_hdl, &rx_uuid,
                               ESP_GATT_PERM_READ,
                               PROP_RX, NULL, NULL);
        // Control Characteristic (optional)
        esp_bt_uuid_t ctrl_uuid = { .len = ESP_UUID_LEN_16, .uuid.uuid16 = TRAINER_UUID_CTRL };
        esp_ble_gatts_add_char(profile.service_hdl, &ctrl_uuid,
                               ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                               PROP_CTRL, &ctrl_attr_val, NULL);
        break; }

    // ───── Characteristic added ─────
    case ESP_GATTS_ADD_CHAR_EVT: {
        switch (param->add_char.char_uuid.uuid.uuid16) {
        case TRAINER_UUID_TX:
            hdl_tx = param->add_char.attr_handle;
            break;

        case TRAINER_UUID_RX:
            hdl_rx = param->add_char.attr_handle;
            // *** Do not add a second CCCD here ***
            break;

        case TRAINER_UUID_CTRL:
            hdl_ctrl = param->add_char.attr_handle;
            break;
        }
        break;
    }
    // ───── Write to TX / CTRL ─────
    case ESP_GATTS_WRITE_EVT: {
        const uint8_t *d = param->write.value;
        uint16_t len = param->write.len;
        if(param->write.need_rsp){
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        }
        if(param->write.handle == hdl_tx){
            // ✦ Handle request from client
            ESP_LOG_BUFFER_HEX_LEVEL(TAG, d, len, ESP_LOG_DEBUG);
            // TODO: forward to CAN & schedule response
        }else if(param->write.handle == hdl_ctrl){
            // ✦ Handle control commands (e.g. OTA, alias table update)
        }
        break; }

    // ───── Read from CTRL (or RX if someone reads history) ─────
    case ESP_GATTS_READ_EVT: {
        ESP_LOGD(TAG, "Read handle 0x%X", param->read.handle);
        break; }

    // ───── Client configures notifications on RX ─────
    case ESP_GATTS_CONF_EVT:
        ESP_LOGI(TAG, "Client confirmed notify, status %d", param->conf.status);
        break;

    // ───── Connections ─────
    case ESP_GATTS_CONNECT_EVT:
        profile.conn_id = param->connect.conn_id;
        ESP_LOGI(TAG, "Client connected, conn_id %d", profile.conn_id);
        break;
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(TAG, "Client disconnected, reason 0x%02X", param->disconnect.reason);
        esp_ble_gap_start_advertising(&adv_params);
        break;
    default:
        break;
    }
}

// -----------------------------------------------------------------------------
//                           GATT dispatcher (1 profile)
// -----------------------------------------------------------------------------
static void gatts_event_handler(esp_gatts_cb_event_t evt, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    if(evt == ESP_GATTS_REG_EVT && param->reg.status == ESP_GATT_OK){
        profile.gatts_if = gatts_if;
    }
    // forward to profile if it matches
    if(profile.gatts_cb && (gatts_if == profile.gatts_if || gatts_if == ESP_GATT_IF_NONE)){
        profile.gatts_cb(evt, gatts_if, param);
    }
}

// -----------------------------------------------------------------------------
// Application entry point
// -----------------------------------------------------------------------------
void ble_connect()
{
    // 1. NVS & BLE controller init
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    // 2. Register callbacks
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(profile.app_id));

    // 3. Increase MTU for bigger payloads
    esp_ble_gatt_set_local_mtu(247);

    ESP_LOGI(TAG, "Trainer-Gateway BLE ready ⸻ name: %s", CONFIG_TRAINER_DEVICE_NAME);    
}
