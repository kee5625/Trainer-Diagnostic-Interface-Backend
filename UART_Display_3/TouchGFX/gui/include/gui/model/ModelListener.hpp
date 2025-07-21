#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <vector>


class Model;

class ModelListener
{
public:
    ModelListener() : model(0) {}

    virtual ~ModelListener() {}

    virtual void set_dtcs(char ** dtcs_model, int dtcs_num_model) {}

    void bind(Model* m)
    {
        model = m;
    }
protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
