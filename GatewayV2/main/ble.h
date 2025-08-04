/*  ble.h  – public hooks the rest of the gateway can call  */
#pragma once

#ifndef TRAINER_BLE_H
#define TRAINER_BLE_H

#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"
#include <stdint.h>
#include <stdbool.h>
#include "TC_ref.h"
#include "TWIA_TC.h"

void BLE_init(void);

/* Push a freshly-received DTC blob up to the mobile / web app             */
/* - returns true if a Central is connected & notifications are enabled    */
bool BLE_push_dtcs(const uint8_t *bytes, uint8_t len);

/* Inform the UI that “clear DTC” has succeeded (optional helper)          */
void BLE_notify_clear(void);

typedef struct {                       /* ■ BLE profile bookkeeping                */
    esp_gatts_cb_t      gatts_cb;      /* callback we registered                   */
    esp_gatt_if_t       gatts_if;      /* interface assigned by stack              */
    uint16_t            app_id;        /* 0 … just one profile                     */

    /* Populated later: */
    uint16_t            conn_id;       /* on ESP_GATTS_CONNECT_EVT                 */
    uint16_t            service_hdl;   /* on ESP_GATTS_CREATE_EVT                  */
    esp_gatt_srvc_id_t  service_id;    /* we filled before create_service()        */
} trainer_profile_t;

typedef struct {
    service_request_t svc;
    uint8_t           pid;     /* single PID that this request targets       */
} ble_req_t;

extern trainer_profile_t profile;
extern uint8_t           hdl_rx;
extern bool stream_on_master;
extern QueueHandle_t ble_queue;

#endif
