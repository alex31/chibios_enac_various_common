# List of all the ChibiOS/HAL files, there is no need to remove the files
# from this list, you can disable parts of the HAL by editing halconf.h.
ifeq ($(USE_SMART_BUILD),yes)
HALCONF := $(strip $(shell cat halconf.h | egrep -e "\#define"))

HALSRC += $(CONTRIB)/hal/src/hal_community.c

ifneq ($(findstring HAL_USE_COMP TRUE,$(HALCONF)),)
HALSRC += $(CONTRIB)/hal/src/hal_comp.c
endif
else
HALSRC += $(CONTRIB)/hal/src/hal_comp.c
endif

# Required include directories
HALINC += $(CONTRIB)/hal/include
