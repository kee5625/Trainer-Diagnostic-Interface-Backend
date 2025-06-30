#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "../../STM32CubeIDE/Application/User/TouchGFX/App/TC_Bridge.hpp"

Model::Model() : modelListener(0)
{
	setModelInstance(this);
}

void Model::tick()
{

}

void Model::TC_Get(const char* data)
{
    modelListener->updateTextBox(data);
}
