################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/rtos_threads/ext_led_blink_thread.c \
../Core/Src/rtos_threads/measurements_thread.c \
../Core/Src/rtos_threads/rtos_hearbeat_logger_thread.c 

OBJS += \
./Core/Src/rtos_threads/ext_led_blink_thread.o \
./Core/Src/rtos_threads/measurements_thread.o \
./Core/Src/rtos_threads/rtos_hearbeat_logger_thread.o 

C_DEPS += \
./Core/Src/rtos_threads/ext_led_blink_thread.d \
./Core/Src/rtos_threads/measurements_thread.d \
./Core/Src/rtos_threads/rtos_hearbeat_logger_thread.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/rtos_threads/ext_led_blink_thread.o: ../Core/Src/rtos_threads/ext_led_blink_thread.c Core/Src/rtos_threads/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../Core/Inc/ -I../Drivers/CMSIS/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc/ -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include/ -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/Src/rtos_threads/ext_led_blink_thread.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/rtos_threads/measurements_thread.o: ../Core/Src/rtos_threads/measurements_thread.c Core/Src/rtos_threads/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../Core/Inc/ -I../Drivers/CMSIS/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc/ -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include/ -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/Src/rtos_threads/measurements_thread.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/rtos_threads/rtos_hearbeat_logger_thread.o: ../Core/Src/rtos_threads/rtos_hearbeat_logger_thread.c Core/Src/rtos_threads/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../Core/Inc/ -I../Drivers/CMSIS/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc/ -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include/ -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/Src/rtos_threads/rtos_hearbeat_logger_thread.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

