# Required platform files.

# Required include directories.
PLATFORMINC += $(COMMUNITY)/hal/ports/STM32/STM32F3xx 

# Drivers compatible with the platform.
include $(COMMUNITY)/hal/ports/STM32/LLD/COMPv1/driver.mk

