################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/line_collector/line_collector.c 

OBJS += \
./Core/line_collector/line_collector.o 

C_DEPS += \
./Core/line_collector/line_collector.d 


# Each subdirectory must supply rules for building sources it contributes
Core/line_collector/%.o Core/line_collector/%.su Core/line_collector/%.cyclo: ../Core/line_collector/%.c Core/line_collector/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-line_collector

clean-Core-2f-line_collector:
	-$(RM) ./Core/line_collector/line_collector.cyclo ./Core/line_collector/line_collector.d ./Core/line_collector/line_collector.o ./Core/line_collector/line_collector.su

.PHONY: clean-Core-2f-line_collector

