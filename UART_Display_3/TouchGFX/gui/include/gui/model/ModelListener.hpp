#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP



class Model;

class ModelListener
{
public:
    ModelListener() : model(0) {}
    
    virtual ~ModelListener() {}

    virtual void updateTextBox(const char* newText) {};

    void bind(Model* m)
    {
        model = m;
    }
protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
