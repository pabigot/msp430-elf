## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

@COMMON_IN@

BOARD_NAME = dve39

if BUILD_IT
DEPEND_TARGETS=$(patsubst %.o,.deps/%.d,$(OBJECTS))
BOARD_TARGET  = 
BOARD_XTRAS = $(BOARD_NAME).ld $(BOARD_NAME)-rom.ld $(BOARD_NAME)-start.o $(BOARD_NAME)-crt0.o
BOARD_HDRS = @top_srcdir@/include/bsp/$(BOARD_NAME).h
BOARD_SPECS = @top_srcdir@/$(BOARD_NAME)-rom.spec @top_srcdir@/$(BOARD_NAME).specs
endif

BOARD_INCDIR = $(top_srcdir)/include
BOARD_ROM_STARTUP = $(BOARD_NAME)-start.o
BOARD_ROM_SPECS   = $(BOARD_NAME)-rom.specs

AM_CFLAGS = -B$(top_srcdir)/ -B./ -B../ -DNO_INIT_FUNCTION -DGDB_THREAD_SUPPORT -G 0 -specs=$(BOARD_NAME)-rom.specs

INCLUDES = $(BSP_INCLUDES) -I$(BOARD_INCDIR)

noinst_DATA = $(DEPEND_TARGETS) $(BOARD_TARGET)
toollib_DATA = $(BOARD_XTRAS)
toollib_LIBRARIES = @BOARD_LIB@
EXTRA_LIBRARIES = libdve39.a

CLEANFILES=

BOARD_SRC = init_dve39.S      \
	    dve39.c           \
	    i82596.c          \
	    ticks.c

include ../cpu.mk
include ../../common/common.mk

libdve39_a_SOURCES = $(BOARD_SRC)   \
	             $(CPU_SOURCE)  \
	             $(COMMON_SRC)  \
	             $(SYSLIB_SRC)  \
		     $(THREADS_SRC) \
		     $(GDBSTUB_SRC) \
		     $(NET_SRC)

dve39.ld: @top_srcdir@/../mips.ldtempl
	rm -f $@
	$(CC) -C -P -E -xc                     \
              -DBSP_STARTFILES=dve39-crt0.o    \
              -DBSP_GROUP="-lc -ldve39 -lgcc"  \
              -DBSP_TEXT_START=0x80010000      \
	      $< > $@

dve39-rom.ld: @top_srcdir@/../mips.ldtempl
	rm -f $@
	$(CC) -C -P -E -xc                     \
              -DBSP_STARTFILES=dve39-start.o   \
              -DBSP_GROUP="-lc -ldve39 -lgcc"  \
              -DBSP_TEXT_START=0xbfc00000      \
              -DBSP_DATA_START=0x80000100      \
	      $< > $@

install-data-local: 
	$(mkinstalldirs) $(tooldir)/include/bsp
	-for i in $(BOARD_HDRS); do \
	    $(INSTALL_DATA) $$i $(tooldir)/include/bsp/`basename $$i`; \
	done;
	-for i in $(BOARD_SPECS); do \
	    $(INSTALL_DATA) $$i $(tooldir)/lib/`basename $$i`; \
	done;
