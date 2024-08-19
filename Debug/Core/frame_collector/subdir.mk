################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/frame_collector/frame_collector.c 

OBJS += \
./Core/frame_collector/frame_collector.o 

C_DEPS += \
./Core/frame_collector/frame_collector.d 


# Each subdirectory must supply rules for building sources it contributes
Core/frame_collector/%.o Core/frame_collector/%.su Core/frame_collector/%.cyclo: ../Core/frame_collector/%.c Core/frame_collector/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-frame_collector

clean-Core-2f-frame_collector:
	-$(RM) ./Core/frame_collector/frame_collector.cyclo ./Core/frame_collector/frame_collector.d ./Core/frame_collector/frame_collector.o ./Core/frame_collector/frame_collector.su

.PHONY: clean-Core-2f-frame_collector

