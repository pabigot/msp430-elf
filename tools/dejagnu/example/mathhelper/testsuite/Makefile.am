##
##  $Id: Makefile.am,v 1.1 2006/08/04 10:40:57 nickc Exp $
## 

AUTOMAKE_OPTIONS = dejagnu

SUBDIRS = mathhelper

all:
	@echo "Nothing to be done for all"

RUNTESTDEFAULTFLAGS = --tool mathhelper

RUNTEST = `if test -x $(top_srcdir)/../dejagnu/runtest; then \
	      echo $(top_srcdir)/../dejagnu/runtest; \
	   else \
	      echo runtest; \
           fi`

CLEANFILES = *.log *.sum site.bak
