################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Aufgabe\ 2\ v2/mailbox.cpp 

OBJS += \
./Aufgabe\ 2\ v2/mailbox.o 

CPP_DEPS += \
./Aufgabe\ 2\ v2/mailbox.d 


# Each subdirectory must supply rules for building sources it contributes
Aufgabe\ 2\ v2/mailbox.o: ../Aufgabe\ 2\ v2/mailbox.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"Aufgabe 2 v2/mailbox.d" -MT"Aufgabe\ 2\ v2/mailbox.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


