################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/lut/crc15_lut.c 

OBJS += \
./Core/Src/lut/crc15_lut.o 

C_DEPS += \
./Core/Src/lut/crc15_lut.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/lut/crc15_lut.o: ../Core/Src/lut/crc15_lut.c Core/Src/lut/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../Core/Inc/ -I../Drivers/CMSIS/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc/ -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include/ -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Core/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/Src/lut/crc15_lut.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

