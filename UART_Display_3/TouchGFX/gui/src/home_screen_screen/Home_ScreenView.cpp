#include <gui/common/FrontendApplication.hpp>
#include <gui/home_screen_screen/Home_ScreenView.hpp>
#include <touchgfx/widgets/AbstractButton.hpp>

Home_ScreenView::Home_ScreenView()
    : wr_TC_Button_Pressed(this, &Home_ScreenView::read_dtcs_button_pressed)
{}

void Home_ScreenView::setupScreen()
{
    Home_ScreenViewBase::setupScreen();
    freeze_data_button.setAction(wr_TC_Button_Pressed);
    Read_TC_Start_button.setAction(wr_TC_Button_Pressed);
    Read_live_data_Start_button.setAction(wr_TC_Button_Pressed);
}

void Home_ScreenView::tearDownScreen()
{
    Home_ScreenViewBase::tearDownScreen();
}

void Home_ScreenView::handleTickEvent(){
}

void Home_ScreenView::read_dtcs_button_pressed(const touchgfx::AbstractButtonContainer& src){
	if (&src == &Read_TC_Start_button)
	{

		application().gotoTC_ScreenScreenWipeTransitionEast();
	}else if (&src == &Read_live_data_Start_button)
	{
		presenter->set_isLIVE(true);
		application().gotoRead_Live_Data_ScreenScreenNoTransition();

	}else if (&src == &freeze_data_button){

		application().gotoRead_Live_Data_ScreenScreenNoTransition();
	}
}


