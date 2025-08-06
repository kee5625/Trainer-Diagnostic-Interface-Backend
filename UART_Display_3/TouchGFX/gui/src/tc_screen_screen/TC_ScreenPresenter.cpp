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

void TC_ScreenPresenter::set_dtcs(char ** dtcs_model, int dtcs_num_model){
	dtcs_num = dtcs_num_model;
	dtcs_list = dtcs_model;
	view.DTCs_Loaded();
}

void TC_ScreenPresenter::Pres_Set_Service(uart_comms_t ser){
	if (ser == UART_DTCs_Reset_cmd || ser == UART_end_of_cmd){ //here crash before model-> call
	}
	model->Model_Set_Service(ser);
}

void TC_ScreenPresenter::set_ISLIVE(bool islive){
	model->set_isLIVE(islive);
}

void TC_ScreenPresenter::activate()
{
}

void TC_ScreenPresenter::deactivate()
{
}

bool TC_ScreenPresenter::Get_DTCs_Status(){
	return model->Get_DTCs_Status();
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




