#ifndef TC_SCREENVIEW_HPP
#define TC_SCREENVIEW_HPP

#include <gui_generated/tc_screen_screen/TC_ScreenViewBase.hpp>
#include <gui/tc_screen_screen/TC_ScreenPresenter.hpp>
#include <touchgfx/widgets/AbstractButton.hpp>

typedef enum {
	select_Phase		= 0,
	loading_Phase		= 1,
	dispaly_Phase		= 2,
}phase_t;

class TC_ScreenView : public TC_ScreenViewBase
{
public:

    TC_ScreenView();
    virtual ~TC_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    void handleTickEvent();
    void DTCs_Loaded();
protected:
    bool pendingDTCUpdate = false;
    touchgfx::Callback<TC_ScreenView, const touchgfx::AbstractButtonContainer&> myButtonCallback;
    void onMyButtonPressed(const touchgfx::AbstractButtonContainer& src);
    touchgfx::Callback<TC_ScreenView, touchgfx::DrawableListItemsInterface*, int16_t, int16_t> wr_Update_Item_CB;
    void Update_Item_CB(touchgfx::DrawableListItemsInterface* items, int16_t containerIndex, int16_t itemIndex);

private:
    phase_t screen_Phase;
    void Select_Phase_Set();

};

#endif // TC_SCREENVIEW_HPP
