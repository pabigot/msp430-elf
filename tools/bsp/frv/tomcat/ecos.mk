#
# tomcat/board.mk_ecos -- Makefile fragment for Tomcat EVB based on eCos HAL.
#
# Copyright (c) 1999, 2000 Cygnus Solutions
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
BOARD_NAME          = tomcat

BOARD_SRCDIR	    = @top_srcdir@/@archdir@/$(BOARD_NAME)
BOARD_INCDIR	    = $(BOARD_NAME)/include

BOARD_HEADERS	    =
BOARD_LIBBSP	    = lib$(BOARD_NAME).a
BOARD_OBJS	    = ticks.o ne2k.o
BOARD_CFLAGS	    =
BOARD_LDFLAGS	    = -L../../newlib/
BOARD_DEFINES	    = -DUSE_MB86941=1 -D__CPU_TOMCAT__ -D__TOMCAT_EVB__

BOARD_SPECS	    = $(BOARD_NAME)-rom.specs
BOARD_RAM_SPECS	    = $(BOARD_NAME).specs
BOARD_RAM_STARTUP   = $(BOARD_NAME)-crt0.o
BOARD_EXTRAS	    = 
