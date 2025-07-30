#include <gui/containers/rowItem_Data.hpp>
#include <stdio.h>

rowItem_Data::rowItem_Data()
{

}

void rowItem_Data::initialize()
{
    rowItem_DataBase::initialize();
}

void rowItem_Data::setTitle(const char *str){
	Unicode::strncpy(Title_TBBuffer, str, TITLE_TB_SIZE);
	invalidate();
}

void rowItem_Data::setValue(const char *str){
	Unicode::strncpy(Value_TBBuffer, static_cast<const char *>(str), VALUE_TB_SIZE);
	invalidate();
}

void rowItem_Data::setCount(int count){
	 char temp[5];
	    snprintf(temp, sizeof(temp), "%d:", count);
	    Unicode::strncpy(count_TBBuffer, temp, COUNT_TB_SIZE);
	    invalidate();
}
