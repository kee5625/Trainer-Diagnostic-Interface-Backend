#include <gui/common/FrontendApplication.hpp>
#include <gui/home_screen_screen/Home_ScreenView.hpp>
#include <touchgfx/widgets/AbstractButton.hpp>

Home_ScreenView::Home_ScreenView()
    : wr_TC_Button_Pressed(this, &Home_ScreenView::read_dtcs_button_pressed) // @suppress("Symbol is not resolved")
{}

void Home_ScreenView::setupScreen()
{
    Home_ScreenViewBase::setupScreen();
    Read_TC_Start_button.setAction(wr_TC_Button_Pressed);
    Read_live_data_Start_button.setAction(wr_TC_Button_Pressed);
}

void Home_ScreenView::tearDownScreen()
{
    Home_ScreenViewBase::tearDownScreen();
}

void Home_ScreenView::handleTickEvent(){
}

void Home_ScreenView::read_dtcs_button_pressed(const touchgfx::AbstractButtonContainer& src){ // @suppress("Member declaration not found")
	if (&src == &Read_TC_Start_button)
	{
		//TC_Read_Screen_Switch
		//When Read_TC_Start_button clicked change screen to TC_Screen
		//Go to TC_Screen with screen transition towards East
		application().gotoTC_ScreenScreenWipeTransitionEast();
	}
	if (&src == &Read_live_data_Start_button)
	{
		//RLD_Screen_Switch
		//When Read_live_data_Start_button clicked change screen to Read_Live_Data_Screen
		//Go to Read_Live_Data_Screen with no screen transition
		application().gotoRead_Live_Data_ScreenScreenNoTransition();
	}
}


