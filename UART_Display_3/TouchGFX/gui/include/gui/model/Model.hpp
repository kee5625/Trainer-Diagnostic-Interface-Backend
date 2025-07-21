#ifndef MODEL_HPP
#define MODEL_HPP

#include <gui/model/ModelListener.hpp>
#include <vector>
#include "UART_COMMS.hpp"

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();

    void dtcs_Set(char** str, int dtcs_size);
    bool Get_DTCs_Status();
    void Model_Set_Service(uart_comms_t ser);
    int dtcs_num_get();
    char** dtcs_get();

protected:
    ModelListener* modelListener;
    char ** dtcs_list_model;
    int dtcs_num_model = 0;
    bool DTCs_Ready = false;
};

#endif // MODEL_HPP
