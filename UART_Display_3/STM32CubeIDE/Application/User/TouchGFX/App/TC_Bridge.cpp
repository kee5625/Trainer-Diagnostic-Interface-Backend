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

void TC_Received(const char* data)
{
    if (modelInstance)
    {
        modelInstance->TC_Get(data);  // call into the GUI logic
    }
}



