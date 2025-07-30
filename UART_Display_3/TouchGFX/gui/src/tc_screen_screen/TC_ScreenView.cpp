#include <gui/tc_screen_screen/TC_ScreenView.hpp>
#include <touchgfx/widgets/AbstractButton.hpp>
#include <string.h>
#include <cstdlib>
#include <vector>
#include <gui/containers/rowItem_dtcs.hpp>
#include "UART_COMMS.hpp"
extern "C" {
    #include "FreeRTOS.h"
}

int tick_count = 0;


static const DTCEntry dtc_table[] = {
   		{"P0304", "Misfire in cylinder 4"},
   		{"P0200", "Injector Circuit/Open"},
		{"P0480", "Cooling Fan 1 Control Circuit"},
   		{"P0481", "Cooling Fan 2 Control Circuit"},
   		{"P0482", "Cooling Fan 2 Control Circuit"},
		{"P0483", "Cooling Fan Rationality Check"},
		{"P0484", "Cooling Fan Circuit Over-Current"},
		{"P0485", "Cooling Fan Power/Ground Circuit"},
		{"P0495", "Fan Speed Too High"},
		{"P0496", "Fan Speed Too Low"},
		{"P01C1", "Fuel Rail Pressure Sensor Circuit Low (Bank 2)."},
		{"P01C1", "Fuel Rail Pressure Sensor Circuit High (Bank 2)."},
		{"P02C0", "Cylinder 10 Injector Restricted."},
		{"C33C1", "Unknown Chassis code."},
};

TC_ScreenView::TC_ScreenView()
    : myButtonCallback(this, &TC_ScreenView::onMyButtonPressed),
	  wr_Update_Item_CB(this, &TC_ScreenView::Update_Item_CB)
{}


const char *TC_ScreenView::DTC_Desc_Grab(char *DTC){
	for (int i = 0; i < (int)(sizeof(dtc_table) / sizeof(dtc_table[0])); i ++){
		if (strcmp(DTC,dtc_table[i].code) == 0)
			return dtc_table[i].desc;
	}

	switch (DTC[0]){
		case 'P': return "Power train problem. If more detail was expected try pressing the back button then requesting codes again.";
		case 'C': return "Chassis  problem. If more detail was expected try pressing the back button then requesting codes again.";
		case 'B': return "Body problem. If more detail was expected try pressing the back button then requesting codes again.";
		case 'U': return "Network problem. If more detail was expected try pressing the back button then requesting codes again.";
		default : return "Unknown problem. Press back button and then try reading codes again.";
	}
}

void TC_ScreenView::Select_Phase_Set(){

		//show_stored_button
		Stored_dtcs_button.setVisible(true);
		Stored_dtcs_button.invalidate();

		//show_pending_button
		Pending_dtcs_button.setVisible(true);
		Pending_dtcs_button.invalidate();

		//show_perm_button
		Perm_dtcs_button.setVisible(true);
		Perm_dtcs_button.invalidate();

		//hide status_TB
		status_TB.setVisible(false);
		status_TB.invalidate();

		//hide Clear_DTCs_button
		Clear_DTCS_button.setVisible(false);
		Clear_DTCS_button.invalidate();

		//clear DTCS_ list
		DTCS_.setNumberOfItems(0);

		//hide DTCS_
		DTCS_.setVisible(false);
		DTCS_.invalidate();

		screen_Phase = Phase::select;
		Unicode::strncpy(status_TBBuffer, "Loading...", sizeof("Loading..."));
}

void TC_ScreenView::setupScreen()
{
	screen_Phase = Phase::select;
    TC_ScreenViewBase::setupScreen();
    Pending_dtcs_button.setAction(myButtonCallback);
    Stored_dtcs_button.setAction(myButtonCallback);
	Perm_dtcs_button.setAction(myButtonCallback);
	Clear_DTCS_button.setAction(myButtonCallback);
	LD_Button.setAction(myButtonCallback);
	back_button.setAction(myButtonCallback);
	DTCS_.setNumberOfItems(presenter->get_Num_DTCs());
	DTCS_.setDrawables(DTCS_ListItems, wr_Update_Item_CB);

}

void TC_ScreenView::handleTickEvent(){
	//at least 3 sec wait and DTC ready(tick_count reset when loading box = visible)

	if (pendingDTCUpdate && tick_count >= 60 && (screen_Phase == Phase::loading || screen_Phase == Phase::no_dtcs)){
		int dtcs_num = presenter->get_Num_DTCs();
		if (dtcs_num != 0){
			screen_Phase = Phase::display;
			status_TB.setVisible(false);
			status_TB.invalidate();
			pendingDTCUpdate = false;
			DTCS_.setVisible(true);//Show_DTCs_list
			Clear_DTCS_button.setVisible(true);//show clear DTCs option
		}else if (screen_Phase != Phase::no_dtcs){
			screen_Phase = Phase::no_dtcs;
			tick_count = -30;
			Unicode::strncpy(status_TBBuffer, "No DTCs", sizeof("No DTCs."));
		}else{
			status_TB.setVisible(false);
			status_TB.invalidate();
			pendingDTCUpdate = false;
			Select_Phase_Set();
		}
		touchgfx::Application::getInstance()->invalidate();
	}
	tick_count++;
}

void TC_ScreenView::tearDownScreen()
{
	Select_Phase_Set();
    TC_ScreenViewBase::tearDownScreen();
}

void TC_ScreenView::DTCs_Loaded(){
	DTCS_.setNumberOfItems(presenter->get_Num_DTCs());
	pendingDTCUpdate = true;
}

void TC_ScreenView::onMyButtonPressed(const touchgfx::AbstractButtonContainer& source){

	if (&source != &Clear_DTCS_button && &source != &LD_Button && &source != &back_button){
		screen_Phase = Phase::loading;
		//hide_stored_button
		Stored_dtcs_button.setVisible(false);
		Stored_dtcs_button.invalidate();

		//hide_pending_button
		Pending_dtcs_button.setVisible(false);
		Pending_dtcs_button.invalidate();

		//hide_perm_button
		Perm_dtcs_button.setVisible(false);
		Perm_dtcs_button.invalidate();

		status_TB.setVisible(true);
		status_TB.invalidate();
		tick_count = 0; //loading text counter
	}

    if (&source == &Pending_dtcs_button){
    	presenter->Pres_Set_Service(UART_DTCs_REQ_PENDING_cmd);
     } else if (&source == &Stored_dtcs_button){
    	presenter->Pres_Set_Service(UART_DTCs_REQ_STORED_cmd);
    } else if (&source == &Perm_dtcs_button){
    	presenter->Pres_Set_Service(UART_DTCs_REQ_PERM_cmd);
	} else if (&source == &Clear_DTCS_button || &source == &LD_Button){
    	if (presenter->get_Num_DTCs() != 0) Select_Phase_Set();

    	if (&source == &LD_Button) application().gotoRead_Live_Data_ScreenScreenWipeTransitionEast();

    	if (&source == &Clear_DTCS_button) presenter->Pres_Set_Service(UART_DTCs_Reset_cmd);
	}
    if (&source == &back_button){
    	if (screen_Phase == Phase::select){
    		application().gotoHome_ScreenScreenWipeTransitionEast();
    	}else{
    		Select_Phase_Set();
    	}
    }
}

void TC_ScreenView::Update_Item_CB(touchgfx::DrawableListItemsInterface* items, int16_t containerIndex, int16_t itemIndex){
	char * temp = static_cast<char *>(pvPortMalloc(5 + 1)); //DTCS are 5 digits plush \0

	if (items == &DTCS_ListItems)
	{
		strcpy(temp,presenter->GetDtcs(itemIndex));
		DTCS_ListItems[containerIndex].dtcs_set(static_cast<const char *>(temp));
		DTCS_ListItems[containerIndex].dtcs_Desc_set(static_cast<const char *>(DTC_Desc_Grab(temp)));
		DTCS_UpdateItem(DTCS_ListItems[containerIndex], itemIndex);
	}
}
