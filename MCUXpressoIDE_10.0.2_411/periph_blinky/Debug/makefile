################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include example/src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: periph_blinky.axf

# Tool invocations
periph_blinky.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"D:\MCUXpressoIDE_10.0.2_411\lpc_chip_15xx\Debug" -L"D:\MCUXpressoIDE_10.0.2_411\lpc_board_nxp_lpcxpresso_1549\Debug" -Xlinker -Map="periph_blinky.map" -Xlinker --gc-sections -mcpu=cortex-m3 -mthumb -T "periph_blinky_Debug.ld" -o "periph_blinky.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) periph_blinky.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "periph_blinky.axf"; # arm-none-eabi-objcopy -v -O binary "periph_blinky.axf" "periph_blinky.bin" ; # checksum -p LPC1549 -d "periph_blinky.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
