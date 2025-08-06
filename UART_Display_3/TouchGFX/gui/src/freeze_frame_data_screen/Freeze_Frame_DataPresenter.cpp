//#include <gui/freeze_frame_data_screen/Freeze_Frame_DataView.hpp>
//#include <gui/freeze_frame_data_screen/Freeze_Frame_DataPresenter.hpp>
//#include "UART_COMMS.hpp"
//#include <freertos.h>
//#include <string.h>
//#include <vector>
//#include <stdio.h>
//#include "PIDs_Library.hpp"
//
//Freeze_Frame_DataPresenter::Freeze_Frame_DataPresenter(Freeze_Frame_DataView& v)
//    : view(v)
//{
//
//}
//
//void Freeze_Frame_DataPresenter::activate()
//{
//	set_Service(UART_PIDS_FREEZE);
//}
//
//void Freeze_Frame_DataPresenter::deactivate()
//{
//
//}
//
//static inline int Num_Supported_PIDs(uint8_t (*mask)[4]){
//	int count = 0;
//	for (int i = 0; i < 7; i ++){
//		for(int k = 0; k < 4; k ++){
//			for (int j = 0; j < 8; j++){
//				count += (mask[i][k] >> j) & 0x01;
//			}
//		}
//	}
//	return count;
//}
//
//inline int Freeze_Frame_DataPresenter::Find_PID_INDEX(uint8_t pid){
//	int index = 0;
//	for (; index < pidListSize; index++){
//		if (pid == pidList[index].pidCode) break;
//	}
//	if (index >= pidListSize - 1) return -1; //not found
//	return index;
//}
//
//
//
////******************************************************set/get functions************************
//const char * const Freeze_Frame_DataPresenter::get_data_title(int index){
//	return pidList[index].description;
//}
//
//uint8_t Freeze_Frame_DataPresenter::get_PIDCode(int index){
//	return pidList[index].pidCode;
//}
//
//const char *Freeze_Frame_DataPresenter::get_Value(int PID){
//	int index = Find_PID_INDEX(PID);
//	return static_cast<const char *>(pidList[index].value);
//
//}
//
////sets either mask or value when called.
//void Freeze_Frame_DataPresenter::set_Service(uart_comms_t ser, uint8_t pid)
//{
//	model->Model_Set_Service(ser,pid);
//}
//
//void Freeze_Frame_DataPresenter::set_Data(int pid, uint8_t *value, uint8_t (*mask)[4], int num_bytes){
//	if (mask){
//		PID_List_init(mask);
//	}
//	if (value){
//		int index = Find_PID_INDEX(pid);
//		if (pid == -1) return; //PID not found in list
//
//		if (pidList[index].value != NULL){
//			vPortFree(pidList[index].value); //clear old value
//			pidList[index].value = NULL;
//		}
//
//		pidList[index].value = PIDInfoTable[pidList[index].pidCode].decode(value);			//get new value
//		view.Update_List(index , pid); 	//needs PID pasted
//	}
//
//}
//
////sets descriptions based on bit-mask
//void Freeze_Frame_DataPresenter::PID_List_init(uint8_t (*mask)[4]){
//	int bitIndex = 0;
//	int index = 0;
//	int num_PIDs = 0;
//
//	//set bits in bitmask = num_PIDs
//	for (int i = 0; i < 7; i ++){
//		for(int k = 0; k < 4; k ++){
//			for (int j = 0; j < 8; j++){
//				num_PIDs += (mask[i][k] >> j) & 0x01;
//			}
//		}
//	}
//
//	pidListSize = num_PIDs;
//	pidList = (PID *)pvPortMalloc(num_PIDs * sizeof(PID));
//	if (!pidList) return;//malloc failure
//
//	while(index < num_PIDs){
//		while (bitIndex < num_Descriptions) {
//			int row = bitIndex / 32;
//			int col = (bitIndex % 32) / 8;
//			int bit = bitIndex % 8;
//
//			if (((mask[row][col] >> bit) & 0x01) == 1) {
//				PID temp = PID{(uint8_t)bitIndex, PIDInfoTable[bitIndex].description, PIDInfoTable[bitIndex].unit, nullptr};
//				pidList[index] = temp;
//				pidListSize ++;
//				break;
//			}
//			bitIndex++;
//		}
//
//		index ++;
//		bitIndex++; //loop break skips other bitIndex++
//	}
//
//	view.set_List_Num_Items(num_PIDs); 						//setting size of scroll list in view
//}
