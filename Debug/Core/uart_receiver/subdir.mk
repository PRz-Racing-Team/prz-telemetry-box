################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/uart_receiver/uart_receiver.c 

OBJS += \
./Core/uart_receiver/uart_receiver.o 

C_DEPS += \
./Core/uart_receiver/uart_receiver.d 


# Each subdirectory must supply rules for building sources it contributes
Core/uart_receiver/%.o Core/uart_receiver/%.su Core/uart_receiver/%.cyclo: ../Core/uart_receiver/%.c Core/uart_receiver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-uart_receiver

clean-Core-2f-uart_receiver:
	-$(RM) ./Core/uart_receiver/uart_receiver.cyclo ./Core/uart_receiver/uart_receiver.d ./Core/uart_receiver/uart_receiver.o ./Core/uart_receiver/uart_receiver.su

.PHONY: clean-Core-2f-uart_receiver

