/* Core target definitions for GNU compiler
   for IBM RS/6000 PowerPC targeted to embedded ELF systems.
   Copyright (C) 1995-2013 Free Software Foundation, Inc.
   Contributed by Cygnus Support.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

/* Add -meabi to target flags.  */
#undef  TARGET_DEFAULT
#define TARGET_DEFAULT MASK_EABI

/* Invoke an initializer function to set up the GOT.  */
#define NAME__MAIN "__eabi"
#define INVOKE__main

#undef  TARGET_OS_CPP_BUILTINS
#define TARGET_OS_CPP_BUILTINS()          \
  do                                      \
    {                                     \
      builtin_define_std ("PPC");         \
      builtin_define ("__embedded__");    \
      builtin_assert ("system=embedded"); \
      builtin_assert ("cpu=powerpc");     \
      builtin_assert ("machine=powerpc"); \
      TARGET_OS_SYSV_CPP_BUILTINS ();     \
    }                                     \
  while (0)
/* FIXME:  These lines are here so that libstdc++-v3 will configure
   and build for various targets like powerc-eabi.  The problem is
   that currently the default startfile and endfile specs for such
   targets is empty, so the symbols needed by eabi-ci.asm and and
   eabi-cn.asm are not defined.  (They are in ecrti.o).  According
   to David Edelsohn the correct fix is to include rs6000/freebsd.h
   in the tm_file list for these targets so that they defaul to the
   FreeBSD startfile spec defined in that header file.  The problem
   with this solution is that it does not build crt1.o.  This is
   presumed to be supplied by the FreeBSD host.  If we are cross
   compiling then and do not have this file available we are
   scuppered.  NickC 3 Feb 06.  */
#undef	STARTFILE_DEFAULT_SPEC
#define STARTFILE_DEFAULT_SPEC "ecrti.o%s"

#undef	ENDFILE_DEFAULT_SPEC
#define ENDFILE_DEFAULT_SPEC "ecrtn.o%s"
