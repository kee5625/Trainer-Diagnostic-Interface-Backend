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

	data.setDrawables(dataListItems, wr_Update_Item_CB);
	home_button.setAction(wr_Button_Press_CB);
	data.setVisible(false);
	Read_Live_Data_ScreenViewBase::setupScreen();
}

void Read_Live_Data_ScreenView::tearDownScreen()
{
    Read_Live_Data_ScreenViewBase::tearDownScreen();
}

void Read_Live_Data_ScreenView::handleTickEvent(){
	if (PendingListUpdate){
		PendingListUpdate = false;
		data.setVisible(true);
		touchgfx::Application::getInstance()->invalidate();
	}


	//updating values
	if ((data.getNumberOfItems() != 0) && (ticks % 60 == 0)){
		for (int i = 0; i < NUM_VISIBLE_CONTAINERS ; i ++){
			presenter->set_Service(UART_DATA_PID, presenter->get_PIDCode(containers[i]));
		}

	}
	ticks ++;
}

//*****************************Helper functions**************************************************

int Read_Live_Data_ScreenView::find_container(int index){

	for (int i = 0; i < NUM_VISIBLE_CONTAINERS; i ++){

		if (index == containers[i]) return i;

	}

	return -1;
}

//******************************Set/Get functions*************************************************************

//deals with updating number of items and individual values
void Read_Live_Data_ScreenView::set_List_Num_Items(int num_items){
	if (num_items >= 0){

		data.setNumberOfItems(num_items);

		if (num_items == 0){

			Failed_loading_tb.setVisible(true);
			Failed_loading_tb.invalidate();

		}

		data.invalidate();
		PendingListUpdate = true;
	}
}

void Read_Live_Data_ScreenView::Update_List(int index, int pid){

	int tempContainer = find_container(index);

	if (tempContainer == -1)	return; //not found in containers

	dataListItems[tempContainer].setValue(presenter->get_Value(index));
	PendingListUpdate = true;
}


//******************************CallBacks*************************************************************

void Read_Live_Data_ScreenView::Button_Press_CB(const touchgfx::AbstractButtonContainer& src){
	if (&src == &home_button)
	{
		presenter->set_Service(UART_DATA_PID, 0x20); //0x20 = exit condition for live data
		application().gotoHome_ScreenScreenWipeTransitionEast();
	}
}


void Read_Live_Data_ScreenView::Update_Item_CB(touchgfx::DrawableListItemsInterface* items, int16_t containerIndex, int16_t itemIndex)
{
	if (containerIndex < NUM_VISIBLE_CONTAINERS){

		containers[containerIndex] = itemIndex;

	}

    if (items == &dataListItems)
    {
    	dataListItems[containerIndex].setCount(itemIndex + 1);
    	dataListItems[containerIndex].setTitle(presenter->get_data_title(itemIndex));
    	dataListItems[containerIndex].setValue(presenter->get_Value(itemIndex));
    	dataListItems[containerIndex].setUnit(presenter->get_data_unit(itemIndex));
    	dataListItems[containerIndex].invalidate();
    	PendingListUpdate = true;
    }
}



