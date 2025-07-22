#ifndef TC_SCREENVIEW_HPP
#define TC_SCREENVIEW_HPP

#include <gui_generated/tc_screen_screen/TC_ScreenViewBase.hpp>
#include <gui/tc_screen_screen/TC_ScreenPresenter.hpp>
#include <touchgfx/widgets/AbstractButton.hpp>

enum class Phase : uint8_t {
	select				= 0,
	loading				= 1,
	no_dtcs      		= 2,
	display				= 3,

};

typedef struct {
    const char *code;
    const char *desc;
} DTCEntry;

class TC_ScreenView : public TC_ScreenViewBase
{
public:

    TC_ScreenView();
    virtual ~TC_ScreenView() {}
    virtual void setupScreen() override;
    virtual void tearDownScreen() override;
    void handleTickEvent();
    void DTCs_Loaded();
protected:
    bool pendingDTCUpdate = false;
    bool no_DTCs = false;
    touchgfx::Callback<TC_ScreenView, const touchgfx::AbstractButtonContainer&> myButtonCallback;
    void onMyButtonPressed(const touchgfx::AbstractButtonContainer& src);
    touchgfx::Callback<TC_ScreenView, touchgfx::DrawableListItemsInterface*, int16_t, int16_t> wr_Update_Item_CB;
    void Update_Item_CB(touchgfx::DrawableListItemsInterface* items, int16_t containerIndex, int16_t itemIndex);

private:
    Phase screen_Phase;
    void Select_Phase_Set();
    const char *DTC_Desc_Grab(char *DTC);
};

#endif // TC_SCREENVIEW_HPP
