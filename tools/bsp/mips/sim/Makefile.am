## mips/sim/Makefile.am - Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

@COMMON_IN@

BOARD_NAME = simstub

if BUILD_IT
DEPEND_TARGETS = $(patsubst %.o,.deps/%.d,$(OBJECTS))
BOARD_TARGET  = 
BOARD_XTRAS = $(BOARD_NAME).ld $(BOARD_NAME)-rom.ld $(BOARD_NAME)-start.o $(BOARD_NAME)-crt0.o
BOARD_SPECS = @top_srcdir@/$(BOARD_NAME)-rom.spec @top_srcdir@/$(BOARD_NAME).specs
endif

BOARD_INCDIR = $(top_srcdir)/include
BOARD_ROM_STARTUP = $(BOARD_NAME)-start.o
BOARD_ROM_SPECS   = $(BOARD_NAME)-rom.specs

AM_CFLAGS = -DNO_INIT_FUNCTION -DGDB_THREAD_SUPPORT -G 0 -specs=$(BOARD_NAME)-rom.specs

INCLUDES = $(BSP_INCLUDES) -I$(BOARD_INCDIR)

noinst_DATA = $(DEPEND_TARGETS) $(BOARD_TARGET)
toollib_DATA = $(BOARD_XTRAS)
toollib_LIBRARIES = @BOARD_LIB@
EXTRA_LIBRARIES = libsimstub.a

CLEANFILES=

BOARD_SRC = init_sim.S        \
	    sim.c

include ../cpu.mk
include ../../common/common.mk

libsimstub_a_SOURCES = $(BOARD_SRC)   \
		       $(CPU_SOURCE)  \
	               $(COMMON_SRC)  \
	               $(SYSLIB_SRC)  \
		       $(THREADS_SRC) \
		       $(GDBSTUB_SRC)

simstub.ld: @top_srcdir@/../mips.ldtempl
	rm -f $@
	$(CC) -C -P -E -xc                     \
              -DBSP_STARTFILES=simstub-crt0.o    \
              -DBSP_GROUP="-lc -lsimstub -lgcc"  \
              -DBSP_TEXT_START=0x80010000      \
	      $< > $@

simstub-rom.ld: @top_srcdir@/../mips.ldtempl
	rm -f $@
	$(CC) -C -P -E -xc                     \
              -DBSP_STARTFILES=simstub-start.o   \
              -DBSP_GROUP="-lc -lsimstub -lgcc"  \
              -DBSP_TEXT_START=0xbfc00000      \
              -DBSP_DATA_START=0x80000200      \
	      $< > $@

install-data-local: 
	-for i in $(BOARD_SPECS); do \
	    $(INSTALL_DATA) $$i $(tooldir)/lib/`basename $$i`; \
	done;
