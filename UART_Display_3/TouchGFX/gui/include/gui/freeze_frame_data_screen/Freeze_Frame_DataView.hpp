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
protected:
};

#endif // FREEZE_FRAME_DATAVIEW_HPP
