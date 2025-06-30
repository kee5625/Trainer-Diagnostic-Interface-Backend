#include <gui/screen1_screen/Screen1View.hpp>

Screen1View::Screen1View()
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}


void Screen1View::updateTCTextBox(const char* newText)
{
	//Unicode::snprintf(TC_TextBoxBuffer, TC_TEXTBOX_SIZE, "%s",newText);
	Unicode::strncpy(TC_TextBoxBuffer, newText, TC_TEXTBOX_SIZE);
    TC_TextBox.invalidate();
}
