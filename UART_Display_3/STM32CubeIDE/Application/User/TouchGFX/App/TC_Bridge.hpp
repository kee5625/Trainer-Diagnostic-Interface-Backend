/*
 * TC_Bridge.hpp
 *
 *  Created on: Jun 27, 2025
 *      Author: nbatcher
 */

#ifndef APPLICATION_USER_TOUCHGFX_APP_TC_BRIDGE_HPP_
#define APPLICATION_USER_TOUCHGFX_APP_TC_BRIDGE_HPP_



#ifdef __cplusplus
extern "C" {
#endif


void TC_Received(const char* data); // Function to call from C


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <gui/model/Model.hpp>
void setModelInstance(Model* model);
#endif


#endif /* APPLICATION_USER_TOUCHGFX_APP_TC_BRIDGE_HPP_ */
