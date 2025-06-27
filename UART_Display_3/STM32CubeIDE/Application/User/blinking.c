/*
 * blinking.c
 *
 *  Created on: Jun 12, 2025
 *      Author: nbatcher
 */
#include "blinking.h"
#include "cmsis_os.h"
#include "USART.h"
#include "stm32h7xx_hal.h"
#include "stdbool.h"

osSemaphoreId_t blink_sem;

void blk_toggle_led(){
	blink_sem = osSemaphoreNew(100,0,NULL);
	init_error_check(blink_sem);
	while(1){
		if (osSemaphoreAcquire(blink_sem,osWaitForever) == osOK && !isCleanUp){
			HAL_GPIO_TogglePin(GPIOI,GPIO_PIN_13);
			osDelay(pdMS_TO_TICKS(1000));
			HAL_GPIO_TogglePin(GPIOI,GPIO_PIN_13);
		}else{
			osSemaphoreDelete(blink_sem);
			osThreadExit();
		}
	}

}

