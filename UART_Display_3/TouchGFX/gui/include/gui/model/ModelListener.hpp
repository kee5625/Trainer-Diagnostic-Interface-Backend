#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP


#include <stdint.h>
#include <cstddef>

class Model;

class ModelListener
{
public:
    ModelListener() : model(0) {}

    virtual ~ModelListener() {}

    virtual void set_dtcs(char ** dtcs_model, int dtcs_num_model) {}

    virtual void set_Data(int pid = 0, uint8_t *value = NULL, uint8_t (*mask)[4] = NULL, int num_bytes = 0) {}

    void bind(Model* m)
    {
        model = m;
    }
protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
