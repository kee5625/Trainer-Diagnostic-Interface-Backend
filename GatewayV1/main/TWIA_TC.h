
#ifndef TWIA_TC
#define TWIA_TC
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

extern QueueHandle_t req_q;

void can_request_task(void *arg);
void ui_task(void *arg);


#endif