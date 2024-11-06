################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../thirdparty/FreeRTOS/Source/croutine.c \
../thirdparty/FreeRTOS/Source/event_groups.c \
../thirdparty/FreeRTOS/Source/list.c \
../thirdparty/FreeRTOS/Source/queue.c \
../thirdparty/FreeRTOS/Source/stream_buffer.c \
../thirdparty/FreeRTOS/Source/tasks.c \
../thirdparty/FreeRTOS/Source/timers.c 

OBJS += \
./thirdparty/FreeRTOS/Source/croutine.o \
./thirdparty/FreeRTOS/Source/event_groups.o \
./thirdparty/FreeRTOS/Source/list.o \
./thirdparty/FreeRTOS/Source/queue.o \
./thirdparty/FreeRTOS/Source/stream_buffer.o \
./thirdparty/FreeRTOS/Source/tasks.o \
./thirdparty/FreeRTOS/Source/timers.o 

C_DEPS += \
./thirdparty/FreeRTOS/Source/croutine.d \
./thirdparty/FreeRTOS/Source/event_groups.d \
./thirdparty/FreeRTOS/Source/list.d \
./thirdparty/FreeRTOS/Source/queue.d \
./thirdparty/FreeRTOS/Source/stream_buffer.d \
./thirdparty/FreeRTOS/Source/tasks.d \
./thirdparty/FreeRTOS/Source/timers.d 


# Each subdirectory must supply rules for building sources it contributes
thirdparty/FreeRTOS/Source/%.o thirdparty/FreeRTOS/Source/%.su thirdparty/FreeRTOS/Source/%.cyclo: ../thirdparty/FreeRTOS/Source/%.c thirdparty/FreeRTOS/Source/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I"C:/Users/harii/Desktop/testing2/01Tasks/thirdparty/FreeRTOS/Source/portable" -I"C:/Users/harii/Desktop/testing2/01Tasks/thirdparty/FreeRTOS/Source/include" -I"C:/Users/harii/Desktop/testing2/01Tasks/thirdparty/FreeRTOS" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-thirdparty-2f-FreeRTOS-2f-Source

clean-thirdparty-2f-FreeRTOS-2f-Source:
	-$(RM) ./thirdparty/FreeRTOS/Source/croutine.cyclo ./thirdparty/FreeRTOS/Source/croutine.d ./thirdparty/FreeRTOS/Source/croutine.o ./thirdparty/FreeRTOS/Source/croutine.su ./thirdparty/FreeRTOS/Source/event_groups.cyclo ./thirdparty/FreeRTOS/Source/event_groups.d ./thirdparty/FreeRTOS/Source/event_groups.o ./thirdparty/FreeRTOS/Source/event_groups.su ./thirdparty/FreeRTOS/Source/list.cyclo ./thirdparty/FreeRTOS/Source/list.d ./thirdparty/FreeRTOS/Source/list.o ./thirdparty/FreeRTOS/Source/list.su ./thirdparty/FreeRTOS/Source/queue.cyclo ./thirdparty/FreeRTOS/Source/queue.d ./thirdparty/FreeRTOS/Source/queue.o ./thirdparty/FreeRTOS/Source/queue.su ./thirdparty/FreeRTOS/Source/stream_buffer.cyclo ./thirdparty/FreeRTOS/Source/stream_buffer.d ./thirdparty/FreeRTOS/Source/stream_buffer.o ./thirdparty/FreeRTOS/Source/stream_buffer.su ./thirdparty/FreeRTOS/Source/tasks.cyclo ./thirdparty/FreeRTOS/Source/tasks.d ./thirdparty/FreeRTOS/Source/tasks.o ./thirdparty/FreeRTOS/Source/tasks.su ./thirdparty/FreeRTOS/Source/timers.cyclo ./thirdparty/FreeRTOS/Source/timers.d ./thirdparty/FreeRTOS/Source/timers.o ./thirdparty/FreeRTOS/Source/timers.su

.PHONY: clean-thirdparty-2f-FreeRTOS-2f-Source

