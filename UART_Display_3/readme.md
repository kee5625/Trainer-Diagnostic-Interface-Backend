#Introduction
This dispaly code is meant to be used with an STM32H750B-DK kit to talk over UART to a gateway chip (ESP32 was test with it). 
The Display will request information from the gateway which will then grab it from a CAN bus. CAN bus follows the public standard ISO 11898-1/ ISO 11898-2. 
See table below of modes integrated into display. For explainations of modes see individual section in project reamd me.

#Integrated/Supported Services/Modes 
01	Show current data*****************************************************************************************Integrated
02	Show freeze frame data************************************************************************************Not yet fully integrated
03	Show stored Diagnostic Trouble Codes**********************************************************************Integrated
04	Clear Diagnostic Trouble Codes and stored values**********************************************************Integrated
07	Show pending Diagnostic Trouble Codes (detected during current or last driving cycle)*********************Integrated
0A	Permanent Diagnostic Trouble Codes (DTCs) (Cleared DTCs)**************************************************Integrated


#UART - Display interaction
UART is used to communicate with the Gateway chip which will provide data request by the display. In most cases the display will start the interaction by sending a 
start byte which is defined in the UART_COMMS.hpp file. Padding on the byte will leave it looking like 1dddd110 with d being for data(this can later be removed to implemet more commands)
below is the command flow for most modes/services. See description above mode/service function for full run through. 

Byte 0 (Display): Start command
Byte 0 (Gateway): Receive commands
Byte 1 (Display): Request command 
Byte 1 (Gateway): Data
Byte N (Display): Received
Byte N (Gateway): Data(if more than one byte)
Byte N + 1 (Display): Request command
Byte N + 1 (Gateway): Done command
N = number of data bytes

#Sources
1. https://en.wikipedia.org/wiki/OBD-II_PIDs