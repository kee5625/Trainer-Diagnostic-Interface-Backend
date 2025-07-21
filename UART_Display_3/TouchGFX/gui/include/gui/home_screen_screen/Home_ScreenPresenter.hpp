#ifndef HOME_SCREENPRESENTER_HPP
#define HOME_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Home_ScreenView;

class Home_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Home_ScreenPresenter(Home_ScreenView& v);

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

    virtual ~Home_ScreenPresenter() {}

private:
    Home_ScreenPresenter();

    Home_ScreenView& view;
};

#endif // HOME_SCREENPRESENTER_HPP
