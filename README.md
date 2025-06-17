# Introduction 
This project aims to create a diagnostic interface for all future trainers. This interface will be able to read trouble codes from trainers, clear trouble codes on trainers, and read live data from trainers. 


# Getting Started
To properly setup the development in your system, follow these steps:
1. If using VS Code, install the "ESP-IDF" Extension.
2. Clone the repository into a local folder.
3. The "ESP-IDF Welcome" page will be automatically prompted by ESP-IDF (If not, type ">ESP-IDF: Configure ESP-IDF Extension" onto the search bar).
4. Select "Configure extension".
5. Choose the "Express" mode (unless a configuration already exists; then select "Use Existing Setup") and follow the steps to configure the extension.


# Build and Flash
(NOTE: Make sure you open the sub-folder with the main.c file in vs code BEFORE building. Trying to build the entire repo will NOT work.)
1. Re-configure ESP-IDF Extension after opening the sub-folder.
2. Type ">ESP-IDF: Build your project" into search bar or press "Ctrl + E, B".
2. To monitor/flash: type ">ESP-IDF: Build, flash, and start a monitor on your device" into the search bar or press "Ctrl + E, D"


# Post Flash
If the board does not automatically leave bootloader mode:
1. Set IO0 pin to HIGH or remove from GND
2. Press RESET/RST (EN) button. If button not present, manually connect the "EN" pin to GND and disconnect.
