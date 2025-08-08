#include <gui/read_live_data_screen_screen/Read_Live_Data_ScreenView.hpp>
#include <gui/read_live_data_screen_screen/Read_Live_Data_ScreenPresenter.hpp>
#include <freertos.h>
#include <string.h>
#include <vector>
#include <stdio.h>
#include "PIDs_Library.hpp"





//******************************************************helper functions************************
static inline int Num_Supported_PIDs(uint8_t (*mask)[4]){
	int count = 0;
	for (int i = 0; i < 7; i ++){
		for(int k = 0; k < 4; k ++){
			for (int j = 0; j < 8; j++){
				count += (mask[i][k] >> j) & 0x01;
			}
		}
	}
	return count;
}

inline int Read_Live_Data_ScreenPresenter::Find_PID_INDEX(uint8_t pid){
	int index = 0;
	for (; index < pidListSize; index++){
		if (pid == pidList[index].pidCode) return index;
	}
	return -1;
}

Read_Live_Data_ScreenPresenter::Read_Live_Data_ScreenPresenter(Read_Live_Data_ScreenView& v)
    : view(v)
{

}

void Read_Live_Data_ScreenPresenter::activate()
{
	if(get_isLIVE()){
		set_Service(UART_PIDS_LIVE);
	}else{
		set_Service(UART_PIDS_FREEZE);
	}
}

void Read_Live_Data_ScreenPresenter::deactivate()
{
	model->set_isLIVE(false);
}

//******************************************************set/get functions************************
const char * const Read_Live_Data_ScreenPresenter::get_data_title(int index){
	return pidList[index].description;
}

const char * const Read_Live_Data_ScreenPresenter::get_data_unit(int index){
	return pidList[index].unit;
}

uint8_t Read_Live_Data_ScreenPresenter::get_PIDCode(int index){
	return pidList[index].pidCode;
}

const char *Read_Live_Data_ScreenPresenter::get_Value(int index){
	return static_cast<const char *>(pidList[index].value);

}

bool Read_Live_Data_ScreenPresenter::get_isLIVE(){
	return model->isLIVE_get();
}

//sets either mask or value when called.
void Read_Live_Data_ScreenPresenter::set_Data(int pid, uint8_t *value, uint8_t (*mask)[4], int num_bytes){
	if (mask){
		PID_List_init(mask);
	}
	if (value){
		int index = Find_PID_INDEX(pid);

		if (index == -1) return; //PID not found in list

		if (pidList[index].value != NULL){
			vPortFree(pidList[index].value); //clear old value
			pidList[index].value = NULL;
		}

		char *decoded_Val = PIDInfoTable[pidList[index].pidCode].decode(value);
		if (decoded_Val == NULL) return;

		pidList[index].value = decoded_Val;			//get new value
		view.Update_List(index , pid); 	//needs PID pasted
	}

}

void Read_Live_Data_ScreenPresenter::set_Service(uart_comms_t serv, uint8_t pid){
	model->Model_Set_Service(serv, pid);
}

//sets descriptions based on bit-mask
void Read_Live_Data_ScreenPresenter::PID_List_init(uint8_t (*mask)[4]){
	int bitIndex = 0;
	int index = 0;
	int num_PIDs = 0;

	//set bits in bitmask = num_PIDs
	for (int i = 0; i < 7; i ++){
		for(int k = 0; k < 4; k ++){
			for (int j = 0; j < 8; j++){
				num_PIDs += (mask[i][k] >> j) & 0x01;
			}
		}
	}

	pidListSize = num_PIDs;
	pidList = (PID *)pvPortMalloc(num_PIDs * sizeof(PID));
	if (!pidList) return;//malloc failure

	//********************************************defining
	while(index < num_PIDs){
		while (bitIndex < num_Descriptions) {
			int row = bitIndex / 32;
			int col = (bitIndex % 32) / 8;
			int bit = bitIndex % 8;

			//checking if index is an available PID
			if (((mask[row][col] >> bit) & 0x01) == 1) {

				int decodedIndex = bitIndex + 1 + 1 * (bitIndex / 0x20);
				PID temp = PID{(uint8_t)decodedIndex, PIDInfoTable[decodedIndex].description, PIDInfoTable[decodedIndex].unit, nullptr};
				pidList[index] = temp;
				break;
			}

			bitIndex++;
		}

		index ++;
		bitIndex++; //loop break skips other bitIndex++
	}

	view.set_List_Num_Items(num_PIDs); 						//setting size of scroll list in view
}




