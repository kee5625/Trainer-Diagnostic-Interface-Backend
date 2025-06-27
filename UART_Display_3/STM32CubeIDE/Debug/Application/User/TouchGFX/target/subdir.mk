################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
C:/TouchGFXProjects/UART_Display_3/TouchGFX/target/CortexMMCUInstrumentation.cpp \
C:/TouchGFXProjects/UART_Display_3/TouchGFX/target/STM32TouchController.cpp \
C:/TouchGFXProjects/UART_Display_3/TouchGFX/target/TouchGFXGPIO.cpp \
C:/TouchGFXProjects/UART_Display_3/TouchGFX/target/TouchGFXHAL.cpp 

OBJS += \
./Application/User/TouchGFX/target/CortexMMCUInstrumentation.o \
./Application/User/TouchGFX/target/STM32TouchController.o \
./Application/User/TouchGFX/target/TouchGFXGPIO.o \
./Application/User/TouchGFX/target/TouchGFXHAL.o 

CPP_DEPS += \
./Application/User/TouchGFX/target/CortexMMCUInstrumentation.d \
./Application/User/TouchGFX/target/STM32TouchController.d \
./Application/User/TouchGFX/target/TouchGFXGPIO.d \
./Application/User/TouchGFX/target/TouchGFXHAL.d 


# Each subdirectory must supply rules for building sources it contributes
Application/User/TouchGFX/target/CortexMMCUInstrumentation.o: C:/TouchGFXProjects/UART_Display_3/TouchGFX/target/CortexMMCUInstrumentation.cpp Application/User/TouchGFX/target/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DSTM32H750xx -DUSE_HAL_DRIVER -DUSE_BPP=16 -DVECT_TAB_QSPI -DDEBUG -DUSE_PWR_LDO_SUPPLY -c -I../../Core/Inc -I../../Drivers/CMSIS/Include -I../../TouchGFX/generated/gui_generated/include -I../../TouchGFX/target -I../../TouchGFX/App -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/ST/touchgfx/framework/include -I../../TouchGFX/target/generated -I../../Drivers/BSP/STM32H750B-DK -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/BSP/Components/Common -I../../TouchGFX/gui/include -I../../TouchGFX/generated/texts/include -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../TouchGFX/generated/images/include -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../TouchGFX/generated/fonts/include -I../../LIBJPEG/App -I../../LIBJPEG/Target -I../../Utilities/JPEG -I../../Middlewares/Third_Party/LibJPEG/include -I../../TouchGFX/generated/videos/include -O3 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -femit-class-debug-always -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/TouchGFX/target/STM32TouchController.o: C:/TouchGFXProjects/UART_Display_3/TouchGFX/target/STM32TouchController.cpp Application/User/TouchGFX/target/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DSTM32H750xx -DUSE_HAL_DRIVER -DUSE_BPP=16 -DVECT_TAB_QSPI -DDEBUG -DUSE_PWR_LDO_SUPPLY -c -I../../Core/Inc -I../../Drivers/CMSIS/Include -I../../TouchGFX/generated/gui_generated/include -I../../TouchGFX/target -I../../TouchGFX/App -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/ST/touchgfx/framework/include -I../../TouchGFX/target/generated -I../../Drivers/BSP/STM32H750B-DK -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/BSP/Components/Common -I../../TouchGFX/gui/include -I../../TouchGFX/generated/texts/include -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../TouchGFX/generated/images/include -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../TouchGFX/generated/fonts/include -I../../LIBJPEG/App -I../../LIBJPEG/Target -I../../Utilities/JPEG -I../../Middlewares/Third_Party/LibJPEG/include -I../../TouchGFX/generated/videos/include -O3 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -femit-class-debug-always -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/TouchGFX/target/TouchGFXGPIO.o: C:/TouchGFXProjects/UART_Display_3/TouchGFX/target/TouchGFXGPIO.cpp Application/User/TouchGFX/target/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DSTM32H750xx -DUSE_HAL_DRIVER -DUSE_BPP=16 -DVECT_TAB_QSPI -DDEBUG -DUSE_PWR_LDO_SUPPLY -c -I../../Core/Inc -I../../Drivers/CMSIS/Include -I../../TouchGFX/generated/gui_generated/include -I../../TouchGFX/target -I../../TouchGFX/App -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/ST/touchgfx/framework/include -I../../TouchGFX/target/generated -I../../Drivers/BSP/STM32H750B-DK -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/BSP/Components/Common -I../../TouchGFX/gui/include -I../../TouchGFX/generated/texts/include -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../TouchGFX/generated/images/include -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../TouchGFX/generated/fonts/include -I../../LIBJPEG/App -I../../LIBJPEG/Target -I../../Utilities/JPEG -I../../Middlewares/Third_Party/LibJPEG/include -I../../TouchGFX/generated/videos/include -O3 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -femit-class-debug-always -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/TouchGFX/target/TouchGFXHAL.o: C:/TouchGFXProjects/UART_Display_3/TouchGFX/target/TouchGFXHAL.cpp Application/User/TouchGFX/target/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DSTM32H750xx -DUSE_HAL_DRIVER -DUSE_BPP=16 -DVECT_TAB_QSPI -DDEBUG -DUSE_PWR_LDO_SUPPLY -c -I../../Core/Inc -I../../Drivers/CMSIS/Include -I../../TouchGFX/generated/gui_generated/include -I../../TouchGFX/target -I../../TouchGFX/App -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/ST/touchgfx/framework/include -I../../TouchGFX/target/generated -I../../Drivers/BSP/STM32H750B-DK -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/BSP/Components/Common -I../../TouchGFX/gui/include -I../../TouchGFX/generated/texts/include -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../TouchGFX/generated/images/include -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../TouchGFX/generated/fonts/include -I../../LIBJPEG/App -I../../LIBJPEG/Target -I../../Utilities/JPEG -I../../Middlewares/Third_Party/LibJPEG/include -I../../TouchGFX/generated/videos/include -O3 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -femit-class-debug-always -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Application-2f-User-2f-TouchGFX-2f-target

clean-Application-2f-User-2f-TouchGFX-2f-target:
	-$(RM) ./Application/User/TouchGFX/target/CortexMMCUInstrumentation.cyclo ./Application/User/TouchGFX/target/CortexMMCUInstrumentation.d ./Application/User/TouchGFX/target/CortexMMCUInstrumentation.o ./Application/User/TouchGFX/target/CortexMMCUInstrumentation.su ./Application/User/TouchGFX/target/STM32TouchController.cyclo ./Application/User/TouchGFX/target/STM32TouchController.d ./Application/User/TouchGFX/target/STM32TouchController.o ./Application/User/TouchGFX/target/STM32TouchController.su ./Application/User/TouchGFX/target/TouchGFXGPIO.cyclo ./Application/User/TouchGFX/target/TouchGFXGPIO.d ./Application/User/TouchGFX/target/TouchGFXGPIO.o ./Application/User/TouchGFX/target/TouchGFXGPIO.su ./Application/User/TouchGFX/target/TouchGFXHAL.cyclo ./Application/User/TouchGFX/target/TouchGFXHAL.d ./Application/User/TouchGFX/target/TouchGFXHAL.o ./Application/User/TouchGFX/target/TouchGFXHAL.su

.PHONY: clean-Application-2f-User-2f-TouchGFX-2f-target

