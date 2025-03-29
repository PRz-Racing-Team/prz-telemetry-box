################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/fancy_timer/stm32-easy-timers/stm32_easy_timers.c \
../Core/fancy_timer/stm32-easy-timers/stm32_easy_timers_errors.c 

OBJS += \
./Core/fancy_timer/stm32-easy-timers/stm32_easy_timers.o \
./Core/fancy_timer/stm32-easy-timers/stm32_easy_timers_errors.o 

C_DEPS += \
./Core/fancy_timer/stm32-easy-timers/stm32_easy_timers.d \
./Core/fancy_timer/stm32-easy-timers/stm32_easy_timers_errors.d 


# Each subdirectory must supply rules for building sources it contributes
Core/fancy_timer/stm32-easy-timers/%.o Core/fancy_timer/stm32-easy-timers/%.su Core/fancy_timer/stm32-easy-timers/%.cyclo: ../Core/fancy_timer/stm32-easy-timers/%.c Core/fancy_timer/stm32-easy-timers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-fancy_timer-2f-stm32-2d-easy-2d-timers

clean-Core-2f-fancy_timer-2f-stm32-2d-easy-2d-timers:
	-$(RM) ./Core/fancy_timer/stm32-easy-timers/stm32_easy_timers.cyclo ./Core/fancy_timer/stm32-easy-timers/stm32_easy_timers.d ./Core/fancy_timer/stm32-easy-timers/stm32_easy_timers.o ./Core/fancy_timer/stm32-easy-timers/stm32_easy_timers.su ./Core/fancy_timer/stm32-easy-timers/stm32_easy_timers_errors.cyclo ./Core/fancy_timer/stm32-easy-timers/stm32_easy_timers_errors.d ./Core/fancy_timer/stm32-easy-timers/stm32_easy_timers_errors.o ./Core/fancy_timer/stm32-easy-timers/stm32_easy_timers_errors.su

.PHONY: clean-Core-2f-fancy_timer-2f-stm32-2d-easy-2d-timers

