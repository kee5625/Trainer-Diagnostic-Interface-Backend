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

TC_ScreenView::TC_ScreenView()
    : myButtonCallback(this, &TC_ScreenView::onMyButtonPressed),
	  wr_Update_Item_CB(this, &TC_ScreenView::Update_Item_CB)
{}


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

		//hide loading_TB
		loading_TB.setVisible(false);
		loading_TB.invalidate();

		//hide Clear_DTCs_button
		Clear_DTCS_button.setVisible(false);
		Clear_DTCS_button.invalidate();

		//clear DTCS_ list
		DTCS_.setNumberOfItems(0);

		//hide DTCS_
		DTCS_.setVisible(false);
		DTCS_.invalidate();

		screen_Phase = select_Phase;
}

void TC_ScreenView::setupScreen()
{
	screen_Phase = select_Phase;
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

void TC_ScreenView::handleTickEvent(){ // @suppress("Member declaration not found")
	//at least 3 sec wait and DTC ready(tick_count reset when loading box = visible)
	if (pendingDTCUpdate && tick_count >= 90 && screen_Phase == loading_Phase){
		screen_Phase = dispaly_Phase;
		tick_count = 0;
		pendingDTCUpdate = false;

		loading_TB.setVisible(false);
		loading_TB.invalidate();

		DTCS_.setNumberOfItems(presenter->get_Num_DTCs());

		//Show_DTCs_list
		DTCS_.setVisible(true);
		touchgfx::Application::getInstance()->invalidate();

		Clear_DTCS_button.setVisible(true);
		Clear_DTCS_button.invalidate();
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

void TC_ScreenView::onMyButtonPressed(const touchgfx::AbstractButtonContainer& source)
{

	if (&source != &Clear_DTCS_button && &source != &LD_Button && &source != &back_button){
		screen_Phase = loading_Phase;
		//hide_stored_button
		Stored_dtcs_button.setVisible(false);
		Stored_dtcs_button.invalidate();

		//hide_pending_button
		Pending_dtcs_button.setVisible(false);
		Pending_dtcs_button.invalidate();

		//hide_perm_button
		Perm_dtcs_button.setVisible(false);
		Perm_dtcs_button.invalidate();

		loading_TB.setVisible(true);
		loading_TB.invalidate();
		tick_count = 0; //loading text counter

		//making sure buttons can't be pressed until code loaded.
//		clear_DTCS_Button.setAction();
//		back_button.setAction();
//		LD_Button.setAction();
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
    	if (screen_Phase == select_Phase){
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
		DTCS_UpdateItem(DTCS_ListItems[containerIndex], itemIndex);
	}
}
