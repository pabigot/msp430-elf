#
# ykb/board.mk -- Makefile fragment for Yellowknife w/Memory Map B
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
BOARD_NAME	    = ykb

BOARD_SRCDIR	    = @top_srcdir@/@archdir@/$(BOARD_GENERIC_NAME)/$(BOARD_NAME)
BOARD_SRCDIR_EXTRAS = @top_srcdir@/@archdir@/$(BOARD_GENERIC_NAME)
BOARD_INCDIR	    = 

BOARD_HEADERS	    = 
BOARD_LIBBSP	    = lib$(BOARD_NAME).a
BOARD_OBJS	    = init_$(BOARD_GENERIC_NAME).o $(BOARD_GENERIC_NAME).o cache6xx.o
BOARD_CFLAGS_RAM    = -mcpu=603e -mstrict-align
BOARD_CFLAGS	    = -mcpu=603e -mstrict-align -mno-sdata -mno-toc
BOARD_LDFLAGS	    = -L../../../newlib/
BOARD_DEFINES	    = 

BOARD_SPECS	    = $(BOARD_NAME)-rom.specs
BOARD_RAM_SPECS	    = $(BOARD_NAME).specs
BOARD_ROM_STARTUP   = $(BOARD_NAME)-start.o
BOARD_RAM_STARTUP   = $(BOARD_NAME)-crt0.o
BOARD_EXTRAS	    = 
