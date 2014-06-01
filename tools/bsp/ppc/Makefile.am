## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

SUBDIRS = @board_list@

CLEANFILES=

install-data-local: 
	$(mkinstalldirs) $(tooldir)/include/bsp
	-for i in @top_srcdir@/include/bsp/*.h; do \
	    $(INSTALL_DATA) $$i $(tooldir)/include/bsp/`basename $$i`; \
	done;
	-for i in @top_srcdir@/../include/bsp/*.h; do \
	    $(INSTALL_DATA) $$i $(tooldir)/include/bsp/`basename $$i`; \
	done;

CONFIG_STATUS_DEPENDENCIES = $(bsp_basedir)/configure.host

MAKEOVERRIDES=

