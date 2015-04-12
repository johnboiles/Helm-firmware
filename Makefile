#
# This makefile started life as the Makefile from teensy-template, found here:
# https://github.com/apmorton/teensy-template
#

ifeq ($(v),)
export verbose = 0
else
export verbose = 1
endif

ifeq ($(verbose),)
export verbose = 0
endif

ifeq ($(verbose),0)
Q = @
MAKEFLAGS += -s
else
Q =
endif
export Q

# The name of your project (used to name the compiled .hex file)
TARGET = $(notdir $(CURDIR))

# configurable options
OPTIONS = -DF_CPU=96000000 -DUSB_SERIAL -DLAYOUT_US_ENGLISH

# options needed by many Arduino libraries to configure for Teensy 3.0
OPTIONS += -D__MK20DX256__ -DARDUINO=105

BUILDDIR = build

ifdef COMSPEC
cygpath-win    = $(shell cygpath -w "$1")
else
cygpath-win    = $1
endif

#************************************************************************
# Location of Teensyduino utilities, Toolchain, and Arduino Libraries.
# To use this makefile without Arduino, copy the resources from these
# locations and edit the pathnames.  The rest of Arduino is not needed.
#************************************************************************

ARDUINO ?= /Applications/Arduino.app/Contents/Resources/Java

# path location for Teensy Loader, teensy_post_compile and teensy_reboot
TOOLS_PATH = $(ARDUINO)/hardware/tools

# path location for Teensy 3 core
CORE_PARENT = $(ARDUINO)/hardware/teensy/cores
CORE_PATH = $(CORE_PARENT)/teensy3

# path location for Arduino libraries
LIBRARY_PARENT = .
LIBRARY_PATH = $(LIBRARY_PARENT)/libraries

# path location for the arm-none-eabi compiler
COMPILER_PATH = $(TOOLS_PATH)/arm-none-eabi/bin

#************************************************************************
# Settings below this point usually do not need to be edited
#************************************************************************

# CPPFLAGS = compiler options for C and C++
CPPFLAGS = -Wall -Wno-psabi -g -Os -mcpu=cortex-m4 -mthumb -nostdlib -MMD $(OPTIONS) -Isrc -I$(CORE_PATH)

# compiler options for C++ only
CXXFLAGS = -std=gnu++0x -felide-constructors -fno-exceptions -fno-rtti

# compiler options for C only
CFLAGS =

LDSCRIPT = $(CORE_PATH)/mk20dx256.ld

# linker options
LDFLAGS = -Os -Wl,--gc-sections -mcpu=cortex-m4 -mthumb -T$(LDSCRIPT)

# additional libraries to link
LIBS = -lm

# names for the compiler programs
CC = $(COMPILER_PATH)/arm-none-eabi-gcc
CXX = $(COMPILER_PATH)/arm-none-eabi-g++
TEST_CXX = /usr/bin/g++
OBJCOPY = $(COMPILER_PATH)/arm-none-eabi-objcopy
SIZE = $(COMPILER_PATH)/arm-none-eabi-size

vpath %.c   $(LIBRARY_PARENT)
vpath %.cpp $(LIBRARY_PARENT)

vpath %.c   $(CORE_PARENT)
vpath %.cpp $(CORE_PARENT)

# automatically create lists of the sources and objects
LC_FILES := $(patsubst $(LIBRARY_PARENT)/%, %, $(wildcard $(LIBRARY_PATH)/*/*.c))
LCPP_FILES := $(patsubst $(LIBRARY_PARENT)/%, %, $(wildcard $(LIBRARY_PATH)/*/*.cpp))
TC_FILES := $(patsubst $(CORE_PARENT)/%, %, $(wildcard $(CORE_PATH)/*.c))
TCPP_FILES := $(patsubst $(CORE_PARENT)/%, %, $(wildcard $(CORE_PATH)/*.cpp))
C_FILES := $(wildcard *.c)
CPP_FILES := $(wildcard *.cpp)
INO_FILES := $(wildcard *.ino)

# include paths for libraries
L_INC := $(foreach lib,$(filter %/, $(wildcard $(LIBRARY_PATH)/*/)), -I$(lib))

OBJS_FILES := $(INO_FILES:.ino=.o) $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o) $(TC_FILES:.c=.o) $(TCPP_FILES:.cpp=.o) $(LC_FILES:.c=.o) $(LCPP_FILES:.cpp=.o)
OBJS := $(foreach obj,$(OBJS_FILES), $(BUILDDIR)/$(obj))
TEST_FILES := $(C_FILES) $(CPP_FILES)

all: hex

build: $(TARGET).elf

hex: $(TARGET).hex

post_compile: $(TARGET).hex
	$(Q)$(TOOLS_PATH)/teensy_post_compile -file="$(basename $<)" -path="$(call cygpath-win,$(CURDIR))" -tools="$(TOOLS_PATH)"

reboot:
	$(Q)-$(TOOLS_PATH)/teensy_reboot

upload: post_compile reboot

upload-remote: $(TARGET).hex
	upload-remote.sh

test:
	@echo "Compiling tests $(TEST_FILES)"
	$(TEST_CXX) -Wall -Itesting -I. $(TEST_FILES) testing/ArduinoMock.cpp testing/Tests.cpp -o test_suite
	@echo "Running tests"
	./test_suite
	rm test_suite

$(BUILDDIR)/%.o: %.c
	@echo "[CC]\t$<"
	$(Q)mkdir -p "$(dir $@)"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) $(L_INC) -o "$@" -c "$<"

$(BUILDDIR)/%.o: %.ino
	@echo "[CXX]\t$<"
	$(Q)mkdir -p "$(dir $@)"
	$(Q)$(CC) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o "$@" -c -x c++ -include Arduino.h "$<"

$(BUILDDIR)/%.o: %.cpp
	@echo "[CXX]\t$<"
	@mkdir -p "$(dir $@)"
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o "$@" -c "$<"

$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@echo "[LD]\t$@"
	$(Q)$(CC) $(LDFLAGS) -o "$@" -Wl,-Map,$(TARGET).map $(OBJS) $(LIBS)

%.hex: %.elf
	@echo "[HEX]\t$@"
	$(Q)$(SIZE) "$<"
	$(Q)$(OBJCOPY) -O ihex -R .eeprom "$<" "$@"

# compiler generated dependency info
-include $(OBJS:.o=.d)

clean:
	@echo Cleaning...
	$(Q)rm -rf "$(BUILDDIR)"
	$(Q)rm -f "$(TARGET).elf" "$(TARGET).hex" "$(TARGET).map"m