## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

@COMMON_IN@

BOARD_NAME = tomcat

if BUILD_IT
DEPEND_TARGETS=$(patsubst %.o,.deps/%.d,$(OBJECTS))
BOARD_TARGET =
BOARD_XTRAS = $(BOARD_NAME)-start.o $(BOARD_NAME)-crt0.o
BOARD_HDRS =
BOARD_SPECS = @top_srcdir@/$(BOARD_NAME)-rom.specs @top_srcdir@/$(BOARD_NAME).specs
endif

BOARD_INCDIR = $(top_srcdir)/include
BOARD_ROM_STARTUP = $(BOARD_NAME)-start.o
BOARD_ROM_SPECS   = $(BOARD_NAME)-rom.specs

COMP_FLAGS= -L. -B${top_srcdir}/ -B${top_srcdir}/../ -fcommon
LDFLAGS = $(COMP_FLAGS) -G0 -specs=$(BOARD_NAME)-rom.specs
AM_CFLAGS = $(LDFLAGS) -G0 -DUSE_MB86941=1 -DHAVE_BSP_CLOCK -DINIT_FUNC=__do_init

INCLUDES = $(BSP_INCLUDES) -I$(BOARD_INCDIR)

noinst_DATA = $(DEPEND_TARGETS) $(BOARD_TARGET)
toollib_DATA = $(BOARD_XTRAS)
toollib_LIBRARIES = @BOARD_LIB@
EXTRA_LIBRARIES = libtomcat.a

CLEANFILES=

BOARD_SRC = init_tomcat.S     \
	    tomcat.c          \
	    ne2k.c            \
	    ticks.c

include ../cpu.mk
include ../../common/common.mk

libtomcat_a_SOURCES = $(BOARD_SRC)   \
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
