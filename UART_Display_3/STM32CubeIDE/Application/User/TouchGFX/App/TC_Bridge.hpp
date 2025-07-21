/*
 * TC_Bridge.hpp
 *
 *  Created on: Jun 27, 2025
 *      Author: nbatcher
 */

#ifndef APPLICATION_USER_TOUCHGFX_APP_TC_BRIDGE_HPP_
#define APPLICATION_USER_TOUCHGFX_APP_TC_BRIDGE_HPP_

#include "UART_COMMS.hpp"

#ifdef __cplusplus
extern "C" {
#endif


void DTCs_GUI_Pass(const char* data, int size); // Function to call from C
void UART_REST_DTCs();
void UART_Set_Service(uart_comms_t ser); //sets service for UART

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <gui/model/Model.hpp>
void setModelInstance(Model* model);
#endif


#endif /* APPLICATION_USER_TOUCHGFX_APP_TC_BRIDGE_HPP_ */
