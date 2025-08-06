#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "../../STM32CubeIDE/Application/User/TouchGFX/App/TC_Bridge.hpp"
#include <vector>
#include <string.h>
#include <cstdlib>
#include "UART_COMMS.hpp"
extern "C" {
    #include "FreeRTOS.h"
	#include "cmsis_os.h"
}

Model::Model() : modelListener(0)
{
	setModelInstance(this);
}

void Model::tick()
{

}

//change to have pointers passed to presenter screen
void Model::dtcs_Set(char** str, int dtcs_size)
{
	DTCs_Ready = true;
	modelListener->set_dtcs(str,dtcs_size);
}

void Model::Model_Set_Service(uart_comms_t ser, int pid){
	if (ser == UART_DTCs_Reset_cmd || ser == UART_end_of_cmd) DTCs_Ready = false;
	UART_Set_Service(ser, pid);
}

void Model::Model_Set_Data(int pid, uint8_t *value, uint8_t (*mask)[4],int num_bytes){
	modelListener->set_Data(pid, value, mask);
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




