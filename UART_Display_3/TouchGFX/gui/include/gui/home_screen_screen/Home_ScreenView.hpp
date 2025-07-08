#ifndef HOME_SCREENVIEW_HPP
#define HOME_SCREENVIEW_HPP

#include <gui_generated/home_screen_screen/Home_ScreenViewBase.hpp>
#include <gui/home_screen_screen/Home_ScreenPresenter.hpp>
#include <touchgfx/widgets/AbstractButton.hpp>

class Home_ScreenView : public Home_ScreenViewBase
{
public:
    Home_ScreenView();
    virtual ~Home_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void TC_loaded();
    virtual void handleTickEvent() override;

protected:
    touchgfx::Callback<Home_ScreenView, const touchgfx::AbstractButtonContainer&> wr_TC_Button_Pressed;
    void read_tc_button_pressed(const touchgfx::AbstractButtonContainer& src);
    bool error_msg_show = false;
    int tick_count = 150; //equivalent to 150 frames or 5 seconds
};

#endif // HOME_SCREENVIEW_HPP
