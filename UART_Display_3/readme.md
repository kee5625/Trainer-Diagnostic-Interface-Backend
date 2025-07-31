#Introduction
This dispaly code is meant to be used with an STM32H750B-DK kit to talk over UART to a gateway chip (ESP32 was test with it). The Display will request information from the gateway which will then grab it from a CAN bus. CAN bus follows the public standard ISO 11898-1/ ISO 11898-2. See in the following table all modes/services integrated into display. For explainations on how modes work see individual section below.


#Integrated Services/Modes
01	Show current data*****************************************************************************************Integrated
02	Show freeze frame data************************************************************************************Not yet fully integrated
03	Show stored Diagnostic Trouble Codes**********************************************************************Integrated
04	Clear Diagnostic Trouble Codes and stored values**********************************************************Integrated
05	Test results, oxygen sensor monitoring (non CAN only)*****************************************************Not Integrated
06	Test results, other component/system monitoring (Test results, oxygen sensor monitoring for CAN only)*****Not Integrated
07	Show pending Diagnostic Trouble Codes (detected during current or last driving cycle)*********************Integrated
08	Control operation of on-board component/system************************************************************Not Integrated
09	Request vehicle information*******************************************************************************Not Integrated
0A	Permanent Diagnostic Trouble Codes (DTCs) (Cleared DTCs)**************************************************Integrated


#Mode 01- Show Current Data (Live Data)

By pressing the live data button on the display it will brinig you to the live data screen which will pull data off the ECU/Trainer and update in real time. To do this the display first request a bitmask of all available pids from the gateway which will then request the bitmask from the ECU(electronic control unit) using certaint PIDs(0x00, 0x20, 0x40, ...). PIDs are part of service 1 that request certain info from the ECUs. The bitmask is grabed by requesting 32 or 0x20 pids at a time, the full bitmask of availble pids is a uint8_t [7][4] array (224 pids total). The total amount of 1's in this bitmask is the total number of available PIDs (full list of pids at wiki in sources section below). There is always a checksum sent from the gateway to the display to check for errors. 

#Mode 03 (stored DTCs), 07 (Pending DTCs), and 0A (Permanent DTCs)

These modes all work in the same way. When the user is on the read codes screen after pressing the read codes button they can press any one of three buttons to grab codes. When a button is pressed the appropriate request will be sent through uart with respect to the UART_COMMS_t. Note that the bytes are packed so that they look like 1dddd110 with d being data(command) and the padding for consistency is at the begging and end. No error checking is used other than built in UART parody which all UART uses in this project.  

#Mode 04

Mode 4 is only available on the read codes page and is activated by pressing the clear codes button when codes are being dispalyed. Pressing the button sends a UART packed byte with the reset command in UART_COMM_t to the gateway which resets the DTCs on the ECU/trainer. 




#Sources
1. https://en.wikipedia.org/wiki/OBD-II_PIDs