/* SH PE object file format target machined macro definitions.
   Copyright (C) 1999, 2001 Free Software Foundation, Inc.
   See ChangeLog for contributers.
 
   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* Switch to SECTION (an `enum in_section').

   This has to be customized for each section in the targets
   enum in_section.

   ??? 
   The problem is that we want to temporarily switch sections in
   ASM_DECLARE_OBJECT_NAME and then switch back to the original section
   afterwards.
   Elf supports assembler directives for pushing and popping sections.
   Standard elf supports popping one section, via the .previous directive.
   We could add something similar to the coff/pe assembler support.  This
   would avoid the need for the switch_to_section function.  The fact that
   this is an assembler feature for ELF is one reason why the compiler
   doesn't have it.  */
extern void		switch_to_section ();
#define SWITCH_TO_SECTION_FUNCTION 				\
void 								\
switch_to_section (section, decl) 				\
     enum in_section section; 					\
     tree decl; 						\
{ 								\
  switch (section) 						\
    { 								\
      case in_text: text_section (); break; 			\
      case in_data: data_section (); break; 			\
      case in_named: named_section (decl, NULL, 0); break; 	\
      case in_ctors: ctors_section (); break; 			\
      case in_dtors: dtors_section (); break; 			\
      case in_drectve: drectve_section (); break; 		\
      default: abort (); break; 				\
    } 								\
}

#define TARGET_DLLIMPORT_DECL_ATTRIBUTES 1

/* A C statement (sans semicolon) to output to the stdio stream
   STREAM any text necessary for declaring the name NAME of an
   initialized variable which is being defined.  This macro must
   output the label definition (perhaps using `ASM_OUTPUT_LABEL'). 
   The argument DECL is the `VAR_DECL' tree node representing the
   variable.

   If this macro is not defined, then the variable name is defined
   in the usual manner as a label (by means of `ASM_OUTPUT_LABEL').  */
#define ASM_DECLARE_OBJECT_NAME(STREAM, NAME, DECL)		\
  do								\
    {								\
      DLL_ATTR_DECLARE_OBJECT_NAME (STREAM, NAME, DECL);	\
      ASM_OUTPUT_LABEL (STREAM, NAME);		 		\
    }								\
  while (0)

/* MS docs say that stack space must be allocated by the
   caller for the callee to use to save R4..R7 in.  */
#define REG_PARM_STACK_SPACE(FNDECL) 16
#define OUTGOING_REG_PARM_STACK_SPACE 1

#ifndef CPP_DEFAULT_CPU_SPEC
#define CPP_DEFAULT_CPU_SPEC "-D__sh3__ -D_SH3_ %{!-ansi: -DSH3}"
#endif

#undef PE_SUBTARGET_CPP_SPEC
#define PE_SUBTARGET_CPP_SPEC " \
%{m3:-D_SH3_ %{!-ansi: -DSH3}} \
%{!-ansi:-DSHx} -Acpu(sh) -Amachine(sh) \
"
/* ???: Before the specs were rationalized, sh-pe didn't define
   __SIZE_TYPE__ and __PTRDIFF_TYPE__ in the specfile.  I don't know
   if that's right, but this undef preserves that behaviour.  */
#undef SUBTARGET_CPP_PTR_SPEC

#include "pe.h"
#include "sh/sh.h"

#undef  TARGET_DEFAULT
#define TARGET_DEFAULT	(LITTLE_ENDIAN_BIT | SH3_BIT | SH2_BIT)

/* Don't assume anything about the header files.  */
#define NO_IMPLICIT_EXTERN_C

#undef  CPP_PREDEFINES
#define CPP_PREDEFINES ""

#undef  STARTFILE_SPEC
#define STARTFILE_SPEC ""

#undef  LINK_SPEC
#define LINK_SPEC "%{mshared:-shared -e _DllMainCRTStartup} \
		   %{mdll:--dll -e _DllMainCRTStartup}"

/* A C statement (sans semicolon) to output to the stdio stream
   STREAM the assembler definition of a common-label named
   NAME whose size is SIZE bytes.  The variable ROUNDED
   is the size rounded up to whatever alignment the caller wants.  */
#undef ASM_OUTPUT_COMMON
#define ASM_OUTPUT_COMMON(FILE, NAME, SIZE, ROUNDED)	\
  DLL_ATTR_OUTPUT_COMMON (FILE, NAME, SIZE, ROUNDED)

/* DLL_ATTR_OUTPUT_COMMON uses this to output the name if necessary.
   In general it should be the same as ASM_OUTPUT_COMMON.  */
#define TARGET_DLL_ATTR_OUTPUT_COMMON(FILE, NAME, SIZE, ROUNDED)	\
  ( fputs ("\t.comm ", (FILE)),			\
    assemble_name ((FILE), (NAME)),		\
    fprintf ((FILE), ",%d\n", (SIZE)))
