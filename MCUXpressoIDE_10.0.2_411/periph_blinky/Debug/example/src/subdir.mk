################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../example/src/cr_startup_lpc15xx.c \
../example/src/sysinit.c \
../example/src/systick.c 

OBJS += \
./example/src/cr_startup_lpc15xx.o \
./example/src/sysinit.o \
./example/src/systick.o 

C_DEPS += \
./example/src/cr_startup_lpc15xx.d \
./example/src/sysinit.d \
./example/src/systick.d 


# Each subdirectory must supply rules for building sources it contributes
example/src/%.o: ../example/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -D__USE_LPCOPEN -D__REDLIB__ -DCORE_M3 -I"D:\MCUXpressoIDE_10.0.2_411\lpc_chip_15xx\inc" -I"D:\MCUXpressoIDE_10.0.2_411\lpc_board_nxp_lpcxpresso_1549\inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


