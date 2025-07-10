#include <gui/read_live_data_screen_screen/Read_Live_Data_ScreenView.hpp>

Read_Live_Data_ScreenView::Read_Live_Data_ScreenView():
	wr_Button_Press_CB(this, &Read_Live_Data_ScreenView::Button_Press_CB),
	wr_Update_Item_CB(this,  &Read_Live_Data_ScreenView::Update_Item_CB)
{}

void Read_Live_Data_ScreenView::setupScreen()
{
	RLD_Options.setDrawables(RLD_OptionsListItems, wr_Update_Item_CB);
	Read_Live_Data_ScreenViewBase::setupScreen();
	home_button.setAction(wr_Button_Press_CB);
}

void Read_Live_Data_ScreenView::tearDownScreen()
{
    Read_Live_Data_ScreenViewBase::tearDownScreen();
}

void Read_Live_Data_ScreenView::Button_Press_CB(const touchgfx::AbstractButtonContainer& src){
	if (&src == &home_button)
	{
		//home_Button_int
		//When home_button clicked change screen to Home_Screen
		//Go to Home_Screen with screen transition towards East
		application().gotoHome_ScreenScreenWipeTransitionEast();
	}
}


void Read_Live_Data_ScreenView::Update_Item_CB(touchgfx::DrawableListItemsInterface* items, int16_t containerIndex, int16_t itemIndex)
{
	const char *titles[59] = {"1. Number of DTCs", "2. Malfunction Indicator Lamp Status", "3. Fuel System 1 Status", "4. Fuel System 2 Status", "5. Calculated Load Value",
			    "6. Engine Coolant Temperature", "7. Short Term Fuel Trim - Bank 1", "8. Long Term Fuel Trim - Bank 1", "9. Short Term Fuel Trim - Bank 2", "10. Long Term Fuel Trim - Bank 2",
			    "11. Fuel Pressure", "12. Intake Manifold Absolute Pressure", "13. Engine RPM", "14. Vehicle Speed", "15. Timing Advance",
			    "16. Intake Air Temperature", "17. Mass Air Flow Rate", "18. Throttle Position", "19. Oxygen Sensor 1 Voltage", "20. Oxygen Sensor 2 Voltage",
			    "21. Oxygen Sensor 3 Voltage", "22. Oxygen Sensor 4 Voltage", "23. Bank 1 Sensor 1 Fuel Trim", "24. Bank 1 Sensor 2 Fuel Trim",
			    "25. Bank 2 Sensor 1 Fuel Trim", "26. Bank 2 Sensor 2 Fuel Trim", "27. Runtime Since Engine Start", "28. Distance Traveled with MIL On",
			    "29. Fuel Rail Pressure (Relative to Manifold)", "30. Fuel Rail Pressure (Direct)", "31. Commanded EGR", "32. EGR Error",
			    "33. Commanded Evaporative Purge", "34. Fuel Level Input", "35. Number of Warm-ups Since Codes Cleared",
			    "36. Distance Traveled Since Codes Cleared", "37. Barometric Pressure", "38. Catalyst Temperature Bank 1 Sensor 1",
			    "39. Catalyst Temperature Bank 1 Sensor 2", "40. Catalyst Temperature Bank 2 Sensor 1", "41. Catalyst Temperature Bank 2 Sensor 2",
			    "42. Control Module Voltage", "43. Absolute Load Value", "44. Commanded Equivalence Ratio",
			    "45. Relative Throttle Position", "46. Ambient Air Temperature", "47. Absolute Throttle Position B", "48. Absolute Throttle Position C",
			    "49. Accelerator Pedal Position D", "50. Accelerator Pedal Position E", "51. Commanded Throttle Actuator",
			    "52. Time Since Engine Start", "53. Time Since Trouble Codes Cleared", "54. Fuel Type", "55. Ethanol Fuel Percentage",
			    "56. Evap System Vapor Pressure", "57. Engine Oil Temperature", "58. Fuel Injection Timing", "59. Engine Fuel Rate",
	};

    if (items == &RLD_OptionsListItems && itemIndex < 59)
    {
    	RLD_OptionsListItems[containerIndex].setTitle(titles[itemIndex]);
    }
}



