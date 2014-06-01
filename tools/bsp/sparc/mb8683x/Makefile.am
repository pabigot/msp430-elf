## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

@COMMON_IN@

BOARD_NAME = mb8683x

if BUILD_IT
DEPEND_TARGETS=$(patsubst %.o,.deps/%.d,$(OBJECTS))
BOARD_TARGET  = bsp.rom
BOARD_XTRAS = $(BOARD_NAME)-start.o $(BOARD_NAME)-crt0.o
BOARD_HDRS =
BOARD_SPECS = @top_srcdir@/mb86x-rom.specs @top_srcdir@/mb86x.specs
endif

BOARD_INCDIR = $(top_srcdir)/include
BOARD_ROM_STARTUP = $(BOARD_NAME)-start.o
BOARD_ROM_SPECS   = mb86x-rom.specs

COMP_FLAGS= -mcpu=sparclite -L. -B${top_srcdir}/
LDFLAGS = $(COMP_FLAGS) -specs=mb86x-rom.specs
AM_CFLAGS = $(LDFLAGS) -DNO_INIT_FUNCTION -DGDB_THREAD_SUPPORT

INCLUDES = $(BSP_INCLUDES) -I$(BOARD_INCDIR)

noinst_DATA = $(DEPEND_TARGETS) $(BOARD_TARGET)
toollib_DATA = $(BOARD_XTRAS)
toollib_LIBRARIES = @BOARD_LIB@
EXTRA_LIBRARIES = libmb8683x.a

CLEANFILES=

BOARD_SRC = init_mb8683x.S     \
	    mb8683x.c          \
	    mb86964.c          \
	    cache-sparcl.S

include ../cpu.mk
include ../../common/common.mk

libmb8683x_a_SOURCES = $(BOARD_SRC)   \
	               $(CPU_SOURCE)  \
	               $(COMMON_SRC)  \
	               $(SYSLIB_SRC)  \
		       $(THREADS_SRC) \
		       $(GDBSTUB_SRC) \
		       $(NET_SRC)

install-data-local: 
	$(mkinstalldirs) $(tooldir)/include/bsp
	-for i in $(BOARD_HDRS); do \
	    $(INSTALL_DATA) $$i $(tooldir)/include/bsp/`basename $$i`; \
	done;
	-for i in $(BOARD_SPECS); do \
	    $(INSTALL_DATA) $$i $(tooldir)/lib/`basename $$i`; \
	done;
