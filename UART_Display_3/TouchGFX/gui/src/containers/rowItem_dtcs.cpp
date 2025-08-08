#include <gui/containers/rowItem_dtcs.hpp>
#include <vector>
#include <touchgfx/Unicode.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Color.hpp>



rowItem_dtcs::rowItem_dtcs()
	: rowItem_dtcsBase()
{
}

void rowItem_dtcs::initialize()
{
    rowItem_dtcsBase::initialize();
}

void rowItem_dtcs::dtcs_Desc_set(const char *str){
	Unicode::strncpy(dtcs_Desc_TBBuffer, str, DTCS_DESC_TB_SIZE);
	dtcs_Desc_TB.setWideTextAction(WIDE_TEXT_WORDWRAP);
	dtcs_Desc_TB.invalidate();
}

void rowItem_dtcs::dtcs_set(const char *str){
	if (str == NULL)
		return;
	Unicode::strncpy(dtcs_TBBuffer, str, DTCS_TB_SIZE);
	dtcs_TB.invalidate();
}
