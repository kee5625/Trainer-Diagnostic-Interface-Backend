################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/ESP-IDF/Gatway_updates/UART_Display_3/Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_bus.c \
C:/ESP-IDF/Gatway_updates/UART_Display_3/Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_qspi.c \
C:/ESP-IDF/Gatway_updates/UART_Display_3/Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_sdram.c \
C:/ESP-IDF/Gatway_updates/UART_Display_3/Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_ts.c 

C_DEPS += \
./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_bus.d \
./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_qspi.d \
./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_sdram.d \
./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_ts.d 

OBJS += \
./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_bus.o \
./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_qspi.o \
./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_sdram.o \
./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_ts.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_bus.o: C:/ESP-IDF/Gatway_updates/UART_Display_3/Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_bus.c Drivers/BSP/STM32H750B-DK/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DSTM32H750xx -DUSE_HAL_DRIVER -DUSE_BPP=16 -DVECT_TAB_QSPI -DDEBUG -DUSE_PWR_LDO_SUPPLY -c -I../../Core/Inc -I../../Drivers/CMSIS/Include -I../../TouchGFX/generated/gui_generated/include -I../../TouchGFX/target -I../../TouchGFX/App -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/ST/touchgfx/framework/include -I../../TouchGFX/target/generated -I../../Drivers/BSP/STM32H750B-DK -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/BSP/Components/Common -I../../TouchGFX/gui/include -I../../TouchGFX/generated/texts/include -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../TouchGFX/generated/images/include -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../TouchGFX/generated/fonts/include -I../../LIBJPEG/App -I../../LIBJPEG/Target -I../../Utilities/JPEG -I../../Middlewares/Third_Party/LibJPEG/include -I../../TouchGFX/generated/videos/include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_qspi.o: C:/ESP-IDF/Gatway_updates/UART_Display_3/Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_qspi.c Drivers/BSP/STM32H750B-DK/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DSTM32H750xx -DUSE_HAL_DRIVER -DUSE_BPP=16 -DVECT_TAB_QSPI -DDEBUG -DUSE_PWR_LDO_SUPPLY -c -I../../Core/Inc -I../../Drivers/CMSIS/Include -I../../TouchGFX/generated/gui_generated/include -I../../TouchGFX/target -I../../TouchGFX/App -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/ST/touchgfx/framework/include -I../../TouchGFX/target/generated -I../../Drivers/BSP/STM32H750B-DK -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/BSP/Components/Common -I../../TouchGFX/gui/include -I../../TouchGFX/generated/texts/include -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../TouchGFX/generated/images/include -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../TouchGFX/generated/fonts/include -I../../LIBJPEG/App -I../../LIBJPEG/Target -I../../Utilities/JPEG -I../../Middlewares/Third_Party/LibJPEG/include -I../../TouchGFX/generated/videos/include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_sdram.o: C:/ESP-IDF/Gatway_updates/UART_Display_3/Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_sdram.c Drivers/BSP/STM32H750B-DK/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DSTM32H750xx -DUSE_HAL_DRIVER -DUSE_BPP=16 -DVECT_TAB_QSPI -DDEBUG -DUSE_PWR_LDO_SUPPLY -c -I../../Core/Inc -I../../Drivers/CMSIS/Include -I../../TouchGFX/generated/gui_generated/include -I../../TouchGFX/target -I../../TouchGFX/App -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/ST/touchgfx/framework/include -I../../TouchGFX/target/generated -I../../Drivers/BSP/STM32H750B-DK -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/BSP/Components/Common -I../../TouchGFX/gui/include -I../../TouchGFX/generated/texts/include -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../TouchGFX/generated/images/include -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../TouchGFX/generated/fonts/include -I../../LIBJPEG/App -I../../LIBJPEG/Target -I../../Utilities/JPEG -I../../Middlewares/Third_Party/LibJPEG/include -I../../TouchGFX/generated/videos/include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_ts.o: C:/ESP-IDF/Gatway_updates/UART_Display_3/Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_ts.c Drivers/BSP/STM32H750B-DK/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DSTM32H750xx -DUSE_HAL_DRIVER -DUSE_BPP=16 -DVECT_TAB_QSPI -DDEBUG -DUSE_PWR_LDO_SUPPLY -c -I../../Core/Inc -I../../Drivers/CMSIS/Include -I../../TouchGFX/generated/gui_generated/include -I../../TouchGFX/target -I../../TouchGFX/App -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/ST/touchgfx/framework/include -I../../TouchGFX/target/generated -I../../Drivers/BSP/STM32H750B-DK -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/BSP/Components/Common -I../../TouchGFX/gui/include -I../../TouchGFX/generated/texts/include -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../TouchGFX/generated/images/include -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../TouchGFX/generated/fonts/include -I../../LIBJPEG/App -I../../LIBJPEG/Target -I../../Utilities/JPEG -I../../Middlewares/Third_Party/LibJPEG/include -I../../TouchGFX/generated/videos/include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-STM32H750B-2d-DK

clean-Drivers-2f-BSP-2f-STM32H750B-2d-DK:
	-$(RM) ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_bus.cyclo ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_bus.d ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_bus.o ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_bus.su ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_qspi.cyclo ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_qspi.d ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_qspi.o ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_qspi.su ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_sdram.cyclo ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_sdram.d ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_sdram.o ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_sdram.su ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_ts.cyclo ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_ts.d ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_ts.o ./Drivers/BSP/STM32H750B-DK/stm32h750b_discovery_ts.su

.PHONY: clean-Drivers-2f-BSP-2f-STM32H750B-2d-DK

