# List of all the ChibiOS/HAL files, there is no need to remove the files
# from this list, you can disable parts of the HAL by editing halconf.h.
ifeq ($(USE_SMART_BUILD),yes)
HALCONF := $(strip $(shell cat $(CONFDIR)/halconf.h | egrep -e "\#define"))

HALSRC += $(COMMUNITY)/hal/src/hal_community.c

ifneq ($(findstring HAL_USE_COMP TRUE,$(HALCONF)),)
HALSRC += $(COMMUNITY)/hal/src/hal_comp.c
endif
ifneq ($(findstring HAL_USE_OPAMP TRUE,$(HALCONF)),)
HALSRC += $(COMMUNITY)/hal/src/hal_opamp.c
endif


else # ifeq ($(USE_SMART_BUILD),yes)
HALSRC += $(COMMUNITY)/hal/src/hal_comp.c
HALSRC += $(COMMUNITY)/hal/src/hal_opamp.c
endif # ifeq ($(USE_SMART_BUILD),yes)



# Required include directories
HALINC += $(COMMUNITY)/hal/include
