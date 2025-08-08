#ifndef TC_SCREENPRESENTER_HPP
#define TC_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>
#include <vector>
#include "UART_COMMS.hpp"


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
    void set_dtcs(char ** dtcs_model, int dtcs_num_model) override;
    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~TC_ScreenPresenter() {}

    bool Get_DTCs_Status();
    void Pres_Set_Service(uart_comms_t ser);
    void set_ISLIVE(bool islive);
    const char * GetDtcs(int increment);
    int get_Num_DTCs();

private:
    char** dtcs_list;
    int dtcs_num = 0;

    TC_ScreenPresenter();
    TC_ScreenView& view;
};

#endif // TC_SCREENPRESENTER_HPP
