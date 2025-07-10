#ifndef READ_LIVE_DATA_SCREENVIEW_HPP
#define READ_LIVE_DATA_SCREENVIEW_HPP

#include <gui_generated/read_live_data_screen_screen/Read_Live_Data_ScreenViewBase.hpp>
#include <gui/read_live_data_screen_screen/Read_Live_Data_ScreenPresenter.hpp>

class Read_Live_Data_ScreenView : public Read_Live_Data_ScreenViewBase
{
public:
    Read_Live_Data_ScreenView();
    virtual ~Read_Live_Data_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
    touchgfx::Callback<Read_Live_Data_ScreenView, const touchgfx::AbstractButtonContainer&> wr_Button_Press_CB;
    void Button_Press_CB(const touchgfx::AbstractButtonContainer& src);
    touchgfx::Callback<Read_Live_Data_ScreenView, touchgfx::DrawableListItemsInterface*, int16_t, int16_t> wr_Update_Item_CB;
    void Update_Item_CB(touchgfx::DrawableListItemsInterface* items, int16_t containerIndex, int16_t itemIndex);
};

#endif // READ_LIVE_DATA_SCREENVIEW_HPP
