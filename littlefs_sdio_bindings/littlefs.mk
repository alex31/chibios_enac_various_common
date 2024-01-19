# LittleFS files.
LITTLEFS_SD_SRC = $(VARIOUS)/littlefs_sdio_bindings/lfs_sd_hal.c \
                  $(VARIOUS)/sdio.c \
		  $(RELATIVE)/../littlefs/lfs.c \
		  $(RELATIVE)/../littlefs/lfs_util.c

LITTLEFS_SD_INC = $(VARIOUS)/littlefs_sdio_bindings \
              $(RELATIVE)/../littlefs

DDEFS      += -DLFS_THREADSAFE=1 -DLFS_NO_DEBUG=0 -DLFS_CONFIG=lfs_config.h

# Shared variables
ALLCSRC += $(LITTLEFS_SD_SRC)
ALLINC  += $(LITTLEFS_SD_INC)
