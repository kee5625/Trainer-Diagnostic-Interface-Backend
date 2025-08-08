#ifndef READ_LIVE_DATA_SCREENPRESENTER_HPP
#define READ_LIVE_DATA_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>
#include <stdint.h>
#include <vector>
#include "PIDs_Library.hpp"

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

    void updateValue(int);
    uint8_t get_PIDCode(int);
    const char * const get_data_title(int);
    const char * const get_data_unit(int index);
    const char *get_Value(int);
    bool get_isLIVE();
    void set_Service(uart_comms_t serv,uint8_t pid = 0);
    void set_Data(int pid = 0, uint8_t *value = NULL, uint8_t (*mask)[4] = NULL, int num_bytes = 0) override;


private:
    PID *pidList;
    int pidListSize = 0;
    int pidDataBytes = 0;
    static const char* Descriptions[num_Descriptions];
    inline int Find_PID_INDEX(uint8_t pid); //same index as the scroll list

    void PID_List_init(uint8_t (*mask)[4]);

    Read_Live_Data_ScreenPresenter();

    Read_Live_Data_ScreenView& view;
};

#endif // READ_LIVE_DATA_SCREENPRESENTER_HPP
