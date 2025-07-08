/*
 * TC_Bridge.cpp
 *
 *  Created on: Jun 27, 2025
 *      Author: nbatcher
 */

// uart_bridge.cpp
#include "TC_Bridge.hpp"
#include <gui/model/Model.hpp>

static Model* modelInstance = nullptr;

void setModelInstance(Model* model)
{
    modelInstance = model;
}

void TC_GUI_Pass(const char* data)
{
    if (modelInstance)
    {
    	modelInstance->TC_Set(data);  // call into the GUI logic
    }
}

void Logic_Reset_TC(){
	UART_REST_TC();
}

