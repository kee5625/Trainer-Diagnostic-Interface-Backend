#ifndef FREEZE_FRAME_DATAVIEW_HPP
#define FREEZE_FRAME_DATAVIEW_HPP

#include <gui_generated/freeze_frame_data_screen/Freeze_Frame_DataViewBase.hpp>
#include <gui/freeze_frame_data_screen/Freeze_Frame_DataPresenter.hpp>

class Freeze_Frame_DataView : public Freeze_Frame_DataViewBase
{
public:
	Freeze_Frame_DataView();
    virtual ~Freeze_Frame_DataView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    void handleTickEvent();

    void set_List_Num_Items(int num_items = -1);
    void Update_List(int index, int pid);
protected:
    touchgfx::Callback<Freeze_Frame_DataView, const touchgfx::AbstractButtonContainer&> wr_Button_Press_CB;
    void Button_Press_CB(const touchgfx::AbstractButtonContainer& src);
    touchgfx::Callback<Freeze_Frame_DataView, touchgfx::DrawableListItemsInterface*, int16_t, int16_t> wr_Update_Item_CB;
    void Update_Item_CB(touchgfx::DrawableListItemsInterface* items, int16_t containerIndex, int16_t itemIndex);

private:
    bool PendingListUpdate = false;
    int update_items[8] = {0,1,2,3,4,5,6,7}; //warning: doesn't use size of array
};

#endif // FREEZE_FRAME_DATAVIEW_HPP
