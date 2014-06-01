/* Definitions of target machine for GNU compiler.
   Big-endian flavor.
   Copyright (c) 1997 Cygnus Support Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#define TARGET_ENDIAN_DEFAULT MASK_BIG_ENDIAN
#define MULTILIB_ENDIAN_DEFAULT "EB"
