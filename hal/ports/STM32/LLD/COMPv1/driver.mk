ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_COMP TRUE,$(HALCONF)),)
PLATFORMSRC += $(COMMUNITY)/hal/ports/STM32/LLD/COMPv1/hal_comp_lld.c
endif
else
PLATFORMSRC += $(COMMUNITY)/hal/ports/STM32/LLD/COMPv1/hal_comp_lld.c
endif

PLATFORMINC += $(COMMUNITY)/hal/ports/STM32/LLD/COMPv1

