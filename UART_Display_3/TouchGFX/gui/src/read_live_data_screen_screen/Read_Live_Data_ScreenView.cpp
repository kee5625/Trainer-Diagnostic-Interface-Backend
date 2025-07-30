#include <gui/read_live_data_screen_screen/Read_Live_Data_ScreenView.hpp>
#include <stdio.h>

//******************************Helper functions*************************************************************
//none for now
int ticks = 0;

Read_Live_Data_ScreenView::Read_Live_Data_ScreenView():
	wr_Button_Press_CB(this, &Read_Live_Data_ScreenView::Button_Press_CB),
	wr_Update_Item_CB(this,  &Read_Live_Data_ScreenView::Update_Item_CB)
{}

void Read_Live_Data_ScreenView::setupScreen()
{
	Read_Live_Data_ScreenViewBase::setupScreen();
	data.setDrawables(dataListItems, wr_Update_Item_CB);
	home_button.setAction(wr_Button_Press_CB);
	presenter->set_Service(UART_PIDS);
	data.setVisible(false);
}

void Read_Live_Data_ScreenView::tearDownScreen()
{
    Read_Live_Data_ScreenViewBase::tearDownScreen();
}

void Read_Live_Data_ScreenView::handleTickEvent(){
	if (PendingListUpdate == true){
		PendingListUpdate = false;
		data.setVisible(true);
		touchgfx::Application::getInstance()->invalidate();
	}


	if (data.getNumberOfItems() != 0 && ticks % 60 == 0){
		//setting number of items to be updated in this update run
		int num_Items = (data.getNumberOfItems() >= 6) ? 6 : data.getNumberOfItems();

		for (int i = 0; i < num_Items; i ++){//6 items visible on scroll list at a time
			presenter->set_Service(UART_DATA_PID, presenter->get_PIDCode(i));
		}
//		presenter->set_Service(UART_DATA_PID, presenter->get_PIDCode(1));
	}
	ticks ++;
}

//******************************Set/Get functions*************************************************************

//deals with updating number of items and individual values
void Read_Live_Data_ScreenView::set_List_Num_Items(int num_items){
	if (num_items >= 0){
		data.setNumberOfItems(num_items);
		data.invalidate();
		PendingListUpdate = true;
	}
}



void Read_Live_Data_ScreenView::Update_List(int index, int pid){
	dataListItems[index].setValue(presenter->get_Value(pid));
	PendingListUpdate = true;
}

//******************************CallBacks*************************************************************

void Read_Live_Data_ScreenView::Button_Press_CB(const touchgfx::AbstractButtonContainer& src){
	if (&src == &home_button)
	{
		presenter->set_Service(UART_DATA_PID, 0x20);
		application().gotoHome_ScreenScreenWipeTransitionEast();
	}
}


void Read_Live_Data_ScreenView::Update_Item_CB(touchgfx::DrawableListItemsInterface* items, int16_t containerIndex, int16_t itemIndex)
{
    if (items == &dataListItems && itemIndex < data.getNumberOfItems())
    {
    	dataListItems[containerIndex].setCount(itemIndex + 1);
    	dataListItems[containerIndex].setTitle(presenter->get_data_title(itemIndex));
    }
}



