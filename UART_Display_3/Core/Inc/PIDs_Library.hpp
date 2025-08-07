/*
 * PIDs_Desc.hpp
 *
 *  Created on: Jul 24, 2025
 *      Author: nbatcher
 */

#ifndef APPLICATION_USER_GUI_PIDS_DESC_HPP_
#define APPLICATION_USER_GUI_PIDS_DESC_HPP_

#define  num_PIDS          174

#ifdef __cplusplus

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
}




#define num_Descriptions          224 	//number of descriptions hard coded/loaded on device
#define LAST_PID				  0xC8
#define FIRST_PID				  0



/**
 * Object used to hold all info on PID. Used by GUI (presenter/view files)
 */
struct PID { //PID = Parameter Identifier: object used in presenter for GUI screens
    uint8_t pidCode = 0;
    const char* description = nullptr;
    const char* unit = nullptr; //empty if string displayed
    char* value = nullptr;

    PID(uint8_t code, const char* desc, const char* u, char* val)
            : pidCode(code), description(desc), unit(u), value(val) {}
};

typedef char* (*DecoderFunc)(const uint8_t* value);




/**
 * Used by PIDInfoTable to hold all necessary decoding values for PIDs.
 */
struct PIDDecoder {
    const char* unit;
    const char *description;
    DecoderFunc decode;
};



/**
 * PID value byte decoding functions
 */

inline char* bit_Mask_Request(const uint8_t* value){
	float newVal = -9999;

	char* buf = (char*)pvPortMalloc(16);
	if (!buf) return nullptr;

	int intPart = (int)newVal;
	int fracPart = (int)((newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);
	return buf;
}

inline char* bit_Map_Request(const uint8_t* value){
	float newVal = -8888;

	char* buf = (char*)pvPortMalloc(16);
	if (!buf) return nullptr;

	int intPart = (int)newVal;
	int fracPart = (int)((newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);

	return buf;
}

inline char* decode_EngineTemp(const uint8_t* value) {
	float newVal = value[0] - 40;
	char* buf = (char*)pvPortMalloc(16);
	if (!buf) return nullptr;
	int intPart = (int)newVal;
	int fracPart = (int)((newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);
    return buf;
}

inline char* decode_FuelPressure(const uint8_t* value){
	float newVal = value[0] * 3;
	char* buf = (char*)pvPortMalloc(16);
	if (!buf) return nullptr;
	int intPart = (int)newVal;
	int fracPart = (int)((newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);
	return  buf;
}

inline char* decode_ManifoldABSPressure(const uint8_t* value){
	float newVal = value[0];
	char* buf = (char*)pvPortMalloc(16);
	if (!buf) return nullptr;
	int intPart = (int)newVal;
	int fracPart = (int)((newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);
	return buf;
}


inline char* decode_EngineSpeed(const uint8_t* value){
	float newVal = (value[0] * 256 + value[1]) / 4;
	char* buf = (char*)pvPortMalloc(16);
	if (!buf) return nullptr;
	int intPart = (int)newVal;
	int fracPart = (int)((newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);
	return buf;
}


inline char* decode_VechicleSpeed(const uint8_t* value){
	float newVal = value[0];
	char* buf = (char*)pvPortMalloc(16);
	if (!buf) return nullptr;
	int intPart = (int)newVal;
	int fracPart = (int)((newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);
	return buf;
}


inline char* decode_TimeingAdvance(const uint8_t* value){
	float newVal = value[0] / 2 - 64;
	char* buf = (char*)pvPortMalloc(16);
	if (!buf) return nullptr;
	int intPart = (int)newVal;
	int fracPart = (int)((newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);
	return buf;
}


inline char* decode_intakeAirTemp(const uint8_t* value){
	float newVal = value[0] - 40;
	char* buf = (char*)pvPortMalloc(16);
	if (!buf) return nullptr;
	int intPart = (int)newVal;
	int fracPart = (int)((newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);
	return buf;
}

inline char* decode_MAFFlowRate(const uint8_t* value){ //MAF = mass air flow sensor
	float newVal = (256.0f * value[0] + value[1]) / 100.0f;
	char* buf = (char*)pvPortMalloc(16);
	if (!buf) return nullptr;
	int intPart = (int)newVal;
	int fracPart = (int)((newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);
	return  buf;
}

inline char* decode_PercentBasic(const uint8_t* value) {
	float newVal = (100.0f / 128.0f) * value[0] - 100.0f;
	char* buf = (char*)pvPortMalloc(16);
    if (!buf) return nullptr;
    int intPart = (int)newVal;
	int fracPart = (int)((newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);
    return  buf; //note caller needs to free buffer after use
}

inline char* decode_Percent255(const uint8_t* value) {
	float newVal = (100.0f / 128.0f) * value[0] - 100.0f;
	char* buf = (char*)pvPortMalloc(16);
	if (!buf) return nullptr;
	int intPart = (int)newVal;
	int fracPart = (int)((newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);

	return buf;
}

inline char* decode_PercentDualBanks(const uint8_t* value) {
    float newVal = (100.0f / 128.0f) * value[0] - 100.0f;
	char* buf = (char*)pvPortMalloc(16);
	if (!buf) return nullptr;
	int intPart = (int)newVal;
	int fracPart = (int)(abs(newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);
    return buf; // or bank2, or average, or store both elsewhere
}



inline char* decode_EngineLoad(const uint8_t* value){
	float newVal = value[0] / 2.55f;
	char* buf = (char*)pvPortMalloc(32);
	if (!buf) return nullptr;
	int intPart = (int)newVal;
	int fracPart = (int)((newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);
	return buf;
}

inline char* decode_ThrottlePosition(const uint8_t* value){
	float newVal = (100.0f / 255.0f) * value[0];
	char* buf = (char*)pvPortMalloc(32);
	if (!buf) return nullptr;
	int intPart = (int)newVal;
	int fracPart = (int)((newVal - intPart) * 100);
	snprintf(buf, 16, "%d.%02d", intPart, fracPart);
	return buf;
}


/**
 * To avoid search algorithm the index should = PID. PID 0x01 = index 1. There will be filler values because some are not available on the Wikipedia website but might exist elsewhere.
 * Also any PID % 0x20 == 0 will be a request for available PID bit-masks and should be filler.
 */
const PIDDecoder PIDInfoTable[]{
		{"", "PIDs supported [$01 - $20]", bit_Mask_Request},       										//0
		{"", "Monitor status since DTCs cleared. (Includes malfunction indicator lamp (MIL), status and number of DTCs, components tests, DTC readiness checks)",bit_Map_Request},//1
		{"", "DTC that caused freeze frame to be stored.", bit_Map_Request},                                //2
		{"", "Fuel system status",bit_Map_Request},                                                         //3
		{"%", "Calculated engine load", decode_EngineLoad},                                                 //4
		{"°C", "Engine coolant temperature",decode_EngineTemp},                                             //5
		{"%", "Short term fuel trim (STFT)—Bank 1",decode_PercentDualBanks},                                //6 here************** decode function here needs work to decode correctly and display both two values
		{"%", "Long term fuel trim (LTFT)—Bank 1",decode_PercentDualBanks},                                 //7
		{"%", "Short term fuel trim (STFT)—Bank 2",decode_PercentDualBanks},                                //8
		{"%", "Long term fuel trim (LTFT)—Bank 2",decode_PercentDualBanks},                                 //9 here**************
		{"kPa", "Fuel pressure (gauge pressure)",decode_FuelPressure},                                      //0A
		{"kPa", "Intake manifold absolute pressure",decode_ManifoldABSPressure},                            //0B
		{"rpm", "Engine speed",decode_EngineSpeed},                                                         //0C
		{"km/h", "Vehicle speed",decode_VechicleSpeed},                                               		//0D
		{"° before TDC", "Timing advance",decode_TimeingAdvance},                                           //0E
		{"°C", "Intake air temperature", decode_intakeAirTemp},                                             //0F
		{"g/s", "Mass air flow sensor (MAF) air flow rate", decode_EngineSpeed},                            //10
		{"%", "Throttle position", decode_ThrottlePosition},                                                //11
		{"", "Commanded secondary air status", bit_Map_Request},                                            //12
		{"", "Oxygen sensors present (in 2 banks)", decode_PercentDualBanks},								//13   here not the right function and there are two different return values from this and some others (did not think about this)
		{"V", "Oxygen Sensor 1", bit_Map_Request},                                                          //14  here ************ all in-between the heres are put to something random for testing
		{"V", "Oxygen Sensor 2", bit_Map_Request},                                                          //15
		{"V", "Oxygen Sensor 3", bit_Map_Request},                                                          //16
		{"V", "Oxygen Sensor 4", bit_Map_Request},                                                          //17
		{"V", "Oxygen Sensor 5", bit_Map_Request},                                                          //18
		{"V", "Oxygen Sensor 6", bit_Map_Request},                                                          //19
		{"V", "Oxygen Sensor 7", bit_Map_Request},                                                          //1A here *************



//		{"V", "Oxygen Sensor 2", 13^},                                                                      //15
//		{"V", "Oxygen Sensor 3", 13^},                                                                      //16
//		{"V", "Oxygen Sensor 4", 13^},                                                                      //17
//		{"V", "Oxygen Sensor 5", 13^},                                                                      //18
//		{"V", "Oxygen Sensor 6", 13^},                                                                      //19
//		{"V", "Oxygen Sensor 7", 13^},                                                                      //1A
//		{"V", "Oxygen Sensor 8", 13^},                                                                      //1B
//		{"", "OBD standards this vehicle conforms to", 14^},                                                //1C
//		{"", "Oxygen sensors present (in 4 banks)", 15^},                                                   //1D
//		{"", "Auxiliary input status", 16^},                                                                //1E
//		{"s", "Run time since engine start", 17^},                                                          //1F
//		{"", "PIDs supported [$21 - $40]", 18^},                                                            //20
//		{"km", "Distance traveled with malfunction indicator lamp (MIL) on", 17^},                          //21
//		{"kPa", "Fuel Rail Pressure (relative to manifold vacuum)", 19^},                                   //22
//		{"kPa", "Fuel Rail Gauge Pressure (diesel, or gasoline direct injection)", 20^},                    //23
//		{"ratio", "Oxygen Sensor 1", 21^},                                                                  //24
//		{"ratio", "Oxygen Sensor 2", 21^},                                                                  //25
//		{"ratio", "Oxygen Sensor 3", 21^},                                                                  //26
//		{"ratio", "Oxygen Sensor 4", 21^},                                                                  //27
//		{"ratio", "Oxygen Sensor 5", 21^},                                                                  //28
//		{"ratio", "Oxygen Sensor 6", 21^},                                                                  //29
//		{"ratio", "Oxygen Sensor 7", 21^},                                                                  //2A
//		{"ratio", "Oxygen Sensor 8", 21^},                                                                  //2B
//		{"%", "Commanded EGR", 11^},                                                                        //2C
//		{"%", "EGR Error", 22^},                                                                            //2D
//		{"%", "Commanded evaporative purge", 11^},                                                          //2E
//		{"%", "Fuel Tank Level Input", 11^},                                                                //2F
//		{"", "Warm-ups since codes cleared", 7^},                                                           //30
//		{"km", "Distance traveled since codes cleared", 17^},                                               //31
//		{"Pa", "Evap. System Vapor Pressure", 8^},                                                          //32
//		{"kPa", "Absolute Barometric Pressure", 7^},                                                        //33
//		{"ratio", "Oxygen Sensor 1", 23^},                                                                  //34
//		{"ratio", "Oxygen Sensor 2", 23^},                                                                  //35
//		{"ratio", "Oxygen Sensor 3", 23^},                                                                  //36
//		{"ratio", "Oxygen Sensor 4", 23^},                                                                  //37
//		{"ratio", "Oxygen Sensor 5", 23^},                                                                  //38
//		{"ratio", "Oxygen Sensor 6", 23^},                                                                  //39
//		{"ratio", "Oxygen Sensor 7", 23^},                                                                  //3A
//		{"ratio", "Oxygen Sensor 8", 23^},                                                                  //3B
//		{"°C", "Catalyst Temperature: Bank 1, Sensor 1", 24^},                                              //3C
//		{"°C", "Catalyst Temperature: Bank 2, Sensor 1", 24^},                                              //3D
//		{"°C", "Catalyst Temperature: Bank 1, Sensor 2", 24^},                                              //3E
//		{"°C", "Catalyst Temperature: Bank 2, Sensor 2", 24^},                                              //3F
//		{"", "PIDs supported [$41 - $60]", 25^},                                                            //40
//		{"", "Monitor status this drive cycle", 1^},                                                        //41
//		{"V", "Control module voltage", 26^},                                                               //42
//		{"%", "Absolute load value", 27^},                                                                  //43
//		{"ratio", "Commanded Air-Fuel Equivalence Ratio (lambda,?)", 28^},                                  //44
//		{"%", "Relative throttle position", 11^},                                                           //45
//		{"°C", "Ambient air temperature", 4^},                                                              //46
//		{"%", "Absolute throttle position B", 11^},                                                         //47
//		{"%", "Absolute throttle position C", 11^},                                                         //48
//		{"%", "Accelerator pedal position D", 11^},                                                         //49
//		{"%", "Accelerator pedal position E", 11^},                                                         //4A
//		{"%", "Accelerator pedal position F", 11^},                                                         //4B
//		{"%", "Commanded throttle actuator", 11^},                                                          //4C
//		{"min", "Time run with MIL on", 17^},                                                               //4D
//		{"min", "Time since trouble codes cleared", 17^},                                                   //4E
//		{"ratio, V, mA, kPa", "Maximum value for Fuel–Air equivalence ratio, oxygen sensor voltage, oxygen sensor current, and intake manifold absolute pressure", 29^},//4F
//		{"g/s", "Maximum value for air flow rate from mass air flow sensor", 30^},                          //50
//		{"", "Fuel Type", 31^},                                                                             //51
//		{"%", "Ethanol fuel %", 11^},                                                                       //52
//		{"kPa", "Absolute Evap system Vapor Pressure", 32^},                                                //53
//		{"Pa", "Evap system vapor pressure", 33^},                                                          //54
//		{"%", "Short term secondary oxygen sensor trim, A: bank 1, B: bank 3", 22^},                        //55
//		{"%", "Long term secondary oxygen sensor trim, A: bank 1, B: bank 3", 22^},                         //56
//		{"%", "Short term secondary oxygen sensor trim, A: bank 2, B: bank 4", 22^},                        //57
//		{"%", "Long term secondary oxygen sensor trim, A: bank 2, B: bank 4", 22^},                         //58
//		{"kPa", "Fuel rail absolute pressure", 20^},                                                        //59
//		{"%", "Relative accelerator pedal position", 11^},                                                  //5A
//		{"%", "Hybrid battery pack remaining life", 11^},                                                   //5B
//		{"°C", "Engine oil temperature", 4^},                                                               //5C
//		{"°", "Fuel injection timing", 34^},                                                                //5D
//		{"L/h", "Engine fuel rate", 35^},                                                                   //5E
//		{"", "Emission requirements to which vehicle is designed", 36^},                                    //5F
//		{"", "PIDs supported [$61 - $80]", 37^},                                                            //60
//		{"%", "Driver's demand engine - percent torque", 38^},                                              //61
//		{"%", "Actual engine - percent torque", 38^},                                                       //62
//		{"N?m", "Engine reference torque", 17^},                                                            //63
//		{"%", "Engine percent torque data", 39^},                                                           //64
//		{"", "Auxiliary input / output supported", 36^},                                                    //65
//		{"g/s", "Mass air flow sensor", 40^},                                                               //66
//		{"°C", "Engine coolant temperature", 41^},                                                          //67
//		{"°C", "Intake air temperature sensor", 41^},                                                       //68
//		{"", "Actual EGR, Commanded EGR, and EGR Error", 42^},                                              //69
//		{"", "Commanded Diesel intake air flow control and relative intake air flow position", 42^},        //6A
//		{"", "Exhaust gas recirculation temperature", 42^},                                                 //6B
//		{"", "Commanded throttle actuator control and relative throttle position", 42^},                    //6C
//		{"", "Fuel pressure control system", 42^},                                                          //6D
//		{"", "Injection pressure control system", 42^},                                                     //6E
//		{"", "Turbocharger compressor inlet pressure", 42^},                                                //6F
//		{"", "Boost pressure control", 42^},                                                                //70
//		{"", "Variable Geometry turbo (VGT) control", 42^},                                                 //71
//		{"", "Wastegate control", 42^},                                                                     //72
//		{"", "Exhaust pressure", 42^},                                                                      //73
//		{"", "Turbocharger RPM", 42^},                                                                      //74
//		{"", "Turbocharger temperature", 42^},                                                              //75
//		{"", "Turbocharger temperature", 42^},                                                              //76
//		{"", "Charge air cooler temperature (CACT)", 42^},                                                  //77
//		{"", "Exhaust Gas temperature (EGT) Bank 1", 43^},                                                  //78
//		{"", "Exhaust Gas temperature (EGT) Bank 2", 43^},                                                  //79
//		{"", "Diesel particulate filter (DPF)", 42^},                                                       //7A
//		{"", "Diesel particulate filter (DPF)", 42^},                                                       //7B
//		{"°C", "Diesel Particulate filter (DPF) temperature", 24^},                                         //7C
//		{"", "NOx NTE (Not-To-Exceed) control area status", 42^},                                           //7D
//		{"", "PM NTE (Not-To-Exceed) control area status", 42^},                                            //7E
//		{"s", "Engine run time [b]", 44^},                                                                  //7F
//		{"", "PIDs supported [$81 - $A0]", 45^},                                                            //80
//		{"", "Engine run time for Auxiliary Emissions Control Device(AECD)", 42^},                          //81
//		{"", "Engine run time for Auxiliary Emissions Control Device(AECD)", 42^},                          //82
//		{"", "NOx sensor", 42^},                                                                            //83
//		{"", "Manifold surface temperature", 42^},                                                          //84
//		{"%", "NOx reagent system", 46^},                                                                   //85
//		{"", "Particulate matter (PM) sensor", 42^},                                                        //86
//		{"", "Intake manifold absolute pressure", 42^},                                                     //87
//		{"", "SCR Induce System", 42^},                                                                     //88
//		{"", "Run Time for AECD #11-#15", 42^},                                                             //89
//		{"", "Run Time for AECD #16-#20", 42^},                                                             //8A
//		{"", "Diesel Aftertreatment", 42^},                                                                 //8B
//		{"", "O2 Sensor (Wide Range)", 42^},                                                                //8C
//		{"%", "Throttle Position G", 42^},                                                                  //8D
//		{"%", "Engine Friction - Percent Torque", 38^},                                                     //8E
//		{"", "PM Sensor Bank 1 & 2", 42^},                                                                  //8F
//		{"h", "WWH-OBD Vehicle OBD System Information", 42^},                                               //90
//		{"h", "WWH-OBD Vehicle OBD System Information", 42^},                                               //91
//		{"", "Fuel System Control", 42^},                                                                   //92
//		{"h", "WWH-OBD Vehicle OBD Counters support", 42^},                                                 //93
//		{"", "NOx Warning And Inducement System", 42^},                                                     //94
//		{"", "Exhaust Gas Temperature Sensor", 42^},                                                        //98
//		{"", "Exhaust Gas Temperature Sensor", 42^},                                                        //99
//		{"", "Hybrid/EV Vehicle System Data, Battery, Voltage", 42^},                                       //9A
//		{"%", "Diesel Exhaust Fluid Sensor Data", 47^},                                                     //9B
//		{"", "O2 Sensor Data", 42^},                                                                        //9C
//		{"g/s", "Engine Fuel Rate", 42^},                                                                   //9D
//		{"kg/h", "Engine Exhaust Flow Rate", 42^},                                                          //9E
//		{"", "Fuel System Percentage Use", 42^},                                                            //9F
//		{"", "PIDs supported [$A1 - $C0]", 48^},                                                            //A0
//		{"ppm", "NOx Sensor Corrected Data", 42^},                                                          //A1
//		{"mg/stroke", "Cylinder Fuel Rate", 49^},                                                           //A2
//		{"Pa", "Evap System Vapor Pressure", 42^},                                                          //A3
//		{"ratio", "Transmission Actual Gear", 50^},                                                         //A4
//		{"%", "Commanded Diesel Exhaust Fluid Dosing", 51^},                                                //A5
//		{"km", "Odometer [c]", 52^},                                                                        //A6
//		{"", "NOx Sensor Concentration Sensors 3 and 4", 42^},                                              //A7
//		{"", "NOx Sensor Corrected Concentration Sensors 3 and 4", 42^},                                    //A8
//		{"", "ABS Disable Switch State", 51^},                                                              //A9
//		{"", "PIDs supported [$C1 - $E0]", 53^},                                                            //C0
//		{"%", "Fuel Level Input A/B", 54^},                                                                 //C3
//		{"seconds / Count", "Exhaust Particulate Control System Diagnostic Time/Count", 55^},               //C4
//		{"kPa", "Fuel Pressure A and B", 42^},                                                              //C5
//		{"h", "Byte 1 - Particulate control - driver inducement system status", 42^},                       //C6
//		{"km", "Distance Since Reflash or Module Replacement", 42^},                                        //C7
//		{"Bit", "NOx Control Diagnostic (NCD) and Particulate Control Diagnostic (PCD) Warning Lamp status", 42^},//C8


};

#endif //C++ funtions
#endif /* APPLICATION_USER_GUI_PIDS_DESC_HPP_ */
