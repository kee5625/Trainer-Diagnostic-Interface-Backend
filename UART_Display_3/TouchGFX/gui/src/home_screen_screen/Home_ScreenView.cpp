#include <gui/common/FrontendApplication.hpp>
#include <gui/home_screen_screen/Home_ScreenView.hpp>
#include <touchgfx/widgets/AbstractButton.hpp>

Home_ScreenView::Home_ScreenView()
    : wr_TC_Button_Pressed(this, &Home_ScreenView::read_tc_button_pressed)
{}

void Home_ScreenView::setupScreen()
{
    Home_ScreenViewBase::setupScreen();
    qrCode1.setVisible(true);
    Read_TC_Start_button.setAction(wr_TC_Button_Pressed);
}

void Home_ScreenView::tearDownScreen()
{
    Home_ScreenViewBase::tearDownScreen();
}

void Home_ScreenView::handleTickEvent(){
	if (tick_count > 0 && error_msg_show){
		tick_count --;
		if (tick_count == 0){
			TC_not_loaded.setVisible(false);
			TC_not_loaded.invalidate();
			tick_count = 150;
		}
	}
}

void Home_ScreenView::read_tc_button_pressed(const touchgfx::AbstractButtonContainer& src){
	if (&src == &Read_TC_Start_button){
		if (presenter->TC_Ready()){
			error_msg_show = false;
			tick_count = 150;
			application().gotoTC_ScreenScreenWipeTransitionEast();
		}else{
			TC_not_loaded.setVisible(true);
			TC_not_loaded.invalidate();
			error_msg_show = true;
		}
	}
}


