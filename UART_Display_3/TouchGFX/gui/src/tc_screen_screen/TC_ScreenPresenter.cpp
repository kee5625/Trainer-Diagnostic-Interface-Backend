#include <gui/tc_screen_screen/TC_ScreenView.hpp>
#include <gui/tc_screen_screen/TC_ScreenPresenter.hpp>
#include <vector>
#include <touchgfx/Unicode.hpp>
#include <freertos.h>
#include <string.h>
#include "UART_COMMS.hpp"

TC_ScreenPresenter::TC_ScreenPresenter(TC_ScreenView& v)
    : view(v)
{

}

void TC_ScreenPresenter::set_dtcs(char ** dtcs_model, int dtcs_num_model){ // @suppress("Member declaration not found")
	dtcs_num = dtcs_num_model;
	dtcs_list = static_cast<char**>(pvPortMalloc(sizeof(char*) * dtcs_num));
	if (!dtcs_list) return;

	char** source = dtcs_model;

	for (int i = 0; i < dtcs_num; i++) {
	    dtcs_list[i] = static_cast<char*>(pvPortMalloc(6));
	    if (!source[i] || !dtcs_list[i]) continue;
	    memcpy(dtcs_list[i], source[i], 6);  // deep copy string data // @suppress("Invalid arguments")
	    volatile int thumb = 0;
	    while(thumb > 0 ){
	    	thumb --;
	    }

	    if (!dtcs_list[i]) {
			for (int j = 0; j < i; j++) {
				vPortFree(dtcs_list[j]); // @suppress("Invalid arguments")
			}
			vPortFree(dtcs_list); // @suppress("Invalid arguments")
			dtcs_list = nullptr;
			dtcs_num = 0;
			return;
		}
	}
	view.DTCs_Loaded();
}

void TC_ScreenPresenter::Pres_Set_Service(uart_comms_t ser){ // @suppress("Member declaration not found")
	if (ser == UART_DTCs_Reset_cmd){
		for (int i = 0; i < dtcs_num; i ++){
			vPortFree(dtcs_list[i]); // @suppress("Invalid arguments")
		}
		vPortFree(dtcs_list); // @suppress("Invalid arguments")
		dtcs_num = 0;
	}
	model->Model_Set_Service(ser); // @suppress("Method cannot be resolved")
}

void TC_ScreenPresenter::activate()
{
}

void TC_ScreenPresenter::deactivate()
{
}

bool TC_ScreenPresenter::Get_DTCs_Status(){ // @suppress("Member declaration not found")
	return model->Get_DTCs_Status(); // @suppress("Method cannot be resolved")
}

int TC_ScreenPresenter::get_Num_DTCs(){
	return dtcs_num;
}

const char *TC_ScreenPresenter::GetDtcs(int increment){
	if (increment > dtcs_num){
		return dtcs_list[0];
	}else if (increment == 0){
		return dtcs_list[0];
	}
	return dtcs_list[increment];
}


