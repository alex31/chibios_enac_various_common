##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
# -Wdouble-promotion -fno-omit-frame-pointer


ETL_LIB = ../../../../../etl/include
FROZEN_LIB := ../../../../../frozen/include
PP_LIB := ../../UAVCAN

CPPSRC = ../../UAVCAN/persistantParam.cpp \
	 ../../UAVCAN/customAllocator.cpp \
         testPP.cpp

# Compiler
CXX := g++-14

# Compiler Flags
CXXFLAGS := -Wall -Wextra -std=c++26



# Object files (replace .cpp with .o)
OBJ := $(CPPSRC:.cpp=.o)

# Executable Name
TARGET := testPP

# Default rule: Build the executable
$(TARGET): $(OBJ) 
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to compile .cpp files to .o
%.o: %.cpp ../../UAVCAN/persistantParam.hpp \
           ../../UAVCAN/tinyString.hpp \
           ../../UAVCAN/customAllocator.hpp \
           nodeParameters.hpp
	$(CXX) $(CXXFLAGS) \
        -I$(ETL_LIB) -I$(FROZEN_LIB) -I$(PP_LIB) -I. \
        -c $< -o $@

# Clean rule
.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)
