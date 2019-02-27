ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_OPAMP TRUE,$(HALCONF)),)
PLATFORMSRC += $(COMMUNITY)/hal/ports/STM32/LLD/OPAMPv2/hal_opamp_lld.c
endif
else
PLATFORMSRC += $(COMMUNITY)/hal/ports/STM32/LLD/OPAMPv2/hal_opamp_lld.c
endif

PLATFORMINC += $(COMMUNITY)/hal/ports/STM32/LLD/OPAMPv2

