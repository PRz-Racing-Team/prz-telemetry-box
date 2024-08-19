################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/circular_buffers/can_frames_cb.c \
../Core/circular_buffers/generic_circular_buffer.c 

OBJS += \
./Core/circular_buffers/can_frames_cb.o \
./Core/circular_buffers/generic_circular_buffer.o 

C_DEPS += \
./Core/circular_buffers/can_frames_cb.d \
./Core/circular_buffers/generic_circular_buffer.d 


# Each subdirectory must supply rules for building sources it contributes
Core/circular_buffers/%.o Core/circular_buffers/%.su Core/circular_buffers/%.cyclo: ../Core/circular_buffers/%.c Core/circular_buffers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-circular_buffers

clean-Core-2f-circular_buffers:
	-$(RM) ./Core/circular_buffers/can_frames_cb.cyclo ./Core/circular_buffers/can_frames_cb.d ./Core/circular_buffers/can_frames_cb.o ./Core/circular_buffers/can_frames_cb.su ./Core/circular_buffers/generic_circular_buffer.cyclo ./Core/circular_buffers/generic_circular_buffer.d ./Core/circular_buffers/generic_circular_buffer.o ./Core/circular_buffers/generic_circular_buffer.su

.PHONY: clean-Core-2f-circular_buffers

