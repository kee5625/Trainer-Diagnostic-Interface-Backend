#include <gui/home_screen_screen/Home_ScreenView.hpp>
#include <gui/home_screen_screen/Home_ScreenPresenter.hpp>

Home_ScreenPresenter::Home_ScreenPresenter(Home_ScreenView& v)
    : view(v)
{

}

void Home_ScreenPresenter::activate()
{

}

void Home_ScreenPresenter::deactivate()
{

}

bool Home_ScreenPresenter::TC_Ready(){
	return model->TC_Status();
}

void Home_ScreenPresenter::updateTextBox(const char * newText){
	//does nothing no tc textbox in this screen
	//funciton is usless because there is no text box that needs updated on this screen
}
