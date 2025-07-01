#ifndef TC_SCREENPRESENTER_HPP
#define TC_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class TC_ScreenView;

class TC_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    TC_ScreenPresenter(TC_ScreenView& v);

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

    virtual ~TC_ScreenPresenter() {}

    void reset_TC();
    void updateTextBox(const char * newText) override;
private:
    TC_ScreenPresenter();

    TC_ScreenView& view;
};

#endif // TC_SCREENPRESENTER_HPP
