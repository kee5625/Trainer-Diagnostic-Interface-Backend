/*
 * blinking.h
 *
 *  Created on: Jun 12, 2025
 *      Author: nbatcher
 */

#ifndef INC_BLINKING_H_
#define INC_BLINKING_H_

#include "stm32h7xx_hal.h"
#include "cmsis_os.h"
#include "USART.h"

extern Action_phase_t Action_phase;
extern osSemaphoreId_t  blink_sem;
void blk_toggle_led();


#endif /* INC_BLINKING_H_ */
