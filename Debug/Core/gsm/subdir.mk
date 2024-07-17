################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/gsm/gsm.c 

OBJS += \
./Core/gsm/gsm.o 

C_DEPS += \
./Core/gsm/gsm.d 


# Each subdirectory must supply rules for building sources it contributes
Core/gsm/%.o Core/gsm/%.su Core/gsm/%.cyclo: ../Core/gsm/%.c Core/gsm/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-gsm

clean-Core-2f-gsm:
	-$(RM) ./Core/gsm/gsm.cyclo ./Core/gsm/gsm.d ./Core/gsm/gsm.o ./Core/gsm/gsm.su

.PHONY: clean-Core-2f-gsm

