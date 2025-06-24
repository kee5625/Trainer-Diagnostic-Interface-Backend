# Introduction 
This project aims to create a diagnostic interface for all future trainers. To do this a gateway will be used to communicate with external displays/devices. The gateway will communicate with the trainer through CAN (see diagram in repo for layout). The functionality of this interface will allow it to display trouble codes, reset trouble codes, and display live data to user.



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
