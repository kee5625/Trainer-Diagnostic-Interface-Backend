# Introduction
This code is made to be used on an ESP32 chip to act as a gateway between a CAN bus with ECUs/Trainers that follow public CAN standards ISO 11898-1/ ISO 11898-2 and an STM32H750B-DK kit with touch screen display (UART). The gateway follows all public CAN standard requirements while the UART uses byte packed commands from the UART_COMMS_t in UART headers (byte = 1dddd110 d for data/command). See general README.md for more information.



# UART SPECS
Baud Rate: 112500
Data Bits: 8
Parity   : Even
stop bits: 1
flow_ctrl: disabled

# CAN/TWAI SPECS
Speed  : 500k
Filter :  0x7E8 to 0x7EF
CAN MSG-----------
*always 8 data bytes per frame as required by standards (0x00 used as padding)
*ID = 0x7E8 to request to all ECUs and 0x7E8 – 0x7EF

# Improvements
1. Change to byte packing for increased error checking and the ability to deal with incorrect commands or UART code being stuck in the wrong function/work flow.
2. Once display's PIDs_Library file is compelete change MAX_BITMASK_NUM in TWAI_TC.h to 0xC8 to request full list of available PIDs.
    *Might be because the Service queue has a 5 second timeout before main will reset it and it keep trying until it is reset but the UART 0x20 was already received and the TWAI service is hogging the CPU causing the UART to wait on the TWAI to receive the exit condition.
        *limiting the number of retrys in TWAI could fix this
3. For TWAI either change Live_Data_Get() to return ESP_IDF error type or make macros to define return and enhance readability. Returns currently defined in the function description just above it.

# Bugs
1. Reset chain that force quits tasks needs worked on because if done too fast or at the right time as far as requesting the TWAI or UART can get stuck in the wrong mode and request till it hits timeout and resets
    *This one could fix other problems but most likely won't, this is also the most difficult to fix as every function in the Gateway has it's own force quit action taken based on if the reset flag is set to true
    *In some instances the Gateway will get stuck resetting a set number of time until it will reset completely. During this about 5s the Gateway will not be able to interact with UART
2. Code will get stuck requesting the same PIDs over and over again and won't exit when home button pressed on display and exit condition sent. This happens after the live data has been going for a while. It seems to queue up a bunch of actions and then eventually will stop after going through all of them.