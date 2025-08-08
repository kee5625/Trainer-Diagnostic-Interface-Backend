#include <gui/common/FrontendApplication.hpp>
#include <new>
#include <gui_generated/common/FrontendApplicationBase.hpp>
#include <gui/common/FrontendHeap.hpp>
#include <touchgfx/transitions/NoTransition.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Texts.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <platform/driver/lcd/LCD16bpp.hpp>
#include <gui/home_screen_screen/Home_ScreenView.hpp>
#include <gui/home_screen_screen/Home_ScreenPresenter.hpp>
#include <gui/tc_screen_screen/TC_ScreenView.hpp>
#include <gui/tc_screen_screen/TC_ScreenPresenter.hpp>
#include <gui/read_live_data_screen_screen/Read_Live_Data_ScreenView.hpp>
#include <gui/read_live_data_screen_screen/Read_Live_Data_ScreenPresenter.hpp>




FrontendApplication::FrontendApplication(Model& m, FrontendHeap& heap)
    : FrontendApplicationBase(m, heap)
{

}

void FrontendApplication::gotoTC_ScreenScreenWipeTransitionEast(){
	// Your custom logic here
	static TC_ScreenView view;
	static TC_ScreenPresenter presenter(view);

	model.bind(&presenter);  // ✅ This is your custom logic

	// Call the base class implementation
	FrontendApplicationBase::gotoTC_ScreenScreenWipeTransitionEastImpl();

}
