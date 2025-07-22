#ifndef FREEZE_FRAME_DATAPRESENTER_HPP
#define FREEZE_FRAME_DATAPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Freeze_Frame_DataView;

class Freeze_Frame_DataPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Freeze_Frame_DataPresenter(Freeze_Frame_DataView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~Freeze_Frame_DataPresenter() {}

private:
    Freeze_Frame_DataPresenter();

    Freeze_Frame_DataView& view;
};

#endif // FREEZE_FRAME_DATAPRESENTER_HPP
