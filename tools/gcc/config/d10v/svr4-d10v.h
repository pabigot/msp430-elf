/* Operating system specific defines to be used when targeting GCC for some
   generic System V Release 4 system.
   
   Copyright (C) 1991, 94-98, 1999, 2002, 2003, 2009
   Free Software Foundation, Inc.
   Contributed by Ron Guilmette (rfg@monkeys.com).

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */


/* To use this file, make up a file with a name like:

	?????svr4.h

   where ????? is replaced by the name of the basic hardware that you
   are targeting for.  Then, in the file ?????svr4.h, put something
   like:

	#include "?????.h"
	#include "svr4.h"

   followed by any really system-specific defines (or overrides of
   defines) which you find that you need.  For example, CPP_PREDEFINES
   is defined here with only the defined -Dunix and -DSVR4.  You should
   probably override that in your target-specific ?????svr4.h file
   with a set of defines that includes these, but also contains an
   appropriate define for the type of hardware that you are targeting.  */

/* Define a symbol indicating that we are using svr4.h.  */
#define USING_SVR4_H

/* For the sake of libgcc2.c, indicate target supports atexit.  */
#define HAVE_ATEXIT

/* Cpp, assembler, linker, library, and startfile spec's.  */

/* Provide an ASM_SPEC appropriate for svr4.  Here we try to support as
   many of the specialized svr4 assembler options as seems reasonable,
   given that there are certain options which we can't (or shouldn't)
   support directly due to the fact that they conflict with other options
   for other svr4 tools (e.g. ld) or with other options for GCC itself.
   For example, we don't support the -o (output file) or -R (remove
   input file) options because GCC already handles these things.  We
   also don't support the -m (run m4) option for the assembler because
   that conflicts with the -m (produce load map) option of the svr4
   linker.  We do however allow passing arbitrary options to the svr4
   assembler via the -Wa, option.

   Note that gcc doesn't allow a space to follow -Y in a -Ym,* or -Yd,*
   option.  */

#undef  ASM_SPEC
#define ASM_SPEC \
  "%{v:-V} %{Qy:} %{!Qn:-Qy} %{n} %{T} %{Ym,*} %{Yd,*} %{Wa,*:%*}"

/* svr4 assemblers need the `-' (indicating input from stdin) to come after
   the -o option (and its argument) for some reason.  If we try to put it
   before the -o option, the assembler will try to read the file named as
   the output file in the -o option as an input file (after it has already
   written some stuff to it) and the binary stuff contained therein will
   cause totally confuse the assembler, resulting in many spurious error
   messages.  */

#undef  ASM_FINAL_SPEC
#define ASM_FINAL_SPEC "%|"

/* Under svr4, the normal location of the `ld' and `as' programs is the
   /usr/ccs/bin directory.  */

#ifndef CROSS_DIRECTORY_STRUCTURE
#undef  MD_EXEC_PREFIX
#define MD_EXEC_PREFIX "/usr/ccs/bin/"
#endif

/* Under svr4, the normal location of the various *crt*.o files is the
   /usr/ccs/lib directory.  */

#ifndef CROSS_DIRECTORY_STRUCTURE
#undef  MD_STARTFILE_PREFIX
#define MD_STARTFILE_PREFIX "/usr/ccs/lib/"
#endif

/* Provide a LIB_SPEC appropriate for svr4.  Here we tack on the default
   standard C library (unless we are building a shared library).  */

#undef	LIB_SPEC
#define LIB_SPEC "%{!shared:%{!symbolic:-lc}}"

/* Provide an ENDFILE_SPEC appropriate for svr4.  Here we tack on our own
   magical crtend.o file (see crtstuff.c) which provides part of the
   support for getting C++ file-scope static object constructed before
   entering `main', followed by the normal svr3/svr4 "finalizer" file,
   which is either `gcrtn.o' or `crtn.o'.  */

#undef  ENDFILE_SPEC
#define ENDFILE_SPEC "crtend.o%s %{pg:gcrtn.o%s}%{!pg:crtn.o%s}"


/* Attach a special .ident directive to the end of the file to identify
   the version of GCC which compiled this code.  The format of the
   .ident string is patterned after the ones produced by native svr4
   C compilers.  */

#define TARGET_ASM_FILE_END d10v_file_end
extern void d10v_file_end (void);

/* Use periods rather than dollar signs in special g++ assembler names.  */

#define NO_DOLLAR_IN_LABEL

/* Writing `int' for a bitfield forces int alignment for the structure.  */

#define PCC_BITFIELD_TYPE_MATTERS 1

/* All ELF targets can support DWARF-2.  */
#ifndef DWARF2_DEBUGGING_INFO
#define DWARF2_DEBUGGING_INFO
#endif

/* The numbers used to denote specific machine registers in the System V
   Release 4 DWARF debugging information are quite likely to be totally
   different from the numbers used in BSD stabs debugging information
   for the same kind of target machine.  Thus, we undefine the macro
   DBX_REGISTER_NUMBER here as an extra inducement to get people to
   provide proper machine-specific definitions of DBX_REGISTER_NUMBER
   (which is also used to provide DWARF registers numbers in dwarfout.c)
   in their tm.h files which include this file.  */

#undef DBX_REGISTER_NUMBER

/* Use DWARF debugging info by default.  */

#ifndef PREFERRED_DEBUGGING_TYPE
#define PREFERRED_DEBUGGING_TYPE DWARF_DEBUG
#endif

#define TARGET_ASM_FILE_START_FILE_DIRECTIVE true

/* This is how to allocate empty space in some section.  The .zero
   pseudo-op is used for this on most svr4 assemblers.  */

#undef  ASM_OUTPUT_SKIP
#define ASM_OUTPUT_SKIP(FILE,SIZE) \
  fprintf (FILE, "\t%s\t%u\n", SKIP_ASM_OP, (SIZE))

/* The prefix to add to user-visible assembler symbols.

   For System V Release 4 the convention is *not* to prepend a leading
   underscore onto user-level symbol names.  */

#undef  USER_LABEL_PREFIX
#define USER_LABEL_PREFIX ""

/* This is how to store into the string LABEL
   the symbol_ref name of an internal numbered label where
   PREFIX is the class of label and NUM is the number within the class.
   This is suitable for output with `assemble_name'.

   For most svr4 systems, the convention is that any symbol which begins
   with a period is not put into the linker symbol table by the assembler.  */

#undef  ASM_GENERATE_INTERNAL_LABEL
#define ASM_GENERATE_INTERNAL_LABEL(LABEL, PREFIX, NUM)		\
  do								\
    {								\
      sprintf (LABEL, "*.%s%d", PREFIX, (unsigned) (NUM));	\
    }								\
  while (0)


#ifndef ASM_OUTPUT_BEFORE_CASE_LABEL
#define ASM_OUTPUT_BEFORE_CASE_LABEL(FILE,PREFIX,NUM,TABLE) \
  ASM_OUTPUT_ALIGN ((FILE), 2);
#endif

#undef  ASM_OUTPUT_CASE_LABEL
#define ASM_OUTPUT_CASE_LABEL(FILE,PREFIX,NUM,JUMPTABLE)		\
  do									\
    {									\
      ASM_OUTPUT_BEFORE_CASE_LABEL (FILE, PREFIX, NUM, JUMPTABLE)	\
      targetm.asm_out.internal_label (FILE, PREFIX, NUM);		\
    }									\
  while (0)

/* The standard SVR4 assembler seems to require that certain builtin
   library routines (e.g. .udiv) be explicitly declared as .globl
   in each assembly file where they are referenced.  */

#undef  ASM_OUTPUT_EXTERNAL_LIBCALL
#define ASM_OUTPUT_EXTERNAL_LIBCALL(FILE, FUN)				\
  (*targetm.asm_out.globalize_label) (FILE, XSTR (FUN, 0))


#undef  ASM_OUTPUT_ALIGNED_COMMON
#define ASM_OUTPUT_ALIGNED_COMMON(FILE, NAME, SIZE, ALIGN)		\
  do									\
    {									\
      fprintf ((FILE), "\t%s\t", COMMON_ASM_OP);			\
      assemble_name ((FILE), (NAME));					\
      fprintf ((FILE), ",%u,%u\n", (SIZE), (ALIGN) / BITS_PER_UNIT);	\
    }									\
  while (0)

#undef  ASM_OUTPUT_ALIGNED_LOCAL
#define ASM_OUTPUT_ALIGNED_LOCAL(FILE, NAME, SIZE, ALIGN)	\
  do								\
    {								\
      fprintf ((FILE), "\t%s\t", LOCAL_ASM_OP);			\
      assemble_name ((FILE), (NAME));				\
      fprintf ((FILE), "\n");					\
      ASM_OUTPUT_ALIGNED_COMMON (FILE, NAME, SIZE, ALIGN);	\
    }								\
  while (0)

/* This is the pseudo-op used to generate a contiguous sequence of byte
   values from a double-quoted string WITHOUT HAVING A TERMINATING NUL
   AUTOMATICALLY APPENDED.  This is the same for most svr4 assemblers.  */

#undef  ASCII_DATA_ASM_OP
#define ASCII_DATA_ASM_OP	".ascii"


/* The following macro defines the format used to output the second
   operand of the .type assembler directive.  Different svr4 assemblers
   expect various different forms for this operand.  The one given here
   is just a default.  You may need to override it in your machine-
   specific tm.h file (depending upon the particulars of your assembler).  */

#define TYPE_OPERAND_FMT	"@%s"

/* Write the extra assembler code needed to declare a function's result.
   Most svr4 assemblers don't require any special declaration of the
   result value, but there are exceptions.  */

#ifndef ASM_DECLARE_RESULT
#define ASM_DECLARE_RESULT(FILE, RESULT)
#endif

/* These macros generate the special .type and .size directives which
   are used to set the corresponding fields of the linker symbol table
   entries in an ELF object file under SVR4.  These macros also output
   the starting labels for the relevant functions/objects.  */


/* A table of bytes codes used by the ASM_OUTPUT_ASCII and
   ASM_OUTPUT_LIMITED_STRING macros.  Each byte in the table
   corresponds to a particular byte value [0..255].  For any
   given byte value, if the value in the corresponding table
   position is zero, the given character can be output directly.
   If the table value is 1, the byte must be output as a \ooo
   octal escape.  If the tables value is anything else, then the
   byte value should be output as a \ followed by the value
   in the table.  Note that we can use standard UN*X escape
   sequences for many control characters, but we don't use
   \a to represent BEL because some svr4 assemblers (e.g. on
   the i386) don't know about that.  Also, we don't use \v
   since some versions of gas, such as 2.2 did not accept it.  */

#define ESCAPES \
"\1\1\1\1\1\1\1\1btn\1fr\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\
\0\0\"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\
\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\\\0\0\0\
\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\
\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\
\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\
\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\
\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"

/* Some svr4 assemblers have a limit on the number of characters which
   can appear in the operand of a .string directive.  If your assembler
   has such a limitation, you should define STRING_LIMIT to reflect that
   limit.  Note that at least some svr4 assemblers have a limit on the
   actual number of bytes in the double-quoted string, and that they
   count each character in an escape sequence as one byte.  Thus, an
   escape sequence like \377 would count as four bytes.

   If your target assembler doesn't support the .string directive, you
   should define this to zero.
*/

#define STRING_LIMIT	((unsigned) 256)


/* The routine used to output sequences of byte values.  We use a special
   version of this for most svr4 targets because doing so makes the
   generated assembly code more compact (and thus faster to assemble)
   as well as more readable.  Note that if we find subparts of the
   character sequence which end with NUL (and which are shorter than
   STRING_LIMIT) we output those using ASM_OUTPUT_LIMITED_STRING.  */

#undef  ASM_OUTPUT_ASCII
#define ASM_OUTPUT_ASCII(FILE, STR, LENGTH)				\
  do									\
    {									\
      unsigned char * _ascii_bytes = (unsigned char *) (STR);		\
      unsigned char * limit = _ascii_bytes + (LENGTH);			\
      unsigned        bytes_in_chunk = 0;				\
      for (; _ascii_bytes < limit; _ascii_bytes++)			\
        {								\
	  unsigned char * p;						\
	  if (bytes_in_chunk >= 60)					\
	    {								\
	      fprintf ((FILE), "\"\n");					\
	      bytes_in_chunk = 0;					\
	    }								\
	  for (p = _ascii_bytes; p < limit && *p != '\0'; p++)		\
	    continue;							\
	  if (p < limit && (p - _ascii_bytes) <= (long)STRING_LIMIT)	\
	    {								\
	      if (bytes_in_chunk > 0)					\
		{							\
		  fprintf ((FILE), "\"\n");				\
		  bytes_in_chunk = 0;					\
		}							\
	      ASM_OUTPUT_LIMITED_STRING ((FILE), _ascii_bytes);		\
	      _ascii_bytes = p;						\
	    }								\
	  else								\
	    {								\
	      int escape;						\
	      unsigned ch;						\
	      if (bytes_in_chunk == 0)					\
		fprintf ((FILE), "\t%s\t\"", ASCII_DATA_ASM_OP);	\
	      switch (escape = ESCAPES[ch = *_ascii_bytes])		\
		{							\
		case 0:							\
		  putc (ch, (FILE));					\
		  bytes_in_chunk++;					\
		  break;						\
		case 1:							\
		  fprintf ((FILE), "\\%03o", ch);			\
		  bytes_in_chunk += 4;					\
		  break;						\
		default:						\
		  putc ('\\', (FILE));					\
		  putc (escape, (FILE));				\
		  bytes_in_chunk += 2;					\
		  break;						\
		}							\
	    }								\
	}								\
      if (bytes_in_chunk > 0)						\
        fprintf ((FILE), "\"\n");					\
    }									\
  while (0)

/* All SVR4 targets use the ELF object file format.  */
#define OBJECT_FORMAT_ELF
