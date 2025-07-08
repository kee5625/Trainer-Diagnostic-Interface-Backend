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

void Model::TC_Set(const char* str)
{
	TC_Ready = (TC_Ready == true) ? false : true;
	this->data = str;
}

void Model::reset_TC(){
	Logic_Reset_TC();
}

bool Model::TC_Status(){
	return TC_Ready;
}
//*************************************Need to add error message if no trouble code
//*************************************given when read tc button pressed.
//Give function will update tc_textBox with correct trouble code.
void Model::TC_Give(){
	if (modelListener != nullptr && data != nullptr){
		modelListener->updateTextBox(data);
	}
}




