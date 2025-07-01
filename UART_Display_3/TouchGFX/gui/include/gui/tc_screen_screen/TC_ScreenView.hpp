#ifndef TC_SCREENVIEW_HPP
#define TC_SCREENVIEW_HPP

#include <gui_generated/tc_screen_screen/TC_ScreenViewBase.hpp>
#include <gui/tc_screen_screen/TC_ScreenPresenter.hpp>
#include <touchgfx/widgets/AbstractButton.hpp>

class TC_ScreenView : public TC_ScreenViewBase
{
public:
    TC_ScreenView();
    virtual ~TC_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();


    void bufferUpdateTCTextBox(const char* newText);
protected:
    touchgfx::Callback<TC_ScreenView, const touchgfx::AbstractButton&> myButtonCallback;
    void onMyButtonPressed(const touchgfx::AbstractButton& source);  // This is your function
    bool TC_Button_First_Press = true;
};

#endif // TC_SCREENVIEW_HPP
