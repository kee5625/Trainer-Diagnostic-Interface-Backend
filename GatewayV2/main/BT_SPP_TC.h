#include "esp_spp_api.h"
#include "esp_gap_bt_api.h"

#ifndef BT_SPP
#define BT_SPP

static void spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
static void bt_gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
void bt_spp_setup();

#endif