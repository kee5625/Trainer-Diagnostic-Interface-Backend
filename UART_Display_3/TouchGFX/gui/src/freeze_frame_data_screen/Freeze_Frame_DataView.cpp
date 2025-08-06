//#include <gui/freeze_frame_data_screen/Freeze_Frame_DataView.hpp>
//#include <stdio.h>
//
//int freeze_ticks= 0;
//
//
//Freeze_Frame_DataView::Freeze_Frame_DataView():
//		wr_Button_CB(this, &Freeze_Frame_DataView::Button_Press_CB),
//		wr_Update_CB(this,  &Freeze_Frame_DataView::Update_Item_CB)
//{
//
//}
//
//void Freeze_Frame_DataView::setupScreen()
//{
//	Freeze_Frame_DataViewBase::setupScreen();
//	Freeze_data.setDrawables(Freeze_dataListItems, wr_Update_CB);
//	home_button.setAction(wr_Button_CB);
//	Freeze_data.setVisible(false);
//}
//
//void Freeze_Frame_DataView::tearDownScreen()
//{
//	Freeze_Frame_DataViewBase::tearDownScreen();
//}
//
//void Freeze_Frame_DataView::handleTickEvent(){
//	if (PendingListUpdate == true){
//		PendingListUpdate = false;
//		Freeze_data.setVisible(true);
//		touchgfx::Application::getInstance()->invalidate();
//	}
//
//
//	if (Freeze_data.getNumberOfItems() != 0 && freeze_ticks % 60 == 0){
//		//setting number of items to be updated in this update run
//		int num_Items = (Freeze_data.getNumberOfItems() >= 6) ? 6 : Freeze_data.getNumberOfItems();
//
//		for (int i = 0; i < num_Items; i ++){//6 items visible on scroll list at a time
//			presenter->set_Service(UART_DATA_PID, presenter->get_PIDCode(i));
//		}
////		presenter->set_Service(UART_data_PID, presenter->get_PIDCode(1));
//	}
//	freeze_ticks++;
//}
//
////******************************Set/Get functions*************************************************************
//
////deals with updating number of items and individual values
//void Freeze_Frame_DataView::set_List_Num_Items(int num_items){
//	if (num_items >= 0){
//		Freeze_data.setNumberOfItems(num_items);
//		Freeze_data.invalidate();
//		PendingListUpdate = true;
//	}
//}
//
//
//
//void Freeze_Frame_DataView::Update_List(int index, int pid){
//	Freeze_dataListItems[index].setValue(presenter->get_Value(pid));
//	PendingListUpdate = true;
//}
//
////******************************CallBacks*************************************************************
//
//void Freeze_Frame_DataView::Button_Press_CB(const touchgfx::AbstractButtonContainer& src){
//	if (&src == &home_button)
//	{
//		presenter->set_Service(UART_DATA_PID, 0x20);
//		application().gotoHome_ScreenScreenWipeTransitionEast();
//	}
//}
//
//
//void Freeze_Frame_DataView::Update_Item_CB(touchgfx::DrawableListItemsInterface* items, int16_t containerIndex, int16_t itemIndex)
//{
//    if (items == &Freeze_dataListItems && itemIndex < Freeze_data.getNumberOfItems())
//    {
//    	Freeze_dataListItems[containerIndex].setCount(itemIndex + 1);
//    	Freeze_dataListItems[containerIndex].setTitle(presenter->get_data_title(itemIndex));
//    }
//}
