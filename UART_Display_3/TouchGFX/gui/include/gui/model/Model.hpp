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

    void TC_Get(const char* data);
protected:
    ModelListener* modelListener;
};

#endif // MODEL_HPP
