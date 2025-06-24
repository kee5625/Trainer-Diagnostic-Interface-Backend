/* ---------------- header includes ---------------- */
#include "TWIA_TC.h"          /* forward-declarations live here      */
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <string.h>

/* exported from gateway_main.c */
extern QueueHandle_t req_q;

/* ------------------------------------------------------------------ */
/* CAN identifiers and pre-built frames used by the two tasks          */
#define ID_MASTER_PING      0x0A2
#define ID_MASTER_START_CMD 0x0A1
#define ID_MASTER_STOP_CMD  0x0A0
#define ID_SLAVE_PING_RESP  0x0B2
#define ID_SLAVE_DATA       0x0B1

static const char *TAG = "TWAI Master";

static const twai_message_t ping_msg   = { .identifier = ID_MASTER_PING,      .data_length_code = 0 };
static const twai_message_t start_msg  = { .identifier = ID_MASTER_START_CMD, .data_length_code = 0 };
static const twai_message_t stop_msg   = { .identifier = ID_MASTER_STOP_CMD,  .data_length_code = 0 };

/* ============ TASK 1 – handles a UI request ================================= */
void can_request_task(void *arg)          /* <<— NO static so main can link  */
{
    uint8_t dummy;
    twai_message_t rx;

    for (;;) {
        /* 1️⃣ wait until UI pushes anything to the queue */
        xQueueReceive(req_q, &dummy, portMAX_DELAY);

        /* ---------- ping / wait for response ---------- */
        twai_transmit(&ping_msg, portMAX_DELAY);
        do {
            twai_receive(&rx, portMAX_DELAY);
        } while (rx.identifier != ID_SLAVE_PING_RESP);

        /* ---------- start cmd ---------- */
        twai_transmit(&start_msg, portMAX_DELAY);

        /* ---------- receive DTC frame ---------- */
        do {
            twai_receive(&rx, portMAX_DELAY);
        } while (rx.identifier != ID_SLAVE_DATA || rx.data_length_code < 5);

        char dtc[6] = {0};
        memcpy(dtc, rx.data, 5);
        ESP_LOGI(TAG, "DTC received = %s", dtc);

        /* ---------- stop cmd ---------- */
        twai_transmit(&stop_msg, portMAX_DELAY);
    }
}

/* ============ TASK 2 – demo “UI” (simulated button) ========================= */
void ui_task(void *arg)                   /* also non-static                    */
{
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(7000));  /* every 7 s                           */
        uint8_t cmd = 1;
        xQueueSend(req_q, &cmd, portMAX_DELAY);
    }
}