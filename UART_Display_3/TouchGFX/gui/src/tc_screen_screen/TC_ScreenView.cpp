#include <gui/tc_screen_screen/TC_ScreenView.hpp>
#include <touchgfx/widgets/AbstractButton.hpp>

TC_ScreenView::TC_ScreenView()
    : myButtonCallback(this, &TC_ScreenView::onMyButtonPressed)
{}


void TC_ScreenView::setupScreen()
{
    TC_ScreenViewBase::setupScreen();
    TC_Button.setAction(myButtonCallback);
    Erase_TC_Button.setAction(myButtonCallback);

}

void TC_ScreenView::tearDownScreen()
{
    TC_ScreenViewBase::tearDownScreen();
}

void TC_ScreenView::bufferUpdateTCTextBox(const char* newText)
{
	//Unicode::snprintf(TC_TextBoxBuffer, TC_TEXTBOX_SIZE, "%s",newText);
	Unicode::strncpy(TC_TextBoxBuffer, newText, TC_TEXTBOX_SIZE);
    TC_TextBox.invalidate();
}


void TC_ScreenView::onMyButtonPressed(const touchgfx::AbstractButton& source)
{
    if (&source == &TC_Button)
    {
    	if (TC_Button_First_Press){
			//Show_TC
			//When TC_Button clicked show TC_TextBox
			//Show TC_TextBox
			TC_TextBox.setVisible(true);
			TC_TextBox.invalidate();

			//Show_Erase_TC_Button
			//When Show_TC completed show Erase_TC_Button
			//Show Erase_TC_Button
			Erase_TC_Button.setVisible(true);
			Erase_TC_Button.invalidate();
			TC_Button_First_Press = false;
    	}else{
    		TC_TextBox.invalidate();
    	}
    }
    if (&source == &Erase_TC_Button){
		//Erase_Change_Screen
		//When Erase_TC_Button clicked change screen to Home_Screen
		//Go to Home_Screen with screen transition towards East
    	application().gotoHome_ScreenScreenWipeTransitionEast();
    	presenter->reset_TC();
     }
}
