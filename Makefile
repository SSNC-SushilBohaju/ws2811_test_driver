# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q       := @
NULL    := 2>/dev/null
endif

SRC_DIR     = src
INC_DIR     = inc

BINARY = firmware

###############################################################################
# Basic Device Setup

###############################################################################
# Linkerscript

# Linker script and libraries not required for Raspberry Pi

###############################################################################
# Includes

# Include directories
DEFS        += -I$(INC_DIR)

###############################################################################
# Executables

CC      := gcc
CXX     := g++
LD      := gcc
AR      := ar
AS      := as
OBJCOPY     := objcopy
OBJDUMP     := objdump
GDB     := gdb
OPT     := -Os
DEBUG       := -ggdb3
CSTD        ?= -std=c99

###############################################################################
# Source files

OBJS        += $(SRC_DIR)/$(BINARY).o
OBJS        += $(SRC_DIR)/core/system.o
OBJS        += $(SRC_DIR)/ws2812b.o

###############################################################################
# C flags

TGT_CFLAGS  += $(OPT) $(CSTD) $(DEBUG)
TGT_CFLAGS  += -Wextra -Wshadow -Wimplicit-function-declaration
TGT_CFLAGS  += -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
TGT_CFLAGS  += -fno-common -ffunction-sections -fdata-sections

###############################################################################
# C++ flags

TGT_CXXFLAGS    += $(OPT) $(CXXSTD) $(DEBUG)
TGT_CXXFLAGS    += -Wextra -Wshadow -Wredundant-decls  -Weffc++
TGT_CXXFLAGS    += -fno-common -ffunction-sections -fdata-sections

###############################################################################
# C & C++ preprocessor common flags

TGT_CPPFLAGS    += -MD
TGT_CPPFLAGS    += -Wall -Wundef
TGT_CPPFLAGS    += $(DEFS)

###############################################################################
# Linker flags

TGT_LDFLAGS     += $(DEBUG)
TGT_LDFLAGS     += -Wl,-Map=$(*).map -Wl,--cref
TGT_LDFLAGS     += -Wl,--gc-sections
ifeq ($(V),99)
TGT_LDFLAGS     += -Wl,--print-gc-sections
endif

###############################################################################
# Used libraries

LDLIBS      += -pthread -lm -lrt

###############################################################################
###############################################################################
###############################################################################

.SUFFIXES: .elf .bin .hex .srec .list .map .images
.SECONDEXPANSION:
.SECONDARY:

all: elf bin

elf: $(BINARY).elf
bin: $(BINARY).bin
hex: $(BINARY).hex
srec: $(BINARY).srec
list: $(BINARY).list
GENERATED_BINARIES=$(BINARY).elf $(BINARY).bin $(BINARY).hex $(BINARY).srec $(BINARY).list $(BINARY).map

images: $(BINARY).images
flash: $(BINARY).flash

# Define a helper macro for debugging make errors online
# you can type "make print-OPENCM3_DIR" and it will show you
# how that ended up being resolved by all of the included
# makefiles.
print-%:
	@echo $*=$($*)

%.images: %.bin %.hex %.srec %.list %.map
	@#printf "*** $* images generated ***\n"

%.bin: %.elf
	@#printf "  OBJCOPY $(*).bin\n"
	$(Q)$(OBJCOPY) -Obinary $(*).elf $(*).bin

%.hex: %.elf
	@#printf "  OBJCOPY $(*).hex\n"
	$(Q)$(OBJCOPY) -Oihex $(*).elf $(*).hex

%.srec: %.elf
	@#printf "  OBJCOPY $(*).srec\n"
	$(Q)$(OBJCOPY) -Osrec $(*).elf $(*).srec

%.list: %.elf
	@#printf "  OBJDUMP $(*).list\n"
	$(Q)$(OBJDUMP) -S $(*).elf > $(*).list

%.elf %.map: $(OBJS) Makefile
	@#printf "  LD      $(*).elf\n"
	$(Q)$(LD) $(TGT_LDFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(*).elf

%.o: %.c
	@#printf "  CC      $(*).c\n"
	$(Q)$(CC) $(TGT_CFLAGS) $(CFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).c

%.o: %.S
	@#printf "  CC      $(*).S\n"
	$(Q)$(CC) $(TGT_CFLAGS) $(CFLAGS) -o $(*).o -c $(*).S

%.o: %.cxx
	@#printf "  CXX     $(*).cxx\n"
	$(Q)$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).cxx

%.o: %.cpp
	@#printf "  CXX     $(*).cpp\n"
	$(Q)$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).cpp

clean:
	@#printf "  CLEAN\n"
	$(Q)$(RM) $(GENERATED_BINARIES) generated.* $(OBJS) $(OBJS:%.o=%.d)

.PHONY: images clean elf bin hex srec list

-include $(OBJS:.o=.d)
