/* Definitions for option handling for XStormy16.
   Copyright (C) 2012
   Free Software Foundation, Inc.

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

#ifndef STORMY16_OPTS_H
#define STORMY16_OPTS_H

/* Type of warning to produce when accessing bitfields in the I/O
   address space.  */
enum set1clr1_fix_type
{
  SET1CLR1_FIX_AUTO,    /* Warn and recompile if necessary.  */
  SET1CLR1_FIX_SILENT,  /* Recompile without warning.  */
  SET1CLR1_FIX_WARN,    /* Warn, but do not recompile.  */
  SET1CLR1_FIX_NOTHING  /* Do nothing.  */
};

#endif /* STORMY16_OPTS_H */
