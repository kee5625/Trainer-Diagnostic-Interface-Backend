#include <gui/containers/rowItem.hpp>

rowItem::rowItem()
{

}

void rowItem::initialize()
{
    rowItemBase::initialize();
}

void rowItem::setTitle(const char *str){
	Unicode::strncpy(Title_TBBuffer, str, TITLE_TB_SIZE);
	Title_TB.invalidate();
}

void rowItem::setValue(const char *str){
	Unicode::strncpy(Value_TBBuffer, str, VALUE_TB_SIZE);
	Value_TB.invalidate();
}
