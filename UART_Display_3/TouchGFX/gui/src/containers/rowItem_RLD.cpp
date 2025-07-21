#include <gui/containers/rowItem_RLD.hpp>

rowItem_RLD::rowItem_RLD()
{

}

void rowItem_RLD::initialize()
{
    rowItem_RLDBase::initialize();
}


void rowItem_RLD::setTitle(const char *str){
	Unicode::strncpy(Title_TBBuffer, str, TITLE_TB_SIZE);
	Title_TB.invalidate();
}

void rowItem_RLD::setValue(const char *str){
	Unicode::strncpy(Value_TBBuffer, str, VALUE_TB_SIZE);
	Value_TB.invalidate();
}
