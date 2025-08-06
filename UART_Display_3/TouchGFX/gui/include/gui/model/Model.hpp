#ifndef MODEL_HPP
#define MODEL_HPP

#include <gui/model/ModelListener.hpp>
#include <vector>
#include "UART_COMMS.hpp"
#include <stdint.h>

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
    void set_isLIVE(bool);
    bool Get_DTCs_Status();
    bool isLIVE_get();
    void Model_Set_Service(uart_comms_t ser, int pid = 0);
    int dtcs_num_get();
    char** dtcs_get();
    void Model_Set_Data(int pid, uint8_t *value = NULL, uint8_t (*mask)[4] = NULL, int num_bytes = 0);

protected:
    ModelListener* modelListener;
    char ** dtcs_list_model;
    int dtcs_num_model = 0;
    bool DTCs_Ready = false;
    bool isLIVE = false;
};

#endif // MODEL_HPP
