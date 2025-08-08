/*
 * TC_Bridge.cpp
 *
 *  Created on: Jun 27, 2025
 *      Author: nbatcher
 */

// uart_bridge.cpp
#include "TC_Bridge.hpp"
#include <gui/model/Model.hpp>
#include <vector>
#include <stdlib.h>
#include <cstring>
extern "C" {
    #include "FreeRTOS.h"
}

static Model* modelInstance = nullptr;

void setModelInstance(Model* model)
{
    modelInstance = model;
}


//change so that passes pointer and this is done in presenter of dtc screen
//codes comes in with every code being 5 digit with only one \0 at the end
void DTCs_GUI_Pass(char** codes, int num_codes)
{
	modelInstance->dtcs_Set(codes, num_codes);
}



void GUI_Set_PIDs(int pid, uint8_t *value, uint8_t (*mask)[4], int num_bytes){
	modelInstance->Model_Set_Data(pid, value, mask,num_bytes);
}
