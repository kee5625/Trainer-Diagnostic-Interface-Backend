#include <gui/containers/rowItem_Data.hpp>
#include <stdio.h>
#include <cstring>

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

	if (str[0] == '0') return; //change here because 0 is a valid value, done for consistency during final demo**************************************************************************************************************

	Unicode::strncpy(Value_TBBuffer, static_cast<const char *>(str), VALUE_TB_SIZE);
	invalidate();

}


void rowItem_Data::setUnit(const char *str){

	Unicode::strncpy(UNIT_TBBuffer, static_cast<const char *>(str), UNIT_TB_SIZE);
	invalidate();

}

void rowItem_Data::setCount(int count){

	 char temp[5];
	    snprintf(temp, sizeof(temp), "%d:", count);
	    Unicode::strncpy(count_TBBuffer, temp, COUNT_TB_SIZE);
	    invalidate();

}
