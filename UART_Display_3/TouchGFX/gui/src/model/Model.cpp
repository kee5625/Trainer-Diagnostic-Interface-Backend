#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "../../STM32CubeIDE/Application/User/TouchGFX/App/TC_Bridge.hpp"
#include <vector>
#include <string.h>
#include <cstdlib>
#include "UART_COMMS.hpp"
extern "C" {
    #include "FreeRTOS.h"
}

Model::Model() : modelListener(0)
{
	setModelInstance(this);
}

void Model::tick()
{

}

void Model::dtcs_Set(char** str, int dtcs_size)
{
	for (int i = 0; i < dtcs_num_model; i ++){
		vPortFree(dtcs_list_model[i]);
	}
	vPortFree(dtcs_list_model);
	dtcs_list_model = static_cast<char **>(pvPortMalloc(sizeof(char*) * dtcs_size));
	if (!dtcs_list_model) return;

	//deep copy, model will hold dtcs for presenter to grab
	for (int j = 0; j < dtcs_size; j ++) {
		dtcs_list_model[j] = static_cast<char *>(pvPortMalloc(6));
		if (!dtcs_list_model[j]) continue;

		if(!str[j]) continue;
		memcpy(dtcs_list_model[j],str[j], 6);
	}
	dtcs_num_model = dtcs_size;
	DTCs_Ready = true;
	modelListener->set_dtcs(dtcs_list_model,dtcs_num_model);
}

void Model::Model_Set_Service(uart_comms_t ser){
	if (ser == UART_DTCs_Reset_cmd) DTCs_Ready = false;
	UART_Set_Service(ser);
}

bool Model::Get_DTCs_Status(){
	return DTCs_Ready;
}

int Model::dtcs_num_get(){
	return dtcs_num_model;
}

char** Model::dtcs_get(){
	return dtcs_list_model;
}




