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

//codes comes in with every code being 5 digit with only one \0 at the end
void DTCs_GUI_Pass(const char* codes, int num_codes)
{
	char **temp;
	temp = static_cast<char **>(pvPortMalloc(sizeof(char*) * num_codes));
	if (!temp) return;
    if (modelInstance)
    {
    	for (int i = 0; i < num_codes; i ++){
    		temp[i] = static_cast<char *>(pvPortMalloc(6));
    		if (!temp[i]) break;  // handle malloc failure

    		memcpy(temp[i],&codes[i * 5], 5);
    		temp[i][5] = 0;
    	}
    	modelInstance->dtcs_Set(temp, num_codes);  // call into the GUI // @suppress("Invalid arguments")
    	for (int i = 0; i < num_codes; i ++) {
    	    vPortFree(temp[i]);
    	}
    	vPortFree(temp);
    }
}

