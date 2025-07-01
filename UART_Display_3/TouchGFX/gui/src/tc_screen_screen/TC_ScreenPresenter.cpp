#include <gui/tc_screen_screen/TC_ScreenView.hpp>
#include <gui/tc_screen_screen/TC_ScreenPresenter.hpp>

TC_ScreenPresenter::TC_ScreenPresenter(TC_ScreenView& v)
    : view(v)
{

}

void TC_ScreenPresenter::activate()
{
	model->TC_Give();
}

void TC_ScreenPresenter::deactivate()
{

}

void TC_ScreenPresenter::reset_TC(){
	model->reset_TC();
}

void TC_ScreenPresenter::updateTextBox(const char * newText) {
	view.bufferUpdateTCTextBox(newText);
}
