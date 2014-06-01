## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

BOARD_NAME = sa-iop

@COMMON_IN@

if BUILD_IT
DEPEND_TARGETS=$(patsubst %.o,.deps/%.d,$(OBJECTS))
BOARD_TARGET  = bsp.rom
BOARD_HDRS = @top_srcdir@/include/bsp/$(BOARD_NAME).h
BOARD_SPECS = $(BOARD_NAME)-rom.specs $(BOARD_NAME).specs
BOARD_XTRAS = $(BOARD_NAME)-start.o $(BOARD_NAME)-crt0.o
endif

BOARD_INCDIR = $(top_srcdir)/include
BOARD_ROM_STARTUP = $(BOARD_NAME)-start.o
BOARD_ROM_SPECS   = $(BOARD_NAME)-rom.specs

COMP_FLAGS= -L. -B./ -B${top_srcdir}/../../common/
LDFLAGS = $(COMP_FLAGS) -specs=$(BOARD_NAME)-rom.specs
AM_CFLAGS = $(LDFLAGS) -DNO_INIT_FUNCTION -DFCLK_MHZ=60

INCLUDES = $(BSP_INCLUDES) -I$(BOARD_INCDIR)

noinst_DATA = $(DEPEND_TARGETS) $(BOARD_TARGET)
toollib_DATA = $(BOARD_XTRAS) $(BOARD_SPECS)
toollib_LIBRARIES = @BOARD_LIB@
EXTRA_LIBRARIES = libsa-iop.a

CLEANFILES=

BOARD_SRC = \
	sa-110.c          \
	init_sa-iop.S     \
	mem285.S          \
	sa-iop.c       

include ../cpu.mk
include ../../common/common.mk

libsa_iop_a_SOURCES = $(BOARD_SRC)   \
	                $(CPU_SOURCE)  \
	                $(COMMON_SRC)  \
	                $(SYSLIB_SRC)  \
		        $(THREADS_SRC) \
		        $(GDBSTUB_SRC)

install-data-local: 
	$(mkinstalldirs) $(tooldir)/include/bsp
	-for i in $(BOARD_HDRS); do \
	    $(INSTALL_DATA) $$i $(tooldir)/include/bsp/`basename $$i`; \
	done;
	-for i in $(BOARD_SPECS); do \
	    $(INSTALL_DATA) $$i $(tooldir)/lib/`basename $$i`; \
	done;
