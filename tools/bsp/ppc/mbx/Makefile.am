## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

@COMMON_IN@

BOARD_NAME = mbx

if BUILD_IT
DEPEND_TARGETS=$(patsubst %.o,.deps/%.d,$(OBJECTS))
BOARD_TARGET  = bsp.rom
BOARD_XTRAS = $(BOARD_NAME)-start.o $(BOARD_NAME)-crt0.o
BOARD_HDRS =
BOARD_SPECS = @top_srcdir@/$(BOARD_NAME)-rom.specs @top_srcdir@/$(BOARD_NAME).specs
endif

BOARD_INCDIR = $(top_srcdir)/include
BOARD_ROM_STARTUP = $(BOARD_NAME)-start.o
BOARD_ROM_SPECS   = $(BOARD_NAME)-rom.specs

noinst_DATA = $(DEPEND_TARGETS) $(BOARD_TARGET)
toollib_DATA = $(BOARD_XTRAS)
toollib_LIBRARIES = @BOARD_LIB@
EXTRA_LIBRARIES = libmbxbsp.a

CLEANFILES=

BOARD_SRC = init_mbx.S     \
	    mbx.c          \
	    cache8xx.S

include ../cpu.mk
include ../../common/common.mk

COMP_FLAGS= -L. -B${top_srcdir}/ -B${top_srcdir}/../
LDFLAGS = $(COMP_FLAGS) -specs=$(BOARD_NAME)-rom.specs
AM_CFLAGS = $(LDFLAGS) $(CPU_DEFINES)

INCLUDES = $(BSP_INCLUDES) -I$(BOARD_INCDIR)

libmbxbsp_a_SOURCES = $(BOARD_SRC)   \
	              $(CPU_SOURCE)  \
	              $(COMMON_SRC)  \
	              $(SYSLIB_SRC)  \
		      $(GDBSTUB_SRC)

install-data-local: 
	$(mkinstalldirs) $(tooldir)/include/bsp
	-for i in $(BOARD_HDRS); do \
	    $(INSTALL_DATA) $$i $(tooldir)/include/bsp/`basename $$i`; \
	done;
	-for i in $(BOARD_SPECS); do \
	    $(INSTALL_DATA) $$i $(tooldir)/lib/`basename $$i`; \
	done;
