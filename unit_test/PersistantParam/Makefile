##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Library paths
ETL_LIB      := ../../../../../etl/include
FROZEN_LIB   := ../../../../../frozen/include
PP_LIB       := ../../UAVCAN
RELATIVE     := ../../../../../UAVCAN

# Build directory (all output files go here)
BUILD_DIR    := build

# Compiler
CXX          := g++-14
CC           := gcc-14  # Use `CC` for C files

SANITIZE     := -fsanitize=address,pointer-subtract,pointer-compare,leak,undefined
# Compiler Flags
CXXFLAGS     := -Wall -Wextra -std=c++26 -O0 -g3 $(SANITIZE)
CFLAGS       := -Wall -Wextra -std=c23   -O0 -g3 $(SANITIZE)
# CXXFLAGS     := -Wall -Wextra -std=c++26 -Og -g3
# CFLAGS       := -Wall -Wextra -std=c23   -Og -g3

# DSDL Paths
DSDL         := $(RELATIVE)/DSDL
DSDLC        := $(RELATIVE)/DSDLC
DSDL_ROOTS_PART := $(sort $(dir $(wildcard $(DSDL)/*/)))
DSDL_ROOTS   := $(filter-out ../../../../DSDL/tests/,$(DSDL_ROOTS_PART))
DSDLC_CSRC   := $(wildcard $(DSDLC)/src/*.c)

# ----------------------------------------------------------------------------
# 1) List your source files by *filename only*, without directory prefix.
#    We'll tell make where to find them via vpath.
# ----------------------------------------------------------------------------

# C++ sources (filenames only!)
CPPSRC := \
  persistantParam.cpp \
  persistantStorage.cpp \
  testPP.cpp

# C sources (filenames only!)
CSRC := \
  canard.c \
  $(notdir $(DSDLC_CSRC))   # e.g. if DSDLC_CSRC has "a/b/xyz.c", we extract just "xyz.c"

# ----------------------------------------------------------------------------
# 2) Use vpath so that make can find those source files in the right folders.
#    We add search paths for *.cpp and *.c.
# ----------------------------------------------------------------------------
vpath %.cpp ../../UAVCAN .
vpath %.c   $(RELATIVE)/libcanard $(DSDLC)/src

# ----------------------------------------------------------------------------
# 3) Generate the list of object files. Now each .cpp or .c from above
#    simply becomes build/filename.o
# ----------------------------------------------------------------------------
OBJ := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(CPPSRC)) \
       $(patsubst %.c,$(BUILD_DIR)/%.o,$(CSRC))

# We'll also generate a list of .d files for dependency tracking
DEP := $(OBJ:.o=.d)

# Executable Name
TARGET := $(BUILD_DIR)/testPP

default: all


.PHONY: all
all: $(TARGET)

# ----------------------------------------------------------------------------
# Ensure `build/` directory exists
# ----------------------------------------------------------------------------
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)



# ----------------------------------------------------------------------------
# Default rule: Build the executable from all object files
# ----------------------------------------------------------------------------
$(TARGET): $(OBJ) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# ----------------------------------------------------------------------------
# Pattern rule to compile .cpp -> .o with dependency tracking
# Because of vpath, make will automatically find the matching .cpp
# anywhere in the search path (UAVCAN or current dir).
# ----------------------------------------------------------------------------
$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	@echo "Compiling C++ source: $<"
	$(CXX) $(CXXFLAGS) \
		-I$(ETL_LIB) \
		-I$(FROZEN_LIB) \
		-I$(PP_LIB) \
		-I. \
		-I$(RELATIVE)/libcanard \
		-I$(DSDLC)/include \
		-MMD -MP -c $< -o $@

# ----------------------------------------------------------------------------
# Pattern rule to compile .c -> .o with dependency tracking
# Because of vpath, make will find .c files in libcanard or DSDLC/src.
# ----------------------------------------------------------------------------
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "Compiling C source: $<"
	$(CC) $(CFLAGS) \
		-I$(RELATIVE)/libcanard \
		-I$(DSDLC)/include \
		-MMD -MP -c $< -o $@

# ----------------------------------------------------------------------------
# Include the auto-generated dependency files if they exist
# ----------------------------------------------------------------------------
-include $(DEP)

# ----------------------------------------------------------------------------
# Clean rule
# ----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
