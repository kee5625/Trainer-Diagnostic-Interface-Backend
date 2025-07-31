#Introduction
This code is made to be used on an ESP32 chip to act as a gateway between a CAN bus with ECUs/Trainers that follow public CAN standards ISO 11898-1/ ISO 11898-2 and an STM32H750B-DK kit with touch screen display (UART). The gateway follows all public CAN standard requirements while the UART uses byte packed commands from the UART_COMMS_t in UART headers (byte = 1dddd110 d for data/command). 

#UART - TWAI interaction
UART will listen until it receives a command which it will then go and queue up in the corresponding service via Set_TWAI_Serv() for the TWAI/CAN to complete. The rx thread will be blocked by a semaphore until the TWAI is finished. After TWAI is done a function which will carry out the mode/service until an exit condition is met (different per mode/service) will be called. After which the rx thread will return to listening for the next command. TX thread unblocked but controlled by RX thread.


#TWAI - CAN interaction
The TWAI/CAN on the Gateway will be controlled by a service thread which is given commands by UART. The service thread queues up actions for the transmit function which will request data from ECUs/ECMs. The rx thread is always listening and filtering out IDs that are not ECUs. This means that it is constantly running which could be changed to have it only run after a service has been set, that way external tools wounldn't cause accidental code to execute on the gateway.


#UART SPECS
Baud Rate: 112500
Data Bits: 8
Parity   : Even
stop bits: 1
flow_ctrl: disabled

#CAN/TWAI SPECS
Speed  : 500k
Filter : Accept all
CAN MSG-----------
*always 8 data bytes per frame as required by standards (0x00 used as padding)
*ID = 0x7E8 to request 