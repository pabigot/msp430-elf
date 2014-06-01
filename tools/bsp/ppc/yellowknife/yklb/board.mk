#
# yklb/board.mk -- Makefile fragment for Yellowknife w/Memory Map B (Little Endian)
#
# Copyright (c) 1999 Cygnus Solutions
#
# The authors hereby grant permission to use, copy, modify, distribute,
# and license this software and its documentation for any purpose, provided
# that existing copyright notices are retained in all copies and that this
# notice is included verbatim in any distributions. No written agreement,
# license, or royalty fee is required for any of the authorized uses.
# Modifications to this software may be copyrighted by their authors
# and need not follow the licensing terms described here, provided that
# the new terms are clearly indicated on the first page of each file where
# they apply.
#
BUILD_OPTIONS	   := $(BUILD_OPTIONS)
BOARD_GENERIC_NAME  = yellowknife
BOARD_NAME	    = yklb

BOARD_SRCDIR	    = @top_srcdir@/@archdir@/$(BOARD_GENERIC_NAME)/$(BOARD_NAME)
BOARD_SRCDIR_EXTRAS = @top_srcdir@/@archdir@/$(BOARD_GENERIC_NAME)
BOARD_INCDIR	    = 

BOARD_HEADERS	    = 
BOARD_LIBBSP	    = lib$(BOARD_NAME).a
BOARD_OBJS	    = init_$(BOARD_GENERIC_NAME).o $(BOARD_GENERIC_NAME).o cache6xx.o
BOARD_EXTRA_OBJS    = rst-big.o init_$(BOARD_GENERIC_NAME)-big.o
BOARD_CFLAGS_RAM    = -mlittle -mcpu=603e -mstrict-align
BOARD_CFLAGS	    = -mlittle -mcpu=603e -mstrict-align -mno-sdata -mno-toc
BOARD_LDFLAGS	    = -L../../../le/newlib/
BOARD_DEFINES	    = 

BOARD_SPECS	    = $(BOARD_NAME)-rom.specs
BOARD_RAM_SPECS	    = $(BOARD_NAME).specs
BOARD_ROM_STARTUP   = $(BOARD_NAME)-start-big.o $(BOARD_NAME)-start.o
BOARD_RAM_STARTUP   = $(BOARD_NAME)-crt0.o

BOARD_BIG_CFLAGS    = -D__MISSING_SYSCALL_NAMES__ -D__SWITCH_TO_LE__ -D__CPU_MPC6XX__ \
		      -D__BOARD_YKLB__ -D__MPC106_MAP_B__ -D__HAVE_FPU__ \
		      -meabi -mcpu=603e -mstrict-align -mno-sdata -mno-toc -mbig

BOARD_CLEAN	    = $(BOARD_NAME)-boot.rom $(BOARD_NAME)-boot.bin

#
# Big endian modules
#
$(BOARD_NAME)-boot.bin: $(BOARD_NAME)-boot.rom Makefile $(ALL_DEPEND_OBJS)
	$(OBJCOPY) --pad-to=0xfff01000 -O binary $(BOARD_NAME)-boot.rom $@

$(BOARD_NAME)-boot.rom: $(BOARD_NAME)-rst-big.o init_$(BOARD_GENERIC_NAME)-big.o Makefile $(ALL_DEPEND_OBJS)
	$(CC) $(CPPFLAGS) $(DEFINES) $(ALL_BIG_CFLAGS) --nostartfiles \
		-L$(COMMON_SRCDIR)/ -Wl,-T,$(LDSCRIPT),-Ttext,0xfff00000 \
		$(BOARD_NAME)-rst-big.o init_$(BOARD_GENERIC_NAME)-big.o -o $@

#
# Little Endian object file containing the binary
# contents of the big endian boot module created above.
#
$(BOARD_NAME)-start-big.o: $(BOARD_NAME)-boot.bin $(BOARD_NAME)-start.o
	-$(OBJCOPY) \
		--add-section .rom_vec=$(BOARD_NAME)-boot.bin \
		--remove-section .text \
		--remove-section .data \
		--remove-section .bss  \
		$(BOARD_NAME)-start.o $@ 2>&1 | \
		grep -v "Output file cannot represent architecture"; true
