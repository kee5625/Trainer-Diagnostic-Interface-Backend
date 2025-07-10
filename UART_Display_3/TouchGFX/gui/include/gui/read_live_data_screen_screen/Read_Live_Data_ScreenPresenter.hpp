#ifndef READ_LIVE_DATA_SCREENPRESENTER_HPP
#define READ_LIVE_DATA_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Read_Live_Data_ScreenView;

class Read_Live_Data_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Read_Live_Data_ScreenPresenter(Read_Live_Data_ScreenView& v);

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

    virtual ~Read_Live_Data_ScreenPresenter() {};

//    void init_TB_Titles();

private:
    Read_Live_Data_ScreenPresenter();

    Read_Live_Data_ScreenView& view;
};

#endif // READ_LIVE_DATA_SCREENPRESENTER_HPP
