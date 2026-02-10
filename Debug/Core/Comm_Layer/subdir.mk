################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Comm_Layer/Comm.c 

OBJS += \
./Core/Comm_Layer/Comm.o 

C_DEPS += \
./Core/Comm_Layer/Comm.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Comm_Layer/%.o Core/Comm_Layer/%.su Core/Comm_Layer/%.cyclo: ../Core/Comm_Layer/%.c Core/Comm_Layer/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I/home/danish/STM32CubeIDE/workspace_1.17.0/DFT_Robot/Core/Comm_Layer -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Comm_Layer

clean-Core-2f-Comm_Layer:
	-$(RM) ./Core/Comm_Layer/Comm.cyclo ./Core/Comm_Layer/Comm.d ./Core/Comm_Layer/Comm.o ./Core/Comm_Layer/Comm.su

.PHONY: clean-Core-2f-Comm_Layer

