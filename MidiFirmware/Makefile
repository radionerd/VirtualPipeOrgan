SHELL := bash
.ONESHELL:
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

ifeq ($(origin .RECIPEPREFIX), undefined)
  $(error This Make does not support .RECIPEPREFIX. Please use GNU Make 4.0 or later)
endif
.RECIPEPREFIX = >

# Default - top level rule is what gets ran when you run just `make`
build: out/image-id
.PHONY: build

# Clean up the output directories; since all the sentinel files go under tmp, this will cause everything to get rebuilt
clean:
> rm -rf tmp
> rm -rf out
.PHONY: clean
CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g $(shell root-config --cflags)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs)

SRCS=adc.cpp hid_pt.cpp multilcd.cpp TM1637.cpp bluepill_ws2812.cpp keyboardscan.cpp mymidi.cpp USBSerialUI.cpp buttonscan.cpp LiquidCrystal_PCF8574RJ.cpp profile.cpp   ws2812ctrl.cpp

OBJS=$(subst .cc,.o,$(SRCS))

all: tool

tool: $(OBJS)
>   $(CXX) $(LDFLAGS) -o tool $(OBJS) $(LDLIBS)

tool.o: tool.cc support.hh

support.o: support.hh support.cc

clean:
>   $(RM) $(OBJS)

distclean: clean
>   $(RM) tool
# List of all .cpp source files.
CPP = main.cpp $(wildcard dir1/*.cpp) $(wildcard dir2/*.cpp)

# All .o files go to build dir.
OBJ = $(CPP:%.cpp=$(BUILD_DIR)/%.o)
# Gcc/Clang will create these .d files containing dependencies.
DEP = $(OBJ:%.o=%.d)

# Default target named after the binary.
$(BIN) : $(BUILD_DIR)/$(BIN)

# Actual target of the binary - depends on all .o files.
$(BUILD_DIR)/$(BIN) : $(OBJ)
    # Create build directories - same structure as sources.
>   mkdir -p $(@D)
    # Just link all the object files.
>   $(CXX) $(CXX_FLAGS) $^ -o $@

# Include all .d files
-include $(DEP)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(DEP)`.
$(BUILD_DIR)/%.o : %.cpp
>   mkdir -p $(@D)
    # The -MMD flags additionaly creates a .d file with
    # the same name as the .o file.
>   $(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

.PHONY : clean
clean :
    # This should remove all generated files.
>   -rm $(BUILD_DIR)/$(BIN) $(OBJ) $(DEP)
