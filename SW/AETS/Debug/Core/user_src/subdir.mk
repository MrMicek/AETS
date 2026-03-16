################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/user_src/app_menu.c \
../Core/user_src/app_params.c \
../Core/user_src/app_sm.c \
../Core/user_src/buzzer.c \
../Core/user_src/canfd.c \
../Core/user_src/commands.c \
../Core/user_src/comuser.c \
../Core/user_src/current.c \
../Core/user_src/display.c \
../Core/user_src/eeprom.c \
../Core/user_src/encoder.c \
../Core/user_src/encoder_menu.c \
../Core/user_src/error.c \
../Core/user_src/io_control.c \
../Core/user_src/kvstore.c \
../Core/user_src/menu.c \
../Core/user_src/mosfet.c \
../Core/user_src/mux.c \
../Core/user_src/power.c \
../Core/user_src/profile_store.c \
../Core/user_src/relay.c \
../Core/user_src/relay_counter.c \
../Core/user_src/relay_health_store.c \
../Core/user_src/telemetry.c \
../Core/user_src/test_seq.c \
../Core/user_src/trigger.c \
../Core/user_src/utility.c 

OBJS += \
./Core/user_src/app_menu.o \
./Core/user_src/app_params.o \
./Core/user_src/app_sm.o \
./Core/user_src/buzzer.o \
./Core/user_src/canfd.o \
./Core/user_src/commands.o \
./Core/user_src/comuser.o \
./Core/user_src/current.o \
./Core/user_src/display.o \
./Core/user_src/eeprom.o \
./Core/user_src/encoder.o \
./Core/user_src/encoder_menu.o \
./Core/user_src/error.o \
./Core/user_src/io_control.o \
./Core/user_src/kvstore.o \
./Core/user_src/menu.o \
./Core/user_src/mosfet.o \
./Core/user_src/mux.o \
./Core/user_src/power.o \
./Core/user_src/profile_store.o \
./Core/user_src/relay.o \
./Core/user_src/relay_counter.o \
./Core/user_src/relay_health_store.o \
./Core/user_src/telemetry.o \
./Core/user_src/test_seq.o \
./Core/user_src/trigger.o \
./Core/user_src/utility.o 

C_DEPS += \
./Core/user_src/app_menu.d \
./Core/user_src/app_params.d \
./Core/user_src/app_sm.d \
./Core/user_src/buzzer.d \
./Core/user_src/canfd.d \
./Core/user_src/commands.d \
./Core/user_src/comuser.d \
./Core/user_src/current.d \
./Core/user_src/display.d \
./Core/user_src/eeprom.d \
./Core/user_src/encoder.d \
./Core/user_src/encoder_menu.d \
./Core/user_src/error.d \
./Core/user_src/io_control.d \
./Core/user_src/kvstore.d \
./Core/user_src/menu.d \
./Core/user_src/mosfet.d \
./Core/user_src/mux.d \
./Core/user_src/power.d \
./Core/user_src/profile_store.d \
./Core/user_src/relay.d \
./Core/user_src/relay_counter.d \
./Core/user_src/relay_health_store.d \
./Core/user_src/telemetry.d \
./Core/user_src/test_seq.d \
./Core/user_src/trigger.d \
./Core/user_src/utility.d 


# Each subdirectory must supply rules for building sources it contributes
Core/user_src/%.o Core/user_src/%.su Core/user_src/%.cyclo: ../Core/user_src/%.c Core/user_src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -DSTM32_THREAD_SAFE_STRATEGY=2 -c -I../Core/Inc -I../USB_Device/App -I../USB_Device/Target -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -I"C:/Users/uiv10467/OneDrive - Vitesco Technologies/Plocha/AETS/AETS/SW/AETS/Core/user_inc" -I"C:/Users/uiv10467/OneDrive - Vitesco Technologies/Plocha/AETS/AETS/SW/AETS/Core/user_src" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-user_src

clean-Core-2f-user_src:
	-$(RM) ./Core/user_src/app_menu.cyclo ./Core/user_src/app_menu.d ./Core/user_src/app_menu.o ./Core/user_src/app_menu.su ./Core/user_src/app_params.cyclo ./Core/user_src/app_params.d ./Core/user_src/app_params.o ./Core/user_src/app_params.su ./Core/user_src/app_sm.cyclo ./Core/user_src/app_sm.d ./Core/user_src/app_sm.o ./Core/user_src/app_sm.su ./Core/user_src/buzzer.cyclo ./Core/user_src/buzzer.d ./Core/user_src/buzzer.o ./Core/user_src/buzzer.su ./Core/user_src/canfd.cyclo ./Core/user_src/canfd.d ./Core/user_src/canfd.o ./Core/user_src/canfd.su ./Core/user_src/commands.cyclo ./Core/user_src/commands.d ./Core/user_src/commands.o ./Core/user_src/commands.su ./Core/user_src/comuser.cyclo ./Core/user_src/comuser.d ./Core/user_src/comuser.o ./Core/user_src/comuser.su ./Core/user_src/current.cyclo ./Core/user_src/current.d ./Core/user_src/current.o ./Core/user_src/current.su ./Core/user_src/display.cyclo ./Core/user_src/display.d ./Core/user_src/display.o ./Core/user_src/display.su ./Core/user_src/eeprom.cyclo ./Core/user_src/eeprom.d ./Core/user_src/eeprom.o ./Core/user_src/eeprom.su ./Core/user_src/encoder.cyclo ./Core/user_src/encoder.d ./Core/user_src/encoder.o ./Core/user_src/encoder.su ./Core/user_src/encoder_menu.cyclo ./Core/user_src/encoder_menu.d ./Core/user_src/encoder_menu.o ./Core/user_src/encoder_menu.su ./Core/user_src/error.cyclo ./Core/user_src/error.d ./Core/user_src/error.o ./Core/user_src/error.su ./Core/user_src/io_control.cyclo ./Core/user_src/io_control.d ./Core/user_src/io_control.o ./Core/user_src/io_control.su ./Core/user_src/kvstore.cyclo ./Core/user_src/kvstore.d ./Core/user_src/kvstore.o ./Core/user_src/kvstore.su ./Core/user_src/menu.cyclo ./Core/user_src/menu.d ./Core/user_src/menu.o ./Core/user_src/menu.su ./Core/user_src/mosfet.cyclo ./Core/user_src/mosfet.d ./Core/user_src/mosfet.o ./Core/user_src/mosfet.su ./Core/user_src/mux.cyclo ./Core/user_src/mux.d ./Core/user_src/mux.o ./Core/user_src/mux.su ./Core/user_src/power.cyclo ./Core/user_src/power.d ./Core/user_src/power.o ./Core/user_src/power.su ./Core/user_src/profile_store.cyclo ./Core/user_src/profile_store.d ./Core/user_src/profile_store.o ./Core/user_src/profile_store.su ./Core/user_src/relay.cyclo ./Core/user_src/relay.d ./Core/user_src/relay.o ./Core/user_src/relay.su ./Core/user_src/relay_counter.cyclo ./Core/user_src/relay_counter.d ./Core/user_src/relay_counter.o ./Core/user_src/relay_counter.su ./Core/user_src/relay_health_store.cyclo ./Core/user_src/relay_health_store.d ./Core/user_src/relay_health_store.o ./Core/user_src/relay_health_store.su ./Core/user_src/telemetry.cyclo ./Core/user_src/telemetry.d ./Core/user_src/telemetry.o ./Core/user_src/telemetry.su ./Core/user_src/test_seq.cyclo ./Core/user_src/test_seq.d ./Core/user_src/test_seq.o ./Core/user_src/test_seq.su ./Core/user_src/trigger.cyclo ./Core/user_src/trigger.d ./Core/user_src/trigger.o ./Core/user_src/trigger.su ./Core/user_src/utility.cyclo ./Core/user_src/utility.d ./Core/user_src/utility.o ./Core/user_src/utility.su

.PHONY: clean-Core-2f-user_src

