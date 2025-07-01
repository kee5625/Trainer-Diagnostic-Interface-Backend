#ifndef MODEL_HPP
#define MODEL_HPP

#include <gui/model/ModelListener.hpp>

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();

    void TC_Set(const char* str);
    void TC_Give();
    bool TC_Status();
    void reset_TC();


protected:
    ModelListener* modelListener;
    const char * data;
    bool TC_Ready = false;
};

#endif // MODEL_HPP
