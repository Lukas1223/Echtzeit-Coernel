################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: art2-v2

dependents:
	-cd /home/mrt/Desktop/Workspace_Echtzeitsimulation_neu/Echtzeit-Coernel/diagramm/Debug && $(MAKE) clean all

# Tool invocations
art2-v2: $(OBJS) $(USER_OBJS) /home/mrt/Desktop/Workspace_Echtzeitsimulation_neu/Echtzeit-Coernel/diagramm/Debug/diagramm
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L"/home/mrt/Desktop/Workspace_Echtzeitsimulation_neu/Echtzeit-Coernel/diagramm" -o "art2-v2" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(C_UPPER_DEPS)$(CXX_DEPS)$(OBJS)$(CPP_DEPS)$(C_DEPS) art2-v2
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/mrt/Desktop/Workspace_Echtzeitsimulation_neu/Echtzeit-Coernel/diagramm/Debug/diagramm:

-include ../makefile.targets
