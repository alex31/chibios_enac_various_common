# Required platform files.

# Required include directories.
PLATFORMINC += $(CONTRIB)/hal/ports/STM32/STM32F3xx 

# Drivers compatible with the platform.
include $(CONTRIB)/hal/ports/STM32/LLD/COMPv1/driver.mk

