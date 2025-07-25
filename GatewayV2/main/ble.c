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
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "driver/gpio.h"

#include "ble.h"
#include "TC_ref.h"
#include "esp_gatts_api.h"

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

/* ---------- logging helpers (common) ---------- */
#define LOG_FRAME(dir, msg)                                                      \
    ESP_LOGI(TAG,                                                                \
        dir " id=0x%03X dlc=%d "                                                 \
        "%02X %02X %02X %02X %02X %02X %02X %02X",                               \
        (unsigned int)(msg).identifier, (msg).data_length_code,                  \
        (msg).data[0], (msg).data[1], (msg).data[2], (msg).data[3],              \
        (msg).data[4], (msg).data[5], (msg).data[6], (msg).data[7])

#define LOG_BIN(label, val)  ESP_LOGI(TAG, label " = %d (0x%02X)", (val), (val))

//globals used by rest of firmware
static bool            conn_active   = false;
static bool            notify_on     = false;
static esp_gatt_if_t   gatts_if_save = ESP_GATT_IF_NONE;
static uint16_t        rx_cccd_hdl   = 0;

extern QueueHandle_t service_queue;
extern bool stream_on_master;

// Atribute Handles
uint8_t hdl_tx = 0;
uint8_t hdl_rx = 0;
uint8_t hdl_ctrl = 0;

// Characteristic definitions
static const esp_gatt_char_prop_t PROP_TX = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
static const esp_gatt_char_prop_t PROP_RX = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY | ESP_GATT_CHAR_PROP_BIT_INDICATE;
static const esp_gatt_char_prop_t PROP_CTRL = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE;

static const uint8_t ctrl_init_val[] = { 0x01 };    //protocol v1;
static esp_attr_value_t ctrl_attr_val = {
    .attr_max_len = 20,
    .attr_len = sizeof(ctrl_init_val),
    .attr_value = (uint8_t *)ctrl_init_val,
};


static void trainer_profile_cb(esp_gatts_cb_event_t, esp_gatt_if_t, esp_ble_gatts_cb_param_t *);

trainer_profile_t profile = {
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

uint8_t cccd_val[2] = {0x00, 0x00};
esp_attr_value_t cccd_attr_val = {
    .attr_max_len = 2,
    .attr_len     = 2,
    .attr_value   = cccd_val,
};

static void gap_event_handler(esp_gap_ble_cb_event_t evt, esp_ble_gap_cb_param_t *param)
{
    switch(evt){
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            //esp_ble_gap_start_advertising(&adv_params);
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            ESP_LOGI(TAG, "Advertising %s", param->adv_start_cmpl.status==ESP_BT_STATUS_SUCCESS?"started":"failed");
            break;
        default:
            break;
    }
}

/* Forward so Gateway-main can ask us to signal bytes */
bool BLE_push_dtcs(const uint8_t *bytes, uint8_t len)
{
    if (!conn_active || !notify_on) return false;

    /* Break long arrays into <20-byte MTU chunks; send as indications
       so we get ACKs and the buffer doesn’t overrun. */
    while (len) {
        uint16_t part = (len > 20) ? 20 : len;
        esp_ble_gatts_send_indicate(gatts_if_save,
                                    /*conn_id*/ profile.conn_id,
                                    /*attr_hdl*/ hdl_rx,
                                    part,
                                    (uint8_t*)bytes,
                                    /*need_confirm*/ true);
        bytes += part;
        len   -= part;
    }
    return true;
}

void BLE_notify_clear(void)
{
    static const uint8_t ok = 0xCC;   /* arbitrary “clear done” flag */
    BLE_push_dtcs(&ok, 1);
}

/* ────────────────────────────────────────────────────────────
 *  1. Map BLE-writes → same service_queue the UART already uses
 * ──────────────────────────────────────────────────────────── */

static void handle_ui_cmd(uint8_t byte)
{
    /* Minimal protocol (expand as you like):
     *   0x01  = “give me pending DTCs”
     *   0x02  = “give me stored DTCs”
     *   0x03  = “give me permanent DTCs”
     *   0x04  = “clear codes”
     */
    
    ESP_LOGI(TAG,"[BLE] cmd 0x%02X → enqueue service_queue", byte);
    service_request_t s;
    
    switch (byte) {
        case 0x01: s = SERV_PENDING_DTCS; break;
        case 0x02: s = SERV_STORED_DTCS;  break;
        case 0x03: s = SERV_PERM_DTCS;    break;
        case 0x04: s = SERV_CLEAR_DTCS;   break;
        case 0x05: s = SERV_LD_DATA;     break;    //one time "Get Data"
        case 0x06:
            stream_on_master = !stream_on_master;
            ESP_LOGI(TAG, "Live stream %s", stream_on_master?"ON":"OFF");
            return;
        default:   return;                      /* ignore unknown op    */
    }
    ESP_LOGI(TAG, "BLE cmd 0x%02X mapped to svc=%d", byte, s);
    if (service_queue) {
       // block until the service task picks it up
       BaseType_t res = xQueueSend(service_queue, &s, pdMS_TO_TICKS(100));
       ESP_LOGI(TAG, "[BLE] xQueueSend(service_queue) → %s",
          res == pdPASS ? "OK" : (res == errQUEUE_FULL ? "FULL" : "ERR"));
   }
}

        

// -----------------------------------------------------------------------------
// GATT Profile Callback
// -----------------------------------------------------------------------------
static void trainer_profile_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param){    
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

        // RX Characteristic (ESP→client)
        esp_bt_uuid_t rx_uuid = { .len = ESP_UUID_LEN_16, .uuid.uuid16 = TRAINER_UUID_RX };
        esp_err_t err = esp_ble_gatts_add_char(
            profile.service_hdl, &rx_uuid,
            ESP_GATT_PERM_READ,
            PROP_RX,
            NULL, NULL);
        if (err) ESP_LOGE(TAG,"add RX char failed (%s)", esp_err_to_name(err));
        
        const uint16_t cccd_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG; // 0x2902
        esp_bt_uuid_t  descr_uuid = { .len = ESP_UUID_LEN_16, .uuid = { .uuid16 = cccd_uuid } };

        err = esp_ble_gatts_add_char_descr(
            profile.service_hdl, &descr_uuid,
            ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            &cccd_attr_val, NULL);
        if (err) ESP_LOGE(TAG,"add CCCD failed (%s)", esp_err_to_name(err));

        // TX Characteristic (client→ESP)
        esp_bt_uuid_t tx_uuid = { .len = ESP_UUID_LEN_16, .uuid.uuid16 = TRAINER_UUID_TX };
        esp_ble_gatts_add_char(profile.service_hdl, &tx_uuid,
                               ESP_GATT_PERM_WRITE,
                               PROP_TX, NULL, NULL);
        
        // Control Characteristic (optional)
        esp_bt_uuid_t ctrl_uuid = { .len = ESP_UUID_LEN_16, .uuid.uuid16 = TRAINER_UUID_CTRL };
        esp_ble_gatts_add_char(profile.service_hdl, &ctrl_uuid,
                               ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                               PROP_CTRL, &ctrl_attr_val, NULL);
        break; }

    
    case ESP_GATTS_ADD_CHAR_EVT: {
        
        if (param->add_char.char_uuid.uuid.uuid16 == TRAINER_UUID_TX)
            hdl_tx = param->add_char.attr_handle;
        else if (param->add_char.char_uuid.uuid.uuid16 == TRAINER_UUID_RX)
            hdl_rx = param->add_char.attr_handle;
        else if (param->add_char.char_uuid.uuid.uuid16 == TRAINER_UUID_CTRL){
            hdl_ctrl = param->add_char.attr_handle;
        }
        break;
    }
    case ESP_GATTS_ADD_CHAR_DESCR_EVT: {
        /* first descriptor we add is the CCCD for RX characteristic */
        rx_cccd_hdl = param->add_char_descr.attr_handle;
        /*start advertising*/
        esp_ble_gap_start_advertising(&adv_params);
        ESP_LOGI(TAG, "Advertising started");
        break;
    }
    // ───── Write to TX / CTRL ─────
    case ESP_GATTS_WRITE_EVT: {
        const uint8_t *d = param->write.value;
        uint16_t len = param->write.len;
        
        
        //Log event handler trigger
        ESP_LOGI(TAG, "[BLE] WRITE_EVT handle=0x%04X, len=%u, need_rsp=%u",
                param->write.handle, param->write.len, param->write.need_rsp);

        if(param->write.need_rsp){
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        }
        if (param->write.handle == hdl_tx) {
            if (param->write.len) handle_ui_cmd(param->write.value[0]);

        }else if (param->write.handle == rx_cccd_hdl && param->write.len >= 2) {
            /* 0x0001 = notifications, 0x0002 = indications*/
            uint16_t cfg = param->write.value[0] | (param->write.value[1] << 8);
            notify_on = (cfg != 0);
        }               
        break; }

    // ───── Read from CTRL (or RX if someone reads history) ─────
    case ESP_GATTS_READ_EVT: {
        ESP_LOGD(TAG, "Read handle 0x%X", param->read.handle);
        break; }

    // ───── Client configures notifications on RX ─────
    case ESP_GATTS_CONF_EVT:
        if (param->conf.status == ESP_GATT_OK) notify_on = true;
        ESP_LOGI(TAG, "Client confirmed notify, status %d", param->conf.status);
        break;

    // ───── Connections ─────
    case ESP_GATTS_CONNECT_EVT:
        profile.conn_id = param->connect.conn_id;
        ESP_LOGI(TAG, "Client connected (conn_id=%d)", profile.conn_id);
        conn_active   = true;
        gatts_if_save = gatts_if;   /* stash for later notifies */
        /* … start CAN if you still do that here … */
        break;
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(TAG, "Client disconnected (reason 0x%02X)", param->disconnect.reason);
        conn_active = notify_on = false;
        stream_on_master = false;
        ESP_LOGI(TAG, "[BLE] Reset stream_on_master=%d", stream_on_master);
        esp_ble_gap_start_advertising(&adv_params);   // resume beaconing
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
void BLE_init()
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