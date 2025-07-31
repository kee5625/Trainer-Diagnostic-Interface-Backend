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

typedef char* (*DecoderFunc)(const uint8_t* value);

struct PIDDecoder {
    const char* unit;
    const char *description;
    DecoderFunc decode;
};

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
//    return {DecodeType::FLOAT, .floatValue = (100.0f / 255.0f) * value[0]};
}

inline char* decode_PercentDualBanks(const uint8_t* value) {
    float newVal = (100.0f / 128.0f) * value[0] - 100;
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

//PID code = index 0x01 = 1
const PIDDecoder PIDInfoTable[]{
		{"", "PIDs supported [$01 - $20]", bit_Mask_Request},       										//0
		{"", "Monitor status since DTCs cleared. (Includes malfunction indicator lamp (MIL), status and number of DTCs, components tests, DTC readiness checks)",bit_Map_Request},//1
		{"", "DTC that caused freeze frame to be stored.", bit_Map_Request},                                //2
		{"", "Fuel system status",bit_Map_Request},                                                         //3
		{"%", "Calculated engine load", decode_EngineLoad},                                                 //4
		{"°C", "Engine coolant temperature",decode_EngineTemp},                                             //5
		{"%", "Short term fuel trim (STFT)—Bank 1",decode_PercentDualBanks},                                //6
		{"%", "Long term fuel trim (LTFT)—Bank 1",decode_PercentDualBanks},                                 //7
		{"%", "Short term fuel trim (STFT)—Bank 2",decode_PercentDualBanks},                                //8
		{"%", "Long term fuel trim (LTFT)—Bank 2",decode_PercentDualBanks},                                 //9
		{"kPa", "Fuel pressure (gauge pressure)",decode_FuelPressure},                                      //0A
		{"kPa", "Intake manifold absolute pressure",decode_ManifoldABSPressure},                            //0B
		{"rpm", "Engine speed",decode_EngineSpeed},                                                         //0C
		{"km/h", "Vehicle speed",decode_ManifoldABSPressure},                                               //0D
		{"° before TDC", "Timing advance",decode_VechicleSpeed},                                            //0E
		{"°C", "Intake air temperature",decode_EngineTemp},                                                 //0E
		{"°C", "Intake air temperature", decode_intakeAirTemp},                                             //0F
		{"g/s", "Mass air flow sensor (MAF) air flow rate", decode_EngineSpeed},                            //10
		{"%", "Throttle position", decode_ThrottlePosition},                                                //11
		{"", "Commanded secondary air status", bit_Map_Request},                                            //12
		{"", "Oxygen sensors present (in 2 banks)", decode_PercentDualBanks},								//13  not the right function



//		{"°C", "Intake air temperature", 4^},                                                               //0F
//		{"g/s", "Mass air flow sensor (MAF) air flow rate", 10^},                                           //10
//		{"%", "Throttle position", 11^},                                                                    //11
//		{"", "Commanded secondary air status", 1^},                                                         //12
//		{"", "Oxygen sensors present (in 2 banks)", 12^},                                                   //13
//		{"V", "Oxygen Sensor 1", 13^},                                                                      //14
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

constexpr const char * const pid_desc_bank[num_PIDS] = {
		"PIDs supported [$01 - $20]",                                                                       //0x0
		"Monitor status since DTCs cleared. (Includes malfunction indicator lamp (MIL), status and number of DTCs, components tests, DTC readiness checks)",//0x1
		"DTC that caused freeze frame to be stored.",                                                       //0x2
		"Fuel system status",                                                                               //0x3
		"Calculated engine load",                                                                           //0x4
		"Engine coolant temperature",                                                                       //0x5
		"Short term fuel trim (STFT)—Bank 1",                                                               //0x6
		"Long term fuel trim (LTFT)—Bank 1",                                                                //0x7
		"Short term fuel trim (STFT)—Bank 2",                                                               //0x8
		"Long term fuel trim (LTFT)—Bank 2",                                                                //0x9
		"Fuel pressure (gauge pressure)",                                                                   //0x0A
		"Intake manifold absolute pressure",                                                                //0x0B
		"Engine speed",                                                                                     //0x0C
		"Vehicle speed",                                                                                    //0x0D
		"Timing advance",                                                                                   //0x0E
		"Intake air temperature",                                                                           //0x0F
		"Mass air flow sensor (MAF) air flow rate",                                                         //0x10
		"Throttle position",                                                                                //0x11
		"Commanded secondary air status",                                                                   //0x12
		"Oxygen sensors present (in 2 banks)",                                                              //0x13
		"Oxygen Sensor 1",                                                                                  //0x14
		"Oxygen Sensor 2",                                                                                  //0x15
		"Oxygen Sensor 3",                                                                                  //0x16
		"Oxygen Sensor 4",                                                                                  //0x17
		"Oxygen Sensor 5",                                                                                  //0x18
		"Oxygen Sensor 6",                                                                                  //0x19
		"Oxygen Sensor 7",                                                                                  //0x1A
		"Oxygen Sensor 8",                                                                                  //0x1B
		"OBD standards this vehicle conforms to",                                                           //0x1C
		"Oxygen sensors present (in 4 banks)",                                                              //0x1D
		"Auxiliary input status",                                                                           //0x1E
		"Run time since engine start",                                                                      //0x1F
		"PIDs supported [$21 - $40]",                                                                       //0x20
		"Distance traveled with malfunction indicator lamp (MIL) on",                                       //0x21
		"Fuel Rail Pressure (relative to manifold vacuum)",                                                 //0x22
		"Fuel Rail Gauge Pressure (diesel, or gasoline direct injection)",                                  //0x23
		"Oxygen Sensor 1",                                                                                  //0x24
		"Oxygen Sensor 2",                                                                                  //0x25
		"Oxygen Sensor 3",                                                                                  //0x26
		"Oxygen Sensor 4",                                                                                  //0x27
		"Oxygen Sensor 5",                                                                                  //0x28
		"Oxygen Sensor 6",                                                                                  //0x29
		"Oxygen Sensor 7",                                                                                  //0x2A
		"Oxygen Sensor 8",                                                                                  //0x2B
		"Commanded EGR",                                                                                    //0x2C
		"EGR Error",                                                                                        //0x2D
		"Commanded evaporative purge",                                                                      //0x2E
		"Fuel Tank Level Input",                                                                            //0x2F
		"Warm-ups since codes cleared",                                                                     //0x30
		"Distance traveled since codes cleared",                                                            //0x31
		"Evap. System Vapor Pressure",                                                                      //0x32
		"Absolute Barometric Pressure",                                                                     //0x33
		"Oxygen Sensor 1",                                                                                  //0x34
		"Oxygen Sensor 2",                                                                                  //0x35
		"Oxygen Sensor 3",                                                                                  //0x36
		"Oxygen Sensor 4",                                                                                  //0x37
		"Oxygen Sensor 5",                                                                                  //0x38
		"Oxygen Sensor 6",                                                                                  //0x39
		"Oxygen Sensor 7",                                                                                  //0x3A
		"Oxygen Sensor 8",                                                                                  //0x3B
		"Catalyst Temperature: Bank 1, Sensor 1",                                                           //0x3C
		"Catalyst Temperature: Bank 2, Sensor 1",                                                           //0x3D
		"Catalyst Temperature: Bank 1, Sensor 2",                                                           //0x3E
		"Catalyst Temperature: Bank 2, Sensor 2",                                                           //0x3F
		"PIDs supported [$41 - $60]",                                                                       //0x40
		"Monitor status this drive cycle",                                                                  //0x41
		"Control module voltage",                                                                           //0x42
		"Absolute load value",                                                                              //0x43
		"Commanded Air-Fuel Equivalence Ratio (lambda,?)",                                                  //0x44
		"Relative throttle position",                                                                       //0x45
		"Ambient air temperature",                                                                          //0x46
		"Absolute throttle position B",                                                                     //0x47
		"Absolute throttle position C",                                                                     //0x48
		"Accelerator pedal position D",                                                                     //0x49
		"Accelerator pedal position E",                                                                     //0x4A
		"Accelerator pedal position F",                                                                     //0x4B
		"Commanded throttle actuator",                                                                      //0x4C
		"Time run with MIL on",                                                                             //0x4D
		"Time since trouble codes cleared",                                                                 //0x4E
		"Maximum value for Fuel–Air equivalence ratio, oxygen sensor voltage, oxygen sensor current, and intake manifold absolute pressure",//0x4F
		"Maximum value for air flow rate from mass air flow sensor",                                        //0x50
		"Fuel Type",                                                                                        //0x51
		"Ethanol fuel %",                                                                                   //0x52
		"Absolute Evap system Vapor Pressure",                                                              //0x53
		"Evap system vapor pressure",                                                                       //0x54
		"Short term secondary oxygen sensor trim, A: bank 1, B: bank 3",                                    //0x55
		"Long term secondary oxygen sensor trim, A: bank 1, B: bank 3",                                     //0x56
		"Short term secondary oxygen sensor trim, A: bank 2, B: bank 4",                                    //0x57
		"Long term secondary oxygen sensor trim, A: bank 2, B: bank 4",                                     //0x58
		"Fuel rail absolute pressure",                                                                      //0x59
		"Relative accelerator pedal position",                                                              //0x5A
		"Hybrid battery pack remaining life",                                                               //0x5B
		"Engine oil temperature",                                                                           //0x5C
		"Fuel injection timing",                                                                            //0x5D
		"Engine fuel rate",                                                                                 //0x5E
		"Emission requirements to which vehicle is designed",                                               //0x5F
		"PIDs supported [$61 - $80]",                                                                       //0x60
		"Driver's demand engine - percent torque",                                                          //0x61
		"Actual engine - percent torque",                                                                   //0x62
		"Engine reference torque",                                                                          //0x63
		"Engine percent torque data",                                                                       //0x64
		"Auxiliary input / output supported",                                                               //0x65
		"Mass air flow sensor",                                                                             //0x66
		"Engine coolant temperature",                                                                       //0x67
		"Intake air temperature sensor",                                                                    //0x68
		"Actual EGR, Commanded EGR, and EGR Error",                                                         //0x69
		"Commanded Diesel intake air flow control and relative intake air flow position",                   //0x6A
		"Exhaust gas recirculation temperature",                                                            //0x6B
		"Commanded throttle actuator control and relative throttle position",                               //0x6C
		"Fuel pressure control system",                                                                     //0x6D
		"Injection pressure control system",                                                                //0x6E
		"Turbocharger compressor inlet pressure",                                                           //0x6F
		"Boost pressure control",                                                                           //0x70
		"Variable Geometry turbo (VGT) control",                                                            //0x71
		"Wastegate control",                                                                                //0x72
		"Exhaust pressure",                                                                                 //0x73
		"Turbocharger RPM",                                                                                 //0x74
		"Turbocharger temperature",                                                                         //0x75
		"Turbocharger temperature",                                                                         //0x76
		"Charge air cooler temperature (CACT)",                                                             //0x77
		"Exhaust Gas temperature (EGT) Bank 1",                                                             //0x78
		"Exhaust Gas temperature (EGT) Bank 2",                                                             //0x79
		"Diesel particulate filter (DPF)",                                                                  //0x7A
		"Diesel particulate filter (DPF)",                                                                  //0x7B
		"Diesel Particulate filter (DPF) temperature",                                                      //0x7C
		"NOx NTE (Not-To-Exceed) control area status",                                                      //0x7D
		"PM NTE (Not-To-Exceed) control area status",                                                       //0x7E
		"Engine run time [b]",                                                                              //0x7F
		"PIDs supported [$81 - $A0]",                                                                       //0x80
		"Engine run time for Auxiliary Emissions Control Device(AECD)",                                     //0x81
		"Engine run time for Auxiliary Emissions Control Device(AECD)",                                     //0x82
		"NOx sensor",                                                                                       //0x83
		"Manifold surface temperature",                                                                     //0x84
		"NOx reagent system",                                                                               //0x85
		"Particulate matter (PM) sensor",                                                                   //0x86
		"Intake manifold absolute pressure",                                                                //0x87
		"SCR Induce System",                                                                                //0x88
		"Run Time for AECD #11-#15",                                                                        //0x89
		"Run Time for AECD #16-#20",                                                                        //0x8A
		"Diesel Aftertreatment",                                                                            //0x8B
		"O2 Sensor (Wide Range)",                                                                           //0x8C
		"Throttle Position G",                                                                              //0x8D
		"Engine Friction - Percent Torque",                                                                 //0x8E
		"PM Sensor Bank 1 & 2",                                                                             //0x8F
		"WWH-OBD Vehicle OBD System Information",                                                           //0x90
		"WWH-OBD Vehicle OBD System Information",                                                           //0x91
		"Fuel System Control",                                                                              //0x92
		"WWH-OBD Vehicle OBD Counters support",                                                             //0x93
		"NOx Warning And Inducement System",                                                                //0x94
		"Exhaust Gas Temperature Sensor",                                                                   //0x98
		"Exhaust Gas Temperature Sensor",                                                                   //0x99
		"Hybrid/EV Vehicle System Data, Battery, Voltage",                                                  //0x9A
		"Diesel Exhaust Fluid Sensor Data",                                                                 //0x9B
		"O2 Sensor Data",                                                                                   //0x9C
		"Engine Fuel Rate",                                                                                 //0x9D
		"Engine Exhaust Flow Rate",                                                                         //0x9E
		"Fuel System Percentage Use",                                                                       //0x9F
		"PIDs supported [$A1 - $C0]",                                                                       //0xA0
		"NOx Sensor Corrected Data",                                                                        //0xA1
		"Cylinder Fuel Rate",                                                                               //0xA2
		"Evap System Vapor Pressure",                                                                       //0xA3
		"Transmission Actual Gear",                                                                         //0xA4
		"Commanded Diesel Exhaust Fluid Dosing",                                                            //0xA5
		"Odometer [c]",                                                                                     //0xA6
		"NOx Sensor Concentration Sensors 3 and 4",                                                         //0xA7
		"NOx Sensor Corrected Concentration Sensors 3 and 4",                                               //0xA8
		"ABS Disable Switch State",                                                                         //0xA9
		"PIDs supported [$C1 - $E0]",                                                                       //0xC0
		"Fuel Level Input A/B",                                                                             //0xC3
		"Exhaust Particulate Control System Diagnostic Time/Count",                                         //0xC4
		"Fuel Pressure A and B",                                                                            //0xC5
		"Byte 1 - Particulate control - driver inducement system status",                                   //0xC6
		"Distance Since Reflash or Module Replacement",                                                     //0xC7
		"NOx Control Diagnostic (NCD) and Particulate Control Diagnostic (PCD) Warning Lamp status",        //0xC8


};

//blanks mean it is a string return value, bit map, etc...
constexpr const char * const unit_LUT[num_PIDS] = {
		"",                                               //0x0
		"",                                               //0x1
		"",                                               //0x2
		"",                                               //0x3
		"%",                                              //0x4
		"°C",                                             //0x5
		"%",                                              //0x6
		"%",                                              //0x7
		"%",                                              //0x8
		"%",                                              //0x9
		"kPa",                                            //0x0A
		"kPa",                                            //0x0B
		"rpm",                                            //0x0C
		"km/h",                                           //0x0D
		"° before TDC",                                   //0x0E
		"°C",                                             //0x0F
		"g/s",                                            //0x10
		"%",                                              //0x11
		"",                                               //0x12
		"",                                               //0x13
		"V",                                              //0x14
		"V",                                              //0x15
		"V",                                              //0x16
		"V",                                              //0x17
		"V",                                              //0x18
		"V",                                              //0x19
		"V",                                              //0x1A
		"V",                                              //0x1B
		"",                                               //0x1C
		"",                                               //0x1D
		"",                                               //0x1E
		"s",                                              //0x1F
		"",                                               //0x20
		"km",                                             //0x21
		"kPa",                                            //0x22
		"kPa",                                            //0x23
		"ratio",                                          //0x24
		"ratio",                                          //0x25
		"ratio",                                          //0x26
		"ratio",                                          //0x27
		"ratio",                                          //0x28
		"ratio",                                          //0x29
		"ratio",                                          //0x2A
		"ratio",                                          //0x2B
		"%",                                              //0x2C
		"%",                                              //0x2D
		"%",                                              //0x2E
		"%",                                              //0x2F
		"",                                               //0x30
		"km",                                             //0x31
		"Pa",                                             //0x32
		"kPa",                                            //0x33
		"ratio",                                          //0x34
		"ratio",                                          //0x35
		"ratio",                                          //0x36
		"ratio",                                          //0x37
		"ratio",                                          //0x38
		"ratio",                                          //0x39
		"ratio",                                          //0x3A
		"ratio",                                          //0x3B
		"°C",                                             //0x3C
		"°C",                                             //0x3D
		"°C",                                             //0x3E
		"°C",                                             //0x3F
		"",                                               //0x40
		"",                                               //0x41
		"V",                                              //0x42
		"%",                                              //0x43
		"ratio",                                          //0x44
		"%",                                              //0x45
		"°C",                                             //0x46
		"%",                                              //0x47
		"%",                                              //0x48
		"%",                                              //0x49
		"%",                                              //0x4A
		"%",                                              //0x4B
		"%",                                              //0x4C
		"min",                                            //0x4D
		"min",                                            //0x4E
		"ratio, V, mA, kPa",                              //0x4F
		"g/s",                                            //0x50
		"",                                               //0x51
		"%",                                              //0x52
		"kPa",                                            //0x53
		"Pa",                                             //0x54
		"%",                                              //0x55
		"%",                                              //0x56
		"%",                                              //0x57
		"%",                                              //0x58
		"kPa",                                            //0x59
		"%",                                              //0x5A
		"%",                                              //0x5B
		"°C",                                             //0x5C
		"°",                                              //0x5D
		"L/h",                                            //0x5E
		"",                                               //0x5F
		"",                                               //0x60
		"%",                                              //0x61
		"%",                                              //0x62
		"N?m",                                            //0x63
		"%",                                              //0x64
		"",                                               //0x65
		"g/s",                                            //0x66
		"°C",                                             //0x67
		"°C",                                             //0x68
		"",                                               //0x69
		"",                                               //0x6A
		"",                                               //0x6B
		"",                                               //0x6C
		"",                                               //0x6D
		"",                                               //0x6E
		"",                                               //0x6F
		"",                                               //0x70
		"",                                               //0x71
		"",                                               //0x72
		"",                                               //0x73
		"",                                               //0x74
		"",                                               //0x75
		"",                                               //0x76
		"",                                               //0x77
		"",                                               //0x78
		"",                                               //0x79
		"",                                               //0x7A
		"",                                               //0x7B
		"°C",                                             //0x7C
		"",                                               //0x7D
		"",                                               //0x7E
		"s",                                              //0x7F
		"",                                               //0x80
		"",                                               //0x81
		"",                                               //0x82
		"",                                               //0x83
		"",                                               //0x84
		"%",                                              //0x85
		"",                                               //0x86
		"",                                               //0x87
		"",                                               //0x88
		"",                                               //0x89
		"",                                               //0x8A
		"",                                               //0x8B
		"",                                               //0x8C
		"%",                                              //0x8D
		"%",                                              //0x8E
		"",                                               //0x8F
		"h",                                              //0x90
		"h",                                              //0x91
		"",                                               //0x92
		"h",                                              //0x93
		"",                                               //0x94
		"",                                               //0x98
		"",                                               //0x99
		"",                                               //0x9A
		"%",                                              //0x9B
		"",                                               //0x9C
		"g/s",                                            //0x9D
		"kg/h",                                           //0x9E
		"",                                               //0x9F
		"",                                               //0xA0
		"ppm",                                            //0xA1
		"mg/stroke",                                      //0xA2
		"Pa",                                             //0xA3
		"ratio",                                          //0xA4
		"%",                                              //0xA5
		"km",                                             //0xA6
		"",                                               //0xA7
		"",                                               //0xA8
		"",                                               //0xA9
		"",                                               //0xC0
		"%",                                              //0xC3
		"seconds / Count",                                //0xC4
		"kPa",                                            //0xC5
		"h",                                              //0xC6
		"km",                                             //0xC7
		"Bit",                                            //0xC8
};

#endif //C++ funtions
#endif /* APPLICATION_USER_GUI_PIDS_DESC_HPP_ */
