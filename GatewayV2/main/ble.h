#ifndef TRAINER_BLE_H
#define TRAINER_BLE_H

#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"

/* Handles exported from ble.c */
extern uint8_t hdl_rx;
extern struct trainer_profile_t profile;   // forward-declared below

/* Same definition used in ble.c so the compiler knows its size */
typedef struct trainer_profile_t {
    esp_gatts_cb_t  gatts_cb;
    uint16_t        gatts_if;
    uint16_t        app_id;
    uint16_t        conn_id;
    uint16_t        service_hdl;
    esp_gatt_srvc_id_t service_id;
} trainer_profile_t;

#endif
