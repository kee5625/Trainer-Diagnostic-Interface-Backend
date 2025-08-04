# Introduction 
This project aims to create a diagnostic interface for all future trainers. To do this a gateway will be used to communicate with external displays/devices. 
The gateway will communicate with the trainer through CAN (see diagram in repo for layout). The functionality of this interface will allow it to display trouble codes, 
reset trouble codes, and display live data to user.

# ------------------------------------------------------------Gateway test setup---------------------------------------------------------------------------
# Getting Started
To properly setup the development in your system, follow these steps:
1. If using VS Code, install the "ESP-IDF" Extension.
2. Clone the repository into a local folder.
3. The "ESP-IDF Welcome" page will be automatically prompted by ESP-IDF (If not, type ">ESP-IDF: Configure ESP-IDF Extension" onto the search bar).
4. Select "Configure extension".
5. Choose the "Express" mode (unless a configuration already exists; then select "Use Existing Setup") and follow the steps to configure the extension.

# Breadboard Setup
Use two esp32 or one esp32 or using an esp32, trainer, and app/display for testing. Current code is setup only for two esp32's.
1. Set up esp32 so they are powered and respective programs onto them (see build and flashing).
2. Create a can bus using 2 transeivers and 2 terminating resistors.
3. Connect both RX and tx pin for TWAI to the 2 transceivers based on transceiver data sheet (rx pin only designed to read 3.3v do not use for rx input 5v). 
4. Connect RX uart pin to TX pin uart pin of opposite board for both (this is for testing with two esp32s).
Note: If using external display and trainer then step 4 will not apply. With trainer, connect trainer to CAN bus. With a dispaly connect it to the gateway through uart. 
If using bt connect dispaly/app through bt.

# -------------------------------------------------------------General Help --------------------------------------------------------------------------------
# Build and Flash
(NOTE: Make sure you open the sub-folder with the main.c file in vs code BEFORE building. Trying to build the entire repo will NOT work.)
1. Re-configure ESP-IDF Extension after opening the sub-folder.
2. Type ">ESP-IDF: Build your project" into search bar or press "Ctrl + E, B".
2. To monitor/flash: type ">ESP-IDF: Build, flash, and start a monitor on your device" into the search bar or press "Ctrl + E, D"


# Post Flash
If the board does not automatically leave bootloader mode:
1. Set IO0 pin to HIGH or remove from GND
2. Press RESET/RST (EN) button. If button not present, manually connect the "EN" pin to GND and disconnect.


# -------------------------------------------------------------BLE Setup --------------------------------------------------------------------------------
1. Once cloned all files, add "ble.c" to CmakeLists and remove any BT_SPP related code
2. Open menuconfig, disable Bluetooth Classic under bluedroid, enable BLE, and enable BLE ONLY in controller options
3. Reconfigure, build, and pray

# -------------------------------------------------------------Modes/Services Desction-------------------------------------------------------------------------------

#Integrated/Supported Services/Modes 
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

By pressing the live data button on the display it will brinig you to the live data screen which will pull data off the ECU/Trainer and update in real time.
 To do this the display first request a bitmask of all available Parameter Identifications(PIDs) from the gateway which will then request the bitmask from the ECU(electronic control unit) using 
 certaint PIDs(0x00, 0x20, 0x40, ...). PIDs are part of service 1 that request certain info from the ECUs. The bitmask is grabed by requesting 32 or 0x20 pids at a time, 
 the full bitmask of availble pids is a uint8_t [7][4] array (224 pids total). The total amount of 1's in this bitmask is the total number of 
 available PIDs(full list of pids at wiki in sources section below). There is always a checksum sent from the gateway to the display for error checking. 

#Mode 03 (stored DTCs), 07 (Pending DTCs), and 0A (Permanent DTCs)

These modes all work in the same way. When the user is on the read codes screen after pressing the read codes button they can press any one of three buttons to grab codes. 
When a button is pressed the appropriate request will be sent through uart with respect to the UART_COMMS_t. Note that the bytes for commands are packed, looking like 1dddd110 
with d being data(command) and the padding for consistency is at the begging and end. No error checking is used other than built in UART parody which all UART uses in this project.  

#Mode 04

Mode 4 is only available on the read codes page and is activated by pressing the clear codes button when codes are being dispalyed. 
Pressing the button sends a UART packed byte with the reset command in UART_COMM_t to the gateway which resets the DTCs on the ECU/trainer. 


# -------------------------------------------------------------CAN Communication Outline-------------------------------------------------------------------------------
CAN frames are used made of up of 8 sections each with a number of bytes 1-11. The only parts that change based on request during communication is the ID and the data section.
The data section is always held at 8 bytes in length with padding bytes added if needed. The ID for the requester is between 0x7DF - 0x7E0. 0x7DF is a request that will be
checked by all ECUs on the network while all the others will request to a specific ECU(0x7E0 is for the ECM). The data will change based on the service requested see below.

# Service 0x01 Request 
Service 1 request will use the 0x7DF ID to ping all ECUs. The 0th data byte will be a 2 representing the number of data bytes excluding itself. The 1st data byte
will represent the requested mode/service(mode byte which is 0x01 for mode 1). The next byte is the Parameter Identifications(PID) requested.

Data Section:
Byte 0: Number bytes (0x02)
Byte 1: Mode byte (0x01)      
Byte 2: PID (0x00 - 0xC8)
Byte 3: Padding (0x00 or simulare)
Byte 4: Padding (0x00 or simulare)  
Byte 5: Padding (0x00 or simulare)       
Byte 6: Padding (0x00 or simulare)   
Byte 7: Padding (0x00 or simulare)   

#Service 0x01 Response
Response to service 1 will come in either of 2 forms. Single frame response( >= 6 data bytes) or multiframe which can be up to 100 bytes or more.
Single frames will have the total number of bytes(0-7) in the 0th byte, mode response in the 1st byte, then repeat PID, and the rest will be data or padding depending on amount 
of returned bytes per PID. A multiframe will have the 0th byte as 0x10 to represent that it is the first frame of a multi frame message. The 1st byte will be the total 
number of data bytes for all frames + mode byte + PID echo. The 2nd byte will be the mode byte followed by the PID echoed and the rest as data. This is then followed by CF
which has the first byte as 0x2N (N being the number consecutive frame it is) followed by data/padding. After the first frame of a multiframe the next frame won't be sent 
until the requester sends a flow control frame which has the 0th byte as 0x30, 1st byte as the number of CF able to be sent before another frame should be sent, the 2nd 
byte should be the separation time(0 = ASAP). 

#Data Sections

Single Frame(SF):
Byte 0: Number bytes (0x01)
Byte 1: Mode byte (0x03, 0x04, 0x07, or 0x0A)      
Byte 2: PID ECHO
Byte 3: Data
Byte 4: Data 
Byte 5: Data       
Byte 6: Data  
Byte 7: Data 

First Frame (FF):
Byte 0: 0x10
Byte 1: Number bytes (0x01)      
Byte 2: Mode byte (0x03, 0x04, 0x07, or 0x0A)
Byte 3: PID ECHO
Byte 4: Data
Byte 5: Data      
Byte 6: Data
Byte 7: Data 

Flow Control Frame(FC):
Byte 0: 0x30
Byte 1: block size (0x00 = no FC after first)      
Byte 2: Sepatation time (0x00 = ASAP)
Byte 3: Padding (0x00 or simulare)
Byte 4: Padding (0x00 or simulare)  
Byte 5: Padding (0x00 or simulare)       
Byte 6: Padding (0x00 or simulare)   
Byte 7: Padding (0x00 or simulare) 

Consecutive Frame(CF):
Byte 0: 0x2N (N = number consecutive frame)
Byte 1: Data      
Byte 2: Data
Byte 3: Data
Byte 4: Data  
Byte 5: Data      
Byte 6: Data   
Byte 7: Data 

#Service 0x03, 0x04, 0x07, and 0x0A Request  
These services are all related to DTCs and the request works in a simulare way. With the 0th byte being the number of bytes (1) and the 1st bytes being the request.
The rest is padding.

Data Section:
Byte 0: Number bytes (0x01)
Byte 1: Mode byte (0x03, 0x04, 0x07, or 0x0A)      
Byte 2: Padding (0x00 or simulare)
Byte 3: Padding (0x00 or simulare)
Byte 4: Padding (0x00 or simulare)  
Byte 5: Padding (0x00 or simulare)       
Byte 6: Padding (0x00 or simulare)   
Byte 7: Padding (0x00 or simulare)   

#Service 0x03, 0x04, 0x07, and 0x0A Response
The response to the DTC request 0x03, 0x04, and 0x07 are the same as service 0x01 but without the PID echo and DTCs coming in two bytes cannot be split between frames. 
This means that there will be more padding. The response for 0x0A is shown below. A bad response will have one of two different mode bytes. The first is a good response
meaning all codes were cleared (0x44). The second being 0x7F which represents a bad response, meaning the codes were not cleared. 

#Negative response
A negative response always has 0x7F as the mode byte and looks like the following.
Data Section:
Byte 0: 0x03        ; Number of data bytes (3)
Byte 1: 0x7F        ; Negative response indicator
Byte 2: 0x01        ; Echo of the requested mode (Mode 1)
Byte 3: 0x11        ; NRC: Service not supported code
Byte 4: 0x00        ; Padding
Byte 5: 0x00        ; Padding
Byte 6: 0x00        ; Padding
Byte 7: 0x00        ; Padding

 

#Sources
1. https://en.wikipedia.org/wiki/OBD-II_PIDs