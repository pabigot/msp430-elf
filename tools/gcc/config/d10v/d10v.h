/* Definitions of target machine for Mitsubishi D10V.
   Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2007, 2008, 2009, 2010  Free Software Foundation, Inc.
   Contributed by Cygnus Support and Red Hat.

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

/* Include generic ELF definitions.  */ 

#include "svr4-d10v.h"

#undef NEWPRO

/* D10v specific macros.  */

/* Align an address.  */
#define D10V_ALIGN(addr, align) (((addr) + (align) - 1) & ~((align) - 1))

/* Truncate and sign extend a value to 16 bits.  */
#define SIGN_EXTEND_SHORT(VALUE) ((int)((((VALUE) & 0xffff) ^ 0x8000) - 0x8000))


/* Driver configuration.  */

/* A C string constant that tells the GCC driver program options to
   pass to CPP.  It can also specify how to translate options you
   give to GCC into options for GCC to pass to the CPP.

   Do not define this macro if it does not need to do anything.  */
#define CPP_SPEC "\
%{mint32:  -D__INT__=32 -D__INT_MAX__=2147483647} \
%{!mint32: -D__INT__=16 -D__INT_MAX__=32767} \
%{mdouble64: -D__DOUBLE__=64} \
%{!mdouble64: -D__DOUBLE__=32}"

/* A C string constant that tells the GCC driver program options to
   pass to the assembler.  It can also specify how to translate
   options you give to GCC into options for GCC to pass to the
   assembler.  See the file `sun3.h' for an example of this.

   Do not define this macro if it does not need to do anything.  */
#undef	ASM_SPEC
#define ASM_SPEC "%{!mno-asm-optimize: %{O*: %{!O0: -O} %{O0: %{masm-optimize: -O}}}}"

/* Another C string constant used much like `LINK_SPEC'.  The
   difference between the two is that `LIB_SPEC' is used at the end
   of the command given to the linker.

   If this macro is not defined, a default is provided that loads the
   standard C library from the usual place.  See `gcc.c'.  */
#undef	LIB_SPEC
#define LIB_SPEC "%{msim: %e-msim is no longer supported} -lc"

/* Another C string constant used much like `LINK_SPEC'.  The
   difference between the two is that `STARTFILE_SPEC' is used at the
   very beginning of the command given to the linker.

   If this macro is not defined, a default is provided that loads the
   standard C startup file from the usual place.  See `gcc.c'.  */
#undef	STARTFILE_SPEC
#define STARTFILE_SPEC "crt0%O%s"

/* Another C string constant used much like `LINK_SPEC'.  The
   difference between the two is that `ENDFILE_SPEC' is used at the
   very end of the command given to the linker.

   Do not define this macro if it does not need to do anything.  */
#undef	ENDFILE_SPEC
#define ENDFILE_SPEC ""

/* Define this macro as a C expression for the initializer of an
   array of string to tell the driver program which options are
   defaults for this target and thus do not need to be handled
   specially when using `MULTILIB_OPTIONS'.

   Do not define this macro if `MULTILIB_OPTIONS' is not defined in
   the target makefile fragment or if none of the options listed in
   `MULTILIB_OPTIONS' are set by default.  */
#define MULTILIB_DEFAULTS { "mint16", "mdouble32" }


/* Run-time target specifications.  */
#define TARGET_CPU_CPP_BUILTINS() 		\
  do						\
    {						\
      builtin_define ("__D10V__");		\
      builtin_assert ("machine=d10v");		\
    }						\
  while (0)

#undef  TARGET_DEFAULT
#define TARGET_DEFAULT	MASK_SMALL_INSNS | MASK_COND_MOVE | MASK_LONGLONG_ALU

#define TARGET_NO_D10V_CALLEE_COPIES  (! TARGET_D10V_CALLEE_COPIES)
#define TARGET_NO_SMALL_INSNS         (! TARGET_SMALL_INSNS)
#define TARGET_NO_COND_MOVE           (! TARGET_COND_MOVE)
#define TARGET_NO_LONGLONG_ALU        (! TARGET_LONGLONG_ALU)

/* This macro is a C statement to print on `stderr' a string
   describing the particular machine description choice.  Every
   machine description should define `TARGET_VERSION'.  */
#define TARGET_VERSION fprintf (stderr, " (D10V)")


/* Storage layout.  */

/* Define this macro to have the value 1 if the most significant bit
   in a byte has the lowest number; otherwise define it to have the
   value zero.  This means that bit-field instructions count from the
   most significant bit.  If the machine has no bit-field
   instructions, then this must still be defined, but it doesn't
   matter which value it is defined to.  This macro need not be a
   constant.

   This macro does not affect the way structure fields are packed into
   bytes or words; that is controlled by `BYTES_BIG_ENDIAN'.  */
#define BITS_BIG_ENDIAN 1

/* Define this macro to have the value 1 if the most significant byte
   in a word has the lowest number.  This macro need not be a
   constant.  */
#define BYTES_BIG_ENDIAN 1

/* Define this macro to have the value 1 if, in a multiword object,
   the most significant word has the lowest number.  This applies to
   both memory locations and registers; GCC fundamentally assumes
   that the order of words in memory is the same as the order in
   registers.  This macro need not be a constant.  */
#define WORDS_BIG_ENDIAN 1

/* Number of storage units in a word; normally 4.  */
#define UNITS_PER_WORD 2

/* Normal alignment required for function parameters on the stack, in
   bits.  All stack parameters receive at least this much alignment
   regardless of data type.  On most machines, this is the same as the
   size of an integer. */
#define PARM_BOUNDARY 16

/* Define this macro if you wish to preserve a certain alignment for
   the stack pointer.  The definition is a C expression for the
   desired alignment (measured in bits).

   If `PUSH_ROUNDING' is not defined, the stack will always be aligned
   to the specified boundary.  If `PUSH_ROUNDING' is defined and
   specifies a less strict alignment than `STACK_BOUNDARY', the stack
   may be momentarily unaligned while pushing arguments. */
#define STACK_BOUNDARY 16

/* Alignment required for a function entry point, in bits. */
#define FUNCTION_BOUNDARY 32

/* Biggest alignment that any data type can require on this machine,
   in bits. */
#define BIGGEST_ALIGNMENT 16

/* Define this macro to be the value 1 if instructions will fail to
   work if given data not on the nominal alignment.  If instructions
   will merely go slower in that case, define this macro as 0. */
#define STRICT_ALIGNMENT 1

/* Layout of Source Language Data Types.  */

/* A C expression whose value is nonzero if pointers that need to be
   extended from being `POINTER_SIZE' bits wide to `Pmode' are
   sign-extended and zero if they are zero-extended.

   You need not define this macro if the `POINTER_SIZE' is equal to
   the width of `Pmode'.  */
#define POINTERS_EXTEND_UNSIGNED 1

/* A C expression for the size in bits of the type `int' on the
   target machine.  If you don't define this, the default is one word.  */
#define INT_TYPE_SIZE (TARGET_INT32 ? 32 : 16)

/* A C expression for the size in bits of the type `short' on the
   target machine.  If you don't define this, the default is half a
   word.  (If this would be less than one storage unit, it is rounded
   up to one unit.)  */
#define SHORT_TYPE_SIZE 16

/* A C expression for the size in bits of the type `long' on the
   target machine.  If you don't define this, the default is one word.  */
#define LONG_TYPE_SIZE 32

/* A C expression for the size in bits of the type `long long' on the
   target machine.  If you don't define this, the default is two
   words.  If you want to support GNU Ada on your machine, the value
   of macro must be at least 64.  */
#define LONG_LONG_TYPE_SIZE 64

/* A C expression for the size in bits of the type `char' on the
   target machine.  If you don't define this, the default is one
   quarter of a word.  (If this would be less than one storage unit,
   it is rounded up to one unit.)  */
#define CHAR_TYPE_SIZE 8

/* A C expression for the size in bits of the type `float' on the
   target machine.  If you don't define this, the default is one word. */
#define FLOAT_TYPE_SIZE 32

/* A C expression for the size in bits of the type `double' on the
   target machine.  If you don't define this, the default is two
   words.  */
#define DOUBLE_TYPE_SIZE ((TARGET_DOUBLE64) ? 64 : 32)

/* A C expression for the size in bits of the type `long double' on
   the target machine.  If you don't define this, the default is two
   words.  */
#define LONG_DOUBLE_TYPE_SIZE 64

/* An expression whose value is 1 or 0, according to whether the type
   `char' should be signed or unsigned by default.  The user can
   always override this default with the options `-fsigned-char' and
   `-funsigned-char'.  */
#define DEFAULT_SIGNED_CHAR 1

/* A C expression for a string describing the name of the data type
   to use for size values.  The typedef name `size_t' is defined
   using the contents of the string.

   The string can contain more than one keyword.  If so, separate
   them with spaces, and write first any length keyword, then
   `unsigned' if appropriate, and finally `int'.  The string must
   exactly match one of the data type names defined in the function
   `init_decl_processing' in the file `c-decl.c'.  You may not omit
   `int' or change the order--that would cause the compiler to crash
   on startup.

   If you don't define this macro, the default is `"long unsigned
   int"'.

   On the D10V, ints can be either 16 or 32 bits, so we need to adjust
   size_t appropriately.  */
#undef	SIZE_TYPE
#define SIZE_TYPE "short unsigned int"

/* A C expression for a string describing the name of the data type
   to use for the result of subtracting two pointers.  The typedef
   name `ptrdiff_t' is defined using the contents of the string.  See
   `SIZE_TYPE' above for more information.

   If you don't define this macro, the default is `"long int"'.  */
#undef	PTRDIFF_TYPE
#define PTRDIFF_TYPE "short int"

/* A C expression for a string describing the name of the data type
   to use for wide characters.  The typedef name `wchar_t' is defined
   using the contents of the string.  See `SIZE_TYPE' above for more
   information.

   If you don't define this macro, the default is `"int"'.  */
#undef	WCHAR_TYPE
#define WCHAR_TYPE "short unsigned int"

/* A C expression for the size in bits of the data type for wide
   characters.  This is used in `cpp', which cannot make use of
   `WCHAR_TYPE'.  */
#undef	WCHAR_TYPE_SIZE
#define WCHAR_TYPE_SIZE 16


/* Basic Characteristics of Registers.  */

/* Return true if a value is inside a range.  */
#define IN_RANGE_P(VALUE, LOW, HIGH) \
  (((unsigned)((VALUE) - (LOW))) <= ((unsigned)((HIGH) - (LOW))))

/* First/last general purpose registers.  */
#define GPR_FIRST	0
#define GPR_LAST	15
#define GPR_ZERO_REGNUM	(GPR_FIRST + 14)	/* register that holds 0 */
#define GPR_P(REGNO)	(IN_RANGE_P (REGNO, GPR_FIRST, GPR_LAST) || REGNO == AP_FIRST)
#define GPR_OR_PSEUDO_P(REGNO) (GPR_P (REGNO) || REGNO >= FIRST_PSEUDO_REGISTER)

/* Registers arguments are passed in.  */
#define ARG_FIRST	(GPR_FIRST + 0)
#define ARG_LAST	(GPR_FIRST + 3)
#define RETURN_REGNUM	ARG_FIRST

/* Register pair to save accumulators in.  */
#define SAVE_ACC_REGNUM	(GPR_FIRST + 12)

/* Register to save accumulator guard digits in.  */
#define SAVE_GUARD_REGNUM (GPR_FIRST + 5)

/* Even/odd registers for multiword items.  */
#define GPR_EVEN_P(REGNO) (GPR_P(REGNO) && ((((REGNO) - GPR_FIRST) & 1) == 0))
#define GPR_ODD_P(REGNO)  (GPR_P(REGNO) && ((((REGNO) - GPR_FIRST) & 1) != 0))

/* First/last control registers.  */
#define CR_FIRST	16
#define CR_LAST		31
#define CR_P(REGNO)	IN_RANGE_P (REGNO, CR_FIRST, CR_LAST)

#define REPEAT_REGNUM	(CR_FIRST + 7)

/* First/last accumulator registers.  */
#define ACCUM_FIRST	32
#define ACCUM_LAST	33
#define ACCUM_P(REGNO)	IN_RANGE_P (REGNO, ACCUM_FIRST, ACCUM_LAST)

/* Argument pointer pseudo register.  */
#define AP_FIRST	34

/* Condition/carry code 'registers'.  We ignore the fact that these
   are actually stored in CR0.  */
#define CC_FIRST	35
#define CC_LAST		37
#define CC_P(REGNO)	IN_RANGE_P (REGNO, CC_FIRST, CC_LAST)

#define F0_REGNUM	(CC_FIRST + 0)
#define F1_REGNUM	(CC_FIRST + 1)
#define CARRY_REGNUM	(CC_FIRST + 2)

/* Number of hardware registers known to the compiler.  They receive
   numbers 0 through `FIRST_PSEUDO_REGISTER-1'; thus, the first
   pseudo register's number really is assigned the number
   `FIRST_PSEUDO_REGISTER'.  */
#define FIRST_PSEUDO_REGISTER 38

/* An initializer that says which registers are used for fixed
   purposes all throughout the compiled code and are therefore not
   available for general allocation.  These would include the stack
   pointer, the frame pointer (except on machines where that can be
   used as a general register when no frame pointer is needed), the
   program counter on machines where that is considered one of the
   addressable registers, and any other numbered register with a
   standard use.

   This information is expressed as a sequence of numbers, separated
   by commas and surrounded by braces.  The Nth number is 1 if
   register N is fixed, 0 otherwise.

   The table initialized from this macro, and the table initialized by
   the following one, may be overridden at run time either
   automatically, by the actions of the macro
   `CONDITIONAL_REGISTER_USAGE', or by the user with the command
   options `-ffixed-REG', `-fcall-used-REG' and `-fcall-saved-REG'.  */
#define FIXED_REGISTERS							\
{									\
  0, /* r0   */	0, /* r1   */	0, /* r2   */	0, /* r3   */		\
  0, /* r4   */	0, /* r5   */	0, /* r6   */	0, /* r7   */		\
  0, /* r8   */	0, /* r9   */	0, /* r10  */	0, /* r11  */		\
  0, /* r12  */	0, /* r13  */	1, /* r14  */	1, /* r15  */		\
  1, /* cr0  */	1, /* cr1  */	1, /* cr2  */	1, /* cr3  */		\
  1, /* cr4  */	1, /* cr5  */	1, /* cr6  */	1, /* cr7  */		\
  1, /* cr8  */	1, /* cr9  */	1, /* cr10 */	1, /* cr11 */		\
  1, /* cr12 */	1, /* cr13 */	1, /* cr14 */	1, /* cr15 */		\
  1, /* a0   */	1, /* a1   */						\
  1, /* ap   */								\
  1, /* f0   */	1, /* f1   */	1, /* c    */				\
}

/* Like `FIXED_REGISTERS' but has 1 for each register that is
   clobbered (in general) by function calls as well as for fixed
   registers.  This macro therefore identifies the registers that are
   not available for general allocation of values that must live
   across function calls.

   If a register has 0 in `CALL_USED_REGISTERS', the compiler
   automatically saves it on function entry and restores it on
   function exit, if the register is used within the function.  */
#define CALL_USED_REGISTERS						\
{									\
  1, /* r0   */	1, /* r1   */	1, /* r2   */	1, /* r3   */		\
  1, /* r4   */ 1, /* r5   */	0, /* r6   */	0, /* r7   */		\
  0, /* r8   */	0, /* r9   */	0, /* r10  */	0, /* r11  */		\
  1, /* r12  */	1, /* r13  */	1, /* r14  */	1, /* r15  */		\
  1, /* cr0  */	1, /* cr1  */	1, /* cr2  */	1, /* cr3  */		\
  1, /* cr4  */	1, /* cr5  */	1, /* cr6  */	1, /* cr7  */		\
  1, /* cr8  */	1, /* cr9  */	1, /* cr10 */	1, /* cr11 */		\
  1, /* cr12 */	1, /* cr13 */	1, /* cr14 */	1, /* cr15 */		\
  1, /* a0   */	1, /* a1   */						\
  1, /* ap   */								\
  1, /* f0   */	1, /* f1   */	1, /* c    */				\
}

/* Zero or more C statements that may conditionally modify two
   variables `fixed_regs' and `call_used_regs' (both of type `char
   []') after they have been initialized from the two preceding
   macros.

   This is necessary in case the fixed or call-clobbered registers
   depend on target flags.  */
#define CONDITIONAL_REGISTER_USAGE			\
  do							\
    {							\
      if (TARGET_ACCUM)					\
        {						\
	  int i;					\
							\
	  for (i = ACCUM_FIRST; i <= ACCUM_LAST; i++)	\
	    fixed_regs[i] = call_used_regs[i] = 0;	\
        }						\
    }							\
  while (0)


/* Order of allocating registers.  */

/* If defined, an initializer for a vector of integers, containing the
   numbers of hard registers in the order in which GCC should
   prefer to use them (from most preferred to least).

   If this macro is not defined, registers are used lowest numbered
   first (all else being equal).

   One use of this macro is on machines where the highest numbered
   registers must always be saved and the save-multiple-registers
   instruction supports only sequences of consecutive registers.  On
   such machines, define `REG_ALLOC_ORDER' to be an initializer that
   lists the highest numbered allocatable register first.

   On the d10v, we save some time by putting the fixed registers at
   the end of the list.  Also, we put the odd registers whose even
   register is fixed before the even/odd register pairs.  */
#define REG_ALLOC_ORDER							\
{									\
  /* Volatile GPR registers */						\
  GPR_FIRST+0,								\
  GPR_FIRST+1,								\
  GPR_FIRST+2,								\
  GPR_FIRST+3,								\
  GPR_FIRST+4,								\
  GPR_FIRST+5,								\
  GPR_FIRST+12,								\
  GPR_FIRST+13,								\
									\
  /* Callee preserved GPR registers */					\
  GPR_FIRST+6,								\
  GPR_FIRST+7,								\
  GPR_FIRST+8,								\
  GPR_FIRST+9,								\
  GPR_FIRST+10,								\
  GPR_FIRST+11,								\
									\
  /* Accumulators */							\
  ACCUM_FIRST+0,							\
  ACCUM_FIRST+1,							\
									\
  /* Repeat count register */						\
  REPEAT_REGNUM,							\
									\
  /* Condition code, carry registers */					\
  F0_REGNUM,								\
  F1_REGNUM,								\
  CARRY_REGNUM,								\
									\
  /* Fixed registers */							\
  GPR_FIRST+14,			/* zero register */			\
  GPR_FIRST+15,			/* stack pointer */			\
  CR_FIRST+0,								\
  CR_FIRST+1,								\
  CR_FIRST+2,								\
  CR_FIRST+3,								\
  CR_FIRST+4,								\
  CR_FIRST+5,								\
  CR_FIRST+6,								\
  CR_FIRST+8,								\
  CR_FIRST+9,								\
  CR_FIRST+10,								\
  CR_FIRST+11,								\
  CR_FIRST+12,								\
  CR_FIRST+13,								\
  CR_FIRST+14,								\
  CR_FIRST+15,								\
  ARG_POINTER_REGNUM,							\
}


/* How values fit in registers.  */

/* A C expression for the number of consecutive hard registers,
   starting at register number REGNO, required to hold a value of mode
   MODE.  */

#define HARD_REGNO_NREGS(REGNO, MODE)					\
  ((ACCUM_P (REGNO))							\
   ? ((GET_MODE_SIZE (MODE) + 4 - 1) / 4)				\
   : ((GET_MODE_SIZE (MODE) + 2 - 1) / 2))

/* A C expression that is nonzero if it is permissible to store a
   value of mode MODE in hard register number REGNO (or in several
   registers starting with that one).  */

extern unsigned char hard_regno_mode_ok[][FIRST_PSEUDO_REGISTER];
#define HARD_REGNO_MODE_OK(REGNO, MODE) hard_regno_mode_ok[ (int)MODE ][ REGNO ]

/* A C expression that is nonzero if it is desirable to choose
   register allocation so as to avoid move instructions between a
   value of mode MODE1 and a value of mode MODE2.

   If `HARD_REGNO_MODE_OK (R, MODE1)' and `HARD_REGNO_MODE_OK (R,
   MODE2)' are ever different for any R, then `MODES_TIEABLE_P (MODE1,
   MODE2)' must be zero.  */

extern unsigned char modes_tieable_p[];
#define MODES_TIEABLE_P(MODE1, MODE2) \
  modes_tieable_p[ (((int)(MODE1)) * (NUM_MACHINE_MODES)) + (int)(MODE2) ]

/* Define this macro if the compiler should avoid copies to/from CCmode
   registers.  You should only define this macro if support fo copying to/from
   CCmode is incomplete.  */
   
#define AVOID_CCMODE_COPIES


/* Register classes.  */

/* An enumeral type that must be defined with all the register class
   names as enumeral values.  `NO_REGS' must be first.  `ALL_REGS'
   must be the last register class, followed by one more enumeral
   value, `LIM_REG_CLASSES', which is not a register class but rather
   tells how many classes there are.

   Each register class has a number, which is the value of casting
   the class name to type `int'.  The number serves as an index in
   many of the tables described below.

   All things being equal, the register allocator prefers the higher
   register class, so put all of the general registers higher than
   the accumulators and CRs.  */

enum reg_class
{
  NO_REGS,
  REPEAT_REGS,
  CR_REGS,
  ACCUM_REGS,
  F0_REGS,
  F1_REGS,
  F_REGS,
  CARRY_REGS,
  CC_REGS,
  ARG0_REGS,
  ARG1_REGS,
  ARG2_REGS,
  ARG3_REGS,
  RETURN_REGS,
  EVEN_REGS,
  GENERAL_REGS,
  ALL_REGS,
  LIM_REG_CLASSES
};

/* The number of distinct register classes.  */
#define N_REG_CLASSES ((int) LIM_REG_CLASSES)

/* An initializer containing the names of the register classes as C
   string constants.  These names are used in writing some of the
   debugging dumps.  */
#define REG_CLASS_NAMES							\
{									\
  "NO_REGS",								\
  "REPEAT_REGS",							\
  "CR_REGS",								\
  "ACCUM_REGS",								\
  "F0_REGS",								\
  "F1_REGS",								\
  "F_REGS",								\
  "CARRY_REGS",								\
  "CC_REGS",								\
  "ARG0_REGS",								\
  "ARG1_REGS",								\
  "ARG2_REGS",								\
  "ARG3_REGS",								\
  "RETURN_REGS",							\
  "EVEN_REGS",								\
  "GENERAL_REGS",							\
  "ALL_REGS"								\
}

/* An initializer containing the contents of the register classes, as
   integers which are bit masks.  The Nth integer specifies the
   contents of class N.  The way the integer MASK is interpreted is
   that register R is in the class if `MASK & (1 << R)' is 1.

   When the machine has more than 32 registers, an integer does not
   suffice.  Then the integers are replaced by sub-initializers,
   braced groupings containing several integers.  Each
   sub-initializer must be suitable as an initializer for the type
   `HARD_REG_SET' which is defined in `hard-reg-set.h'.  */
#define REG_CLASS_CONTENTS						\
{									\
  { 0x00000000, 0x00000000 },	/* NO_REGS */				\
  { 0x00800000, 0x00000000 },	/* REPEAT_REGS */			\
  { 0xffff0000, 0x00000000 },	/* CR_REGS */				\
  { 0x00000000, 0x00000003 },	/* ACCUM_REGS */			\
  { 0x00000000, 0x00000008 },	/* F0_REGS */				\
  { 0x00000000, 0x00000010 },	/* F1_REGS */				\
  { 0x00000000, 0x00000018 },	/* F_REGS */				\
  { 0x00000000, 0x00000020 },	/* CARRY_REGS */			\
  { 0x00000000, 0x00000038 },	/* CC_REGS */				\
  { 0x00000001, 0x00000000 },	/* ARG0_REGS */				\
  { 0x00000002, 0x00000000 },	/* ARG1_REGS */				\
  { 0x00000004, 0x00000000 },	/* ARG2_REGS */				\
  { 0x00000008, 0x00000000 },	/* ARG3_REGS */				\
  { 0x00002000, 0x00000000 },	/* R13_REGS */				\
  { 0x0000ffff, 0x00000000 },	/* EVEN_REGS */				\
  { 0x0000ffff, 0x00000004 },	/* GENERAL_REGS */			\
  { 0xffffffff, 0x0000001f },	/* ALL_REGS */				\
}

/* A C expression whose value is a register class containing hard
   register REGNO.  In general there is more than one such class;
   choose a class which is "minimal", meaning that no smaller class
   also contains the register.  */

extern enum reg_class regno_reg_class[];
#define REGNO_REG_CLASS(REGNO) regno_reg_class[ (REGNO) ]

/* A macro whose definition is the name of the class to which a valid
   base register must belong.  A base register is one used in an
   address which is the register value plus a displacement.  */
#define BASE_REG_CLASS GENERAL_REGS

/* A macro whose definition is the name of the class to which a valid
   index register must belong.  An index register is one used in an
   address where its value is either multiplied by a scale factor or
   added to another register (as well as added to a displacement).  */
#define INDEX_REG_CLASS GENERAL_REGS

/* A C expression which defines the machine-dependent operand
   constraint letters for register classes.  If CHAR is such a
   letter, the value should be the register class corresponding to
   it.  Otherwise, the value should be `NO_REGS'.  The register
   letter `r', corresponding to class `GENERAL_REGS', will not be
   passed to this macro; you do not need to handle it.  */

extern enum reg_class reg_class_from_letter[];
#define REG_CLASS_FROM_LETTER(CHAR) reg_class_from_letter[ CHAR ]

/* A C expression which is nonzero if register number NUM is suitable
   for use as a base register in operand addresses.  It may be either
   a suitable hard register or a pseudo register that has been
   allocated such a hard register.  */

/* These assume that REGNO is a hard or pseudo reg number.
   They give nonzero only if REGNO is a hard reg of the suitable class
   or a pseudo reg currently allocated to a suitable hard reg.
   Since they use reg_renumber, they are safe only once reg_renumber
   has been allocated, which happens in local-alloc.c.  */

#define REGNO_OK_FOR_BASE_P(REGNO)					\
  ((REGNO) < FIRST_PSEUDO_REGISTER					\
   ? GPR_P (REGNO) || (REGNO) == ARG_POINTER_REGNUM			\
   : (reg_renumber[REGNO] >= 0 && GPR_P (reg_renumber[REGNO])))

/* A C expression which is nonzero if register number NUM is suitable
   for use as an index register in operand addresses.  It may be
   either a suitable hard register or a pseudo register that has been
   allocated such a hard register.

   The difference between an index register and a base register is
   that the index register may be scaled.  If an address involves the
   sum of two registers, neither one of them scaled, then either one
   may be labeled the "base" and the other the "index"; but whichever
   labeling is used must fit the machine's constraints of which
   registers may serve in each capacity.  The compiler will try both
   labelings, looking for one that is valid, and will reload one or
   both registers only if neither labeling works.  */
#define REGNO_OK_FOR_INDEX_P(REGNO) REGNO_OK_FOR_BASE_P (REGNO)

/* A C expression that places additional restrictions on the register
   class to use when it is necessary to copy value X into a register
   in class CLASS.  The value is a register class; perhaps CLASS, or
   perhaps another, smaller class.  On many machines, the following
   definition is safe:

        #define PREFERRED_RELOAD_CLASS(X,CLASS) CLASS

   Sometimes returning a more restrictive class makes better code.
   For example, on the 68000, when X is an integer constant that is
   in range for a `moveq' instruction, the value of this macro is
   always `DATA_REGS' as long as CLASS includes the data registers.
   Requiring a data register guarantees that a `moveq' will be used.

   If X is a `const_double', by returning `NO_REGS' you can force X
   into a memory constant.  This is useful on certain machines where
   immediate floating values cannot be loaded into certain kinds of
   registers. */
#define PREFERRED_RELOAD_CLASS(X, CLASS) \
  (((CLASS) == EVEN_REGS || (CLASS) == ACCUM_REGS) \
   ? (TARGET_ACCUM ? (CLASS) : EVEN_REGS) : GENERAL_REGS)

/* Many machines have some registers that cannot be copied directly
   to or from memory or even from other types of registers.  An
   example is the `MQ' register, which on most machines, can only be
   copied to or from general registers, but not memory.  Some
   machines allow copying all registers to and from memory, but
   require a scratch register for stores to some memory locations
   (e.g., those with symbolic address on the RT, and those with
   certain symbolic address on the Sparc when compiling PIC).  In
   some cases, both an intermediate and a scratch register are
   required.

   You should define these macros to indicate to the reload phase
   that it may need to allocate at least one register for a reload in
   addition to the register to contain the data.  Specifically, if
   copying X to a register CLASS in MODE requires an intermediate
   register, you should define `SECONDARY_INPUT_RELOAD_CLASS' to
   return the largest register class all of whose registers can be
   used as intermediate registers or scratch registers.

   If copying a register CLASS in MODE to X requires an intermediate
   or scratch register, `SECONDARY_OUTPUT_RELOAD_CLASS' should be
   defined to return the largest register class required.  If the
   requirements for input and output reloads are the same, the macro
   `SECONDARY_RELOAD_CLASS' should be used instead of defining both
   macros identically.

   The values returned by these macros are often `GENERAL_REGS'.
   Return `NO_REGS' if no spare register is needed; i.e., if X can be
   directly copied to or from a register of CLASS in MODE without
   requiring a scratch register.  Do not define this macro if it
   would always return `NO_REGS'.

   If a scratch register is required (either with or without an
   intermediate register), you should define patterns for
   `reload_inM' or `reload_outM', as required.  These patterns,
   which will normally be implemented with a `define_expand',
   should be similar to the `movM' patterns, except that
   operand 2 is the scratch register.

   Define constraints for the reload register and scratch register
   that contain a single register class.  If the original reload
   register (whose class is CLASS) can meet the constraint given in
   the pattern, the value returned by these macros is used for the
   class of the scratch register.  Otherwise, two additional reload
   registers are required.  Their classes are obtained from the
   constraints in the insn pattern.

   X might be a pseudo-register or a `subreg' of a pseudo-register,
   which could either be in a hard register or in memory.  Use
   `true_regnum' to find out; it will return -1 if the pseudo is in
   memory and the hard register number if it is in a register.

   These macros should not be used in the case where a particular
   class of registers can only be copied to memory and not to another
   class of registers.  In that case, secondary reload registers are
   not needed and would not be helpful.  Instead, a stack location
   must be used to perform the copy and the `movM' pattern should use
   memory as a intermediate storage.  This case often occurs between
   floating-point and general registers. */
#define SECONDARY_RELOAD_CLASS(CLASS, MODE, X)				\
 ((CLASS) == ACCUM_REGS ? 	EVEN_REGS				\
  : ((CLASS) == GENERAL_REGS || (CLASS) == EVEN_REGS) ? NO_REGS		\
  : (CLASS) == F0_REGS ?	GENERAL_REGS				\
  : 			  	GENERAL_REGS)

/* Normally the compiler avoids choosing registers that have been
   explicitly mentioned in the rtl as spill registers (these
   registers are normally those used to pass parameters and return
   values).  However, some machines have so few registers of certain
   classes that there would not be enough registers to use as spill
   registers if this were done.

   Define `SMALL_REGISTER_CLASSES' on these machines.  When it is
   defined, the compiler allows registers explicitly used in the rtl
   to be used as spill registers but avoids extending the lifetime of
   these registers.

   It is always safe to define this macro, but if you unnecessarily
   define it, you will reduce the amount of optimizations that can be
   performed in some cases.  If you do not define this macro when it
   is required, the compiler will run out of spill registers and
   print a fatal error message.  For most machines, you should not
   define this macro.  */
#define SMALL_REGISTER_CLASSES 1

/* A C expression for the maximum number of consecutive registers of
   class CLASS needed to hold a value of mode MODE.

   This is closely related to the macro `HARD_REGNO_NREGS'.  In fact,
   the value of the macro `CLASS_MAX_NREGS (CLASS, MODE)' should be
   the maximum value of `HARD_REGNO_NREGS (REGNO, MODE)' for all
   REGNO values in the class CLASS.

   This macro helps control the handling of multiple-word values in
   the reload pass.  */
#define CLASS_MAX_NREGS(CLASS, MODE)					\
  (((CLASS) == ACCUM_REGS)						\
  ? ((GET_MODE_SIZE (MODE) + 4 - 1) / 4)				\
  : ((GET_MODE_SIZE (MODE) + 2 - 1) / 2))

/* A C expression that defines the machine-dependent operand
   constraint letters that specify particular ranges of integer
   values.  If C is one of those letters, the expression should check
   that VALUE, an integer, is in the appropriate range and return 1
   if so, 0 otherwise.  If C is not one of those letters, the value
   should be 0 regardless of VALUE.

   'I' is for 4-bit unsigned constants (imm4), note bit pattern 0 == 16.
   'J' is for ~ (1 << n), for n <= 15
   'K' is for 32-bit constants with the lower 8 bits in the range -8..7.
   'L' is for 4-bit signed constants (imm4).
   'M' is for 16-bit non-negative constants.
   'N' is for 4-bit negative unsigned constants (imm4), note bit pattern 0 == 16.
   'O' is zero.
   'P' is for (1 << n), where n <= 15.  */

#define CONST_OK_FOR_LETTER_P(VALUE, C)					\
  ((C) == 'I'   ? IN_RANGE_P (VALUE, 1, 16)				\
 : (C) == 'J' ? IN_RANGE_P (exact_log2 (~ (VALUE)), 0, 15)		\
 : (C) == 'K' ? IN_RANGE_P (SIGN_EXTEND_SHORT (VALUE), -8, 7)		\
 : (C) == 'L' ? IN_RANGE_P (VALUE, -8, 7)				\
 : (C) == 'M' ? IN_RANGE_P (VALUE, 0, 32767)				\
 : (C) == 'N' ? IN_RANGE_P (VALUE, -16, -1)				\
 : (C) == 'O' ? ((VALUE) == 0)						\
 : (C) == 'P' ? IN_RANGE_P (exact_log2 (VALUE), 0, 15)			\
 : 0)

/* A C expression that defines the machine-dependent operand
   constraint letters that specify particular ranges of
   `const_double' values.

   If C is one of those letters, the expression should check that
   VALUE, an RTX of code `const_double', is in the appropriate range
   and return 1 if so, 0 otherwise.  If C is not one of those
   letters, the value should be 0 regardless of VALUE.

   `const_double' is used for all floating-point constants and for
   `DImode' fixed-point constants.  A given letter can accept either
   or both kinds of values.  It can use `GET_MODE' to distinguish
   between these kinds.  */

#define CONST_DOUBLE_OK_FOR_LETTER_P(VALUE, C) \
  ((C) == 'G' ? (CONST_DOUBLE_LOW (VALUE) == 0 && CONST_DOUBLE_HIGH (VALUE) == 0) \
  : 0)

/* A C expression that defines the optional machine-dependent
     constraint letters that can be used to segregate specific types of
     operands, usually memory references, for the target machine.
     Normally this macro will not be defined.  If it is required for a
     particular target machine, it should return 1 if VALUE corresponds
     to the operand type represented by the constraint letter C.  If C
     is not defined as an extra constraint, the value returned should
     be 0 regardless of VALUE.

     'Q' is for memory references that are short.  */

#define EXTRA_CONSTRAINT(VALUE, C)					\
  ((C) == 'Q'	? short_memory_operand (VALUE, GET_MODE (VALUE))	\
   : (C) == 'R'   ? GET_CODE (XEXP (VALUE, 0)) == REG			\
   : (C) == 'S'   ? CONSTANT_ADDRESS_P (XEXP (VALUE, 0))		\
   :		  0)


/* Stack layout.  */

/* Structure used to define the d10v stack.  */
typedef struct d10v_stack
{
  int varargs_p;		/* whether this is a varargs function */
  int varargs_size;		/* size to hold varargs args passed in regs */
  int vars_size;		/* variable save area size */
  int parm_size;		/* outgoing parameter size */
  int gpr_size;			/* size of saved GPR registers */
  int accum_size;		/* size of saved ACCUM registers */
  int total_size;		/* total bytes allocated for stack */
				/* which registers are to be saved */
  unsigned char save_p[FIRST_PSEUDO_REGISTER];
} d10v_stack_t;

/* Define this macro if pushing a word onto the stack moves the stack
   pointer to a smaller address.

   When we say, "define this macro if ...," it means that the
   compiler checks this macro only with `#ifdef' so the precise
   definition used does not matter.  */

#define STACK_GROWS_DOWNWARD 1

/* Offset from the frame pointer to the first local variable slot to
   be allocated.

   If `FRAME_GROWS_DOWNWARD', find the next slot's offset by
   subtracting the first slot's length from `STARTING_FRAME_OFFSET'.
   Otherwise, it is found by adding the length of the first slot to
   the value `STARTING_FRAME_OFFSET'.

   On the D10V, the frame pointer is the same as the stack pointer,
   except for dynamic allocations.  So we start after the outgoing
   parameter area.  */

#define STARTING_FRAME_OFFSET						\
  (D10V_ALIGN (crtl->outgoing_args_size, 2))

/* Offset from the argument pointer register to the first argument's
   address.  On some machines it may depend on the data type of the
   function.

   If `ARGS_GROW_DOWNWARD', this is the offset to the location above
   the first argument's address.  */
#define FIRST_PARM_OFFSET(FUNDECL) 0

/* A C expression whose value is RTL representing the value of the return
   address for the frame COUNT steps up from the current frame, after the
   prologue.  FRAMEADDR is the frame pointer of the COUNT frame, or the frame
   pointer of the COUNT - 1 frame if `RETURN_ADDR_IN_PREVIOUS_FRAME' is
   defined.
   
   The value of the expression must always be the correct address when COUNT is
   zero, but may be `NULL_RTX' if there is not way to determine the return
   address of other frames.  */
#define RETURN_ADDR_RTX(COUNT, FRAME)					\
  ((COUNT) == 0 ?  gen_rtx_REG (Pmode, RETURN_ADDRESS_REGNUM ) : NULL_RTX)

/* Stack based registers.  */

/* The register number of the stack pointer register, which must also
   be a fixed register according to `FIXED_REGISTERS'.  On most
   machines, the hardware determines which register this is.  */
#define STACK_POINTER_REGNUM (GPR_FIRST + 15)

/* The register number of the frame pointer register, which is used to
   access automatic variables in the stack frame.  On some machines,
   the hardware determines which register this is.  On other
   machines, you can choose any register you wish for this purpose.  */
#define FRAME_POINTER_REGNUM (GPR_FIRST + 11)

/* The register number of the arg pointer register, which is used to
   access the function's argument list.  On some machines, this is
   the same as the frame pointer register.  On some machines, the
   hardware determines which register this is.  On other machines,
   you can choose any register you wish for this purpose.  If this is
   not the same register as the frame pointer register, then you must
   mark it as a fixed register according to `FIXED_REGISTERS', or
   arrange to be able to eliminate it.  */
#define ARG_POINTER_REGNUM AP_FIRST

/* Register numbers used for passing a function's static chain
   pointer.

   The static chain register need not be a fixed register.

   If the static chain is passed in memory, these macros should not be
   defined; instead, the next two macros should be defined.  */
#define STATIC_CHAIN_REGNUM (GPR_FIRST + 4)

/* Local d10v return link register number.  */
#define RETURN_ADDRESS_REGNUM (GPR_FIRST + 13)


/* Eliminating the frame and arg pointers.  */

/* If defined, this macro specifies a table of register pairs used to
   eliminate unneeded registers that point into the stack frame.  If
   it is not defined, the only elimination attempted by the compiler
   is to replace references to the frame pointer with references to
   the stack pointer.

   The definition of this macro is a list of structure
   initializations, each of which specifies an original and
   replacement register.

   On some machines, the position of the argument pointer is not
   known until the compilation is completed.  In such a case, a
   separate hard register must be used for the argument pointer.
   This register can be eliminated by replacing it with either the
   frame pointer or the argument pointer, depending on whether or not
   the frame pointer has been eliminated.

   In this case, you might specify:
        #define ELIMINABLE_REGS  \
        {{ARG_POINTER_REGNUM, STACK_POINTER_REGNUM}, \
         {ARG_POINTER_REGNUM, FRAME_POINTER_REGNUM}, \
         {FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM}}

   Note that the elimination of the argument pointer with the stack
   pointer is specified first since that is the preferred elimination.  */

#define ELIMINABLE_REGS							\
{{ FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM },			\
 { ARG_POINTER_REGNUM,	 STACK_POINTER_REGNUM },			\
 { ARG_POINTER_REGNUM,   FRAME_POINTER_REGNUM }}			\

/* This macro is similar to `INITIAL_FRAME_POINTER_OFFSET'.  It
   specifies the initial difference between the specified pair of
   registers.  This macro must be defined if `ELIMINABLE_REGS' is
   defined.  */

#define INITIAL_ELIMINATION_OFFSET(FROM, TO, OFFSET)			\
{									\
  d10v_stack_t *info = d10v_stack_info ();				\
									\
 if ((FROM) == FRAME_POINTER_REGNUM && (TO) == STACK_POINTER_REGNUM)	\
   (OFFSET) = 0;							\
 else if ((FROM) == ARG_POINTER_REGNUM && (TO) == FRAME_POINTER_REGNUM)	\
   (OFFSET) = info->total_size;						\
 else if ((FROM) == ARG_POINTER_REGNUM && (TO) == STACK_POINTER_REGNUM)	\
   (OFFSET) = info->total_size;						\
  else									\
    gcc_unreachable ();							\
}


/* Passing arguments.  */

#define ACCUMULATE_OUTGOING_ARGS 1

#define OUTGOING_REG_PARM_STACK_SPACE(FNTYPE) 1


/* Function arguments.  */

/* A C expression that controls whether a function argument is passed
   in a register, and which register.

   The arguments are CUM, which summarizes all the previous
   arguments; MODE, the machine mode of the argument; TYPE, the data
   type of the argument as a tree node or 0 if that is not known
   (which happens for C support library functions); and NAMED, which
   is 1 for an ordinary argument and 0 for nameless arguments that
   correspond to `...' in the called function's prototype.

   The value of the expression should either be a `reg' RTX for the
   hard register in which to pass the argument, or zero to pass the
   argument on the stack.

   For machines like the Vax and 68000, where normally all arguments
   are pushed, zero suffices as a definition.  */

#define FUNCTION_ARG(CUM, MODE, TYPE, NAMED) \
  function_arg (& (CUM), MODE, TYPE, NAMED)

/* If defined, a C expression that indicates when it is the called
   function's responsibility to make a copy of arguments passed by
   invisible reference.  Normally, the caller makes a copy and passes
   the address of the copy to the routine being called.  When
   FUNCTION_ARG_CALLEE_COPIES is defined and is nonzero, the caller
   does not make a copy.  Instead, it passes a pointer to the "live"
   value.  The called function must not modify this value.  If it can
   be determined that the value won't be modified, it need not make a
   copy; otherwise a copy must be made.  */
#define FUNCTION_ARG_CALLEE_COPIES(CUM, MODE, TYPE, NAMED) \
  function_arg_callee_copies (& (CUM), MODE, TYPE, NAMED)

/* A C type for declaring a variable that is used as the first
   argument of `FUNCTION_ARG' and other related values.  For some
   target machines, the type `int' suffices and can hold the number
   of bytes of argument so far.

   There is no need to record in `CUMULATIVE_ARGS' anything about the
   arguments that have been passed on the stack.  The compiler has
   other variables to keep track of that.  For target machines on
   which all arguments are passed on the stack, there is no need to
   store anything in `CUMULATIVE_ARGS'; however, the data structure
   must exist and should not be empty, so use `int'.  */

typedef struct
{
  int reg, stack, argcount;
}
d10v_cumulative_args;

#define CUMULATIVE_ARGS d10v_cumulative_args

/* A C statement (sans semicolon) for initializing the variable CUM
   for the state at the beginning of the argument list.  The variable
   has type `CUMULATIVE_ARGS'.  The value of FNTYPE is the tree node
   for the data type of the function which will receive the args, or 0
   if the args are to a compiler support library function.  The value
   of INDIRECT is nonzero when processing an indirect call, for
   example a call through a function pointer.  The value of INDIRECT
   is zero for a call to an explicitly named function, a library
   function call, or when `INIT_CUMULATIVE_ARGS' is used to find
   arguments for the function being compiled.

   When processing a call to a compiler support library function,
   LIBNAME identifies which one.  It is a `symbol_ref' rtx which
   contains the name of the function, as a string.  LIBNAME is 0 when
   an ordinary C function call is being processed.  Thus, each time
   this macro is called, either LIBNAME or FNTYPE is nonzero, but
   never both of them at once.  */
#define INIT_CUMULATIVE_ARGS(CUM, FNTYPE, LIBNAME, FNDECL, N_NAMED_ARGS) \
  init_cumulative_args (& (CUM), FNTYPE, LIBNAME, FNDECL, FALSE)

/* Like `INIT_CUMULATIVE_ARGS' but overrides it for the purposes of
   finding the arguments for the function being compiled.  If this
   macro is undefined, `INIT_CUMULATIVE_ARGS' is used instead.

   The value passed for LIBNAME is always 0, since library routines
   with special calling conventions are never compiled with GCC.
   The argument LIBNAME exists for symmetry with
   `INIT_CUMULATIVE_ARGS'.  */
#define INIT_CUMULATIVE_INCOMING_ARGS(CUM, FNTYPE, LIBNAME) \
  init_cumulative_args (& (CUM), FNTYPE, LIBNAME, NULL, TRUE)

/* A C statement (sans semicolon) to update the summarizer variable
   CUM to advance past an argument in the argument list.  The values
   MODE, TYPE and NAMED describe that argument.  Once this is done,
   the variable CUM is suitable for analyzing the *following*
   argument with `FUNCTION_ARG', etc.

   This macro need not do anything if the argument in question was
   passed on the stack.  The compiler knows how to track the amount
   of stack space used for arguments without any special help.  */
#define FUNCTION_ARG_ADVANCE(CUM, MODE, TYPE, NAMED) \
   function_arg_advance (& (CUM), MODE, TYPE, NAMED)

/* If defined, a C expression that gives the alignment boundary, in
   bits, of an argument with the specified mode and type.  If it is
   not defined, `PARM_BOUNDARY' is used for all arguments.  */
#define FUNCTION_ARG_BOUNDARY(MODE, TYPE) \
  function_arg_boundary (MODE, TYPE)

/* A C expression that is nonzero if REGNO is the number of a hard
   register in which function arguments are sometimes passed.  This
   does *not* include implicit arguments such as the static chain and
   the structure-value address.  On many machines, no registers can be
   used for this purpose since all function arguments are pushed on
   the stack.  */
#define FUNCTION_ARG_REGNO_P(REGNO) IN_RANGE_P (REGNO, ARG_FIRST, ARG_LAST)


/* How values are returned.  */

/* A C expression to create an RTX representing the place where a
   function returns a value of data type VALTYPE.  VALTYPE is a tree
   node representing a data type.  Write `TYPE_MODE (VALTYPE)' to get
   the machine mode used to represent that type.  On many machines,
   only the mode is relevant.  (Actually, on most machines, scalar
   values are returned in the same place regardless of mode).

   If `PROMOTE_FUNCTION_RETURN' is defined, you must apply the same
   promotion rules specified in `PROMOTE_MODE' if VALTYPE is a scalar
   type.

   If the precise function being called is known, FUNC is a tree node
   (`FUNCTION_DECL') for it; otherwise, FUNC is a null pointer.  This
   makes it possible to use a different value-returning convention
   for specific functions when all their calls are known.

   `FUNCTION_VALUE' is not used for return vales with aggregate data
   types, because these are returned in another way.  See
   `STRUCT_VALUE_REGNUM' and related macros, below.  */
#define FUNCTION_VALUE(VALTYPE, FUNC) \
  gen_rtx_REG (TYPE_MODE (VALTYPE), RETURN_REGNUM)

/* A C expression to create an RTX representing the place where a
   library function returns a value of mode MODE.  If the precise
   function being called is known, FUNC is a tree node
   (`FUNCTION_DECL') for it; otherwise, FUNC is a null pointer.  This
   makes it possible to use a different value-returning convention
   for specific functions when all their calls are known.

   Note that "library function" in this context means a compiler
   support routine, used to perform arithmetic, whose name is known
   specially by the compiler and was not mentioned in the C code being
   compiled.

   The definition of `LIBRARY_VALUE' need not be concerned aggregate
   data types, because none of the library functions returns such
   types.  */
#define LIBCALL_VALUE(MODE) gen_rtx_REG (MODE, RETURN_REGNUM)

/* A C expression that is nonzero if REGNO is the number of a hard
   register in which the values of called function may come back.

   A register whose use for returning values is limited to serving as
   the second of a pair (for a value of type `double', say) need not
   be recognized by this macro.  So for most machines, this definition
   suffices:

        #define FUNCTION_VALUE_REGNO_P(N) ((N) == 0)

   If the machine has register windows, so that the caller and the
   called function use different registers for the return value, this
   macro should recognize only the caller's register numbers.  */
#define FUNCTION_VALUE_REGNO_P(REGNO) ((REGNO) == RETURN_REGNUM)

/* Define this macro to be 1 if all structure and union return values
   must be in memory.  Since this results in slower code, this should
   be defined only if needed for compatibility with other compilers
   or with an ABI.  If you define this macro to be 0, then the
   conventions used for structure and union return values are decided
   by the `RETURN_IN_MEMORY' macro.

   If not defined, this defaults to the value 1.  */
#define DEFAULT_PCC_STRUCT_RETURN 0 

/* If the structure value address is not passed in a register, define
   `STRUCT_VALUE' as an expression returning an RTX for the place
   where the address is passed.  If it returns 0, the address is
   passed as an "invisible" first argument.  */
#define STRUCT_VALUE 0


/* Profiling.  */

/* A C statement or compound statement to output to FILE some
   assembler code to call the profiling subroutine `mcount'.  */
#define FUNCTION_PROFILER(FILE, LABELNO) gcc_unreachable ()


/* Trampolines.  */

/* A C expression for the size in bytes of the trampoline, as an
   integer. */
#define TRAMPOLINE_SIZE 8

/* Alignment required for trampolines, in bits.  */
#define TRAMPOLINE_ALIGNMENT 32

/* Address modes.  */

#define HAVE_POST_INCREMENT 1
#define HAVE_POST_DECREMENT 1
#define HAVE_PRE_DECREMENT 1
/* #define HAVE_PRE_INCREMENT 0 */

/* A C expression that is 1 if the RTX X is a constant which is a
   valid address.  */
#define CONSTANT_ADDRESS_P(X) CONSTANT_P(X)

/* A number, the maximum number of registers that can appear in a
   valid memory address.  Note that it is up to you to specify a
   value equal to the maximum number that `GO_IF_LEGITIMATE_ADDRESS'
   would ever accept.  */
#define MAX_REGS_PER_ADDRESS 1

#ifdef	REG_OK_STRICT
#define REG_OK_STRICT_P 1
#else
#define REG_OK_STRICT_P 0
#endif

/* A C expression that is nonzero if X (assumed to be a `reg' RTX) is
   valid for use as a base register.  For hard registers, it should
   always accept those which the hardware permits and reject the
   others.  Whether the macro accepts or rejects pseudo registers
   must be controlled by `REG_OK_STRICT' as described above.  This
   usually requires two variant definitions, of which `REG_OK_STRICT'
   controls the one actually used.  */
#ifdef REG_OK_STRICT
#define REG_OK_FOR_BASE_P(X) GPR_P (REGNO (X))
#else
#define REG_OK_FOR_BASE_P(X)						\
 (GPR_P (REGNO (X))							\
  || REGNO (X) == ARG_POINTER_REGNUM					\
  || REGNO (X) >= FIRST_PSEUDO_REGISTER)
#endif

/* A C expression that is nonzero if X (assumed to be a `reg' RTX) is
   valid for use as an index register.

   The difference between an index register and a base register is
   that the index register may be scaled.  If an address involves the
   sum of two registers, neither one of them scaled, then either one
   may be labeled the "base" and the other the "index"; but whichever
   labeling is used must fit the machine's constraints of which
   registers may serve in each capacity.  The compiler will try both
   labelings, looking for one that is valid, and will reload one or
   both registers only if neither labeling works.  */
#define REG_OK_FOR_INDEX_P(X) REG_OK_FOR_BASE_P(X)

#define GO_IF_MODE_DEPENDENT_ADDRESS(ADDR, LABEL)

/* A C expression that is nonzero if X is a legitimate constant for
   an immediate operand on the target machine.  You can assume that X
   satisfies `CONSTANT_P', so you need not check this.  In fact, `1'
   is a suitable definition for this macro on machines where anything
   `CONSTANT_P' is valid.  */
#define LEGITIMATE_CONSTANT_P(X) 1


/* Condition codes.  */

/* Returns a mode from class `MODE_CC' to be used when comparison
   operation code OP is applied to rtx X and Y.  */
#define SELECT_CC_MODE(OP, X, Y) (d10v_emit_comparison ((OP), (X), (Y), \
  NULL_RTX, NULL_RTX, GET_MODE (X), FALSE))

/* Describing Relative Costs of Operations.  */

/* A C expression for the cost of a branch instruction.  A value of 1
   is the default; other values are interpreted relative to that.  */
#define BRANCH_COST(speed,predictable) d10v_branch_cost

/* A C expression for the maximum number of instructions to execute via
   conditional execution instructions instead of a branch.  A value of
   BRANCH_COST+1 is the default if the machine does not use cc0, and 1 if it
   does use cc0.  */
#define MAX_CONDITIONAL_EXECUTE d10v_cond_exec

/* Define this macro as a C expression which is nonzero if accessing
   less than a word of memory (i.e. a `char' or a `short') is no
   faster than accessing a word of memory, i.e., if such access
   require more than one instruction or if there is no difference in
   cost between byte and (aligned) word loads.

   When this macro is not defined, the compiler will access a field by
   finding the smallest containing object; when it is defined, a
   fullword load will be used if alignment permits.  Unless bytes
   accesses are faster than word accesses, using word accesses is
   preferable since it may eliminate subsequent memory access if
   subsequent accesses occur to other fields in the same word of the
   structure, but to different bytes.  */
#define SLOW_BYTE_ACCESS 0

/* The number of scalar move insns which should be generated instead
   of a string move insn or a library call.  Increasing the value
   will always make code faster, but eventually incurs high cost in
   increased code size.

   If you don't define this, a reasonable default is used.  */
#define MOVE_RATIO(speed)  4

/* Define this macro if it is as good or better to call a constant
   function address than to call an address kept in a register.  */
#define NO_FUNCTION_CSE

/* Dividing the Output into Sections (Texts, Data, ...).  */

/* A C expression whose value is a string containing the assembler
   operation that should precede instructions and read-only data.
   Normally `".text"' is right.  */
#define TEXT_SECTION_ASM_OP "\t.text"

/* A C expression whose value is a string containing the assembler
   operation to identify the following data as writable initialized
   data.  Normally `".data"' is right.  */
#define DATA_SECTION_ASM_OP "\t.data"

/* If defined, a C expression whose value is a string containing the
   assembler operation to identify the following data as
   uninitialized global data.  If not defined, and neither
   `ASM_OUTPUT_BSS' nor `ASM_OUTPUT_ALIGNED_BSS' are defined,
   uninitialized global data will be output in the data section if
   `-fno-common' is passed, otherwise `ASM_OUTPUT_COMMON' will be
   used.  */
#ifndef BSS_SECTION_ASM_OP
#define BSS_SECTION_ASM_OP ".section\t.bss"
#endif

/* The Overall Framework of an Assembler File.  */

/* A C string constant describing how to begin a comment in the target
   assembler language.  The compiler assumes that the comment will
   end at the end of the line.  */
#define ASM_COMMENT_START ";"

/* A C string constant for text to be output before each `asm'
   statement or group of consecutive ones.  Normally this is
   `"#APP"', which is a comment that has no effect on most assemblers
   but tells the GNU assembler that it must check the lines that
   follow for all valid assembler constructs.  */
#define ASM_APP_ON ";APP_ON\n"

/* A C string constant for text to be output after each `asm'
   statement or group of consecutive ones.  Normally this is
   `"#NO_APP"', which tells the GNU assembler to resume making the
   time-saving assumptions that are valid for ordinary compiler
   output.  */
#define ASM_APP_OFF ";APP_OFF\n"


/* Output of Data.  */

#define ASM_OUTPUT_DOUBLE(FILE, VALUE)					\
  {									\
    if (REAL_VALUE_ISINF (VALUE)					\
        || REAL_VALUE_ISNAN (VALUE)					\
	|| REAL_VALUE_MINUS_ZERO (VALUE))				\
      {									\
	long t[2];							\
	REAL_VALUE_TO_TARGET_DOUBLE ((VALUE), t);			\
	fprintf (FILE, "\t.long 0x%lx\n\t.long 0x%lx\n",		\
		t[0] & 0xffffffff, t[1] & 0xffffffff);			\
      }									\
    else								\
      {									\
	char str[30];							\
	REAL_VALUE_TO_DECIMAL (VALUE, "%.20e", str);			\
	fprintf (FILE, "\t.double 0d%s\n", str);			\
      }									\
  }

#define ASM_OUTPUT_FLOAT(FILE, VALUE)					\
  {									\
    if (REAL_VALUE_ISINF (VALUE)					\
        || REAL_VALUE_ISNAN (VALUE)					\
	|| REAL_VALUE_MINUS_ZERO (VALUE))				\
      {									\
	long t;								\
	REAL_VALUE_TO_TARGET_SINGLE ((VALUE), t);			\
	fprintf (FILE, "\t.long 0x%lx\n", t & 0xffffffff);		\
      }									\
    else								\
      {									\
	char str[30];							\
	REAL_VALUE_TO_DECIMAL ((VALUE), "%.20e", str);			\
	fprintf (FILE, "\t.float 0d%s\n", str);				\
      }									\
  }

#define ASM_OUTPUT_INT(STREAM, EXP)		\
  do						\
    {						\
      fputs ("\t.long ", STREAM);		\
      output_addr_const (STREAM, EXP);		\
      putc ('\n', STREAM);			\
    }						\
  while (0)

/* Use d10v_output_addr_const, not output_addr_const for HI values
   to insure proper truncation and sign extension of negative offsets,
   which sometimes get converted to unsigned HI values.  */
#define ASM_OUTPUT_SHORT(STREAM, EXP)					\
  do									\
    {									\
      fputs ("\t.word ", STREAM);					\
      if (GET_CODE (EXP) == LABEL_REF)					\
        {								\
          d10v_output_addr_const (STREAM, EXP);				\
          fputs ("@word", STREAM);					\
        }								\
      else if (GET_CODE (EXP) != SYMBOL_REF)				\
        d10v_output_addr_const (STREAM, EXP);				\
      else								\
        {								\
          tree id = get_identifier (XSTR (EXP, 0));			\
									\
          if (TREE_SIDE_EFFECTS (id))					\
	    fprintf (STREAM, "%s@word", XSTR (EXP, 0));			\
          else								\
	    d10v_output_addr_const (STREAM, EXP);			\
        }								\
									\
      putc ('\n', STREAM);						\
    }									\
  while (0)

#define ASM_OUTPUT_CHAR(STREAM, EXP)		\
  do						\
    {						\
      fputs ("\t.byte ", STREAM);		\
      output_addr_const (STREAM, EXP);		\
      putc ('\n', STREAM);			\
    }						\
  while (0)

/* A C statement to output to the stdio stream STREAM an assembler
   instruction to assemble a single byte containing the number VALUE.  */
#define ASM_OUTPUT_BYTE(STREAM, VALUE) \
  fprintf (STREAM, "\t%s %d\n", ASM_BYTE_OP, VALUE)

/* Define this macro as a C expression which is nonzero if C is used as a
   logical line separator by the assembler.

   If you do not define this macro, the default is that only the character `;'
   is treated as a logical line separator.  */
#define IS_ASM_LOGICAL_LINE_SEPARATOR(C, STR) ((C) == '|')


/* Like `ASM_OUTPUT_BSS' except takes the required alignment as a
   separate, explicit argument.  If you define this macro, it is used
   in place of `ASM_OUTPUT_BSS', and gives you more flexibility in
   handling the required alignment of the variable.  The alignment is
   specified as the number of bits.

   Try to use function `asm_output_aligned_bss' defined in file
   `varasm.c' when defining this macro. */
#ifndef ASM_OUTPUT_ALIGNED_BSS
#define ASM_OUTPUT_ALIGNED_BSS(FILE, DECL, NAME, SIZE, ALIGNMENT) \
  asm_output_aligned_bss (FILE, DECL, NAME, SIZE, ALIGNMENT)
#endif


/* Output and Generation of Labels.  */

/* Globalizing directive for a label.  */
#define GLOBAL_ASM_OP "\t.globl "

/* A C statement (sans semicolon) to output on STREAM an assembler
   pseudo-op to declare a library function name external.  The name
   of the library function is given by SYMREF, which has type `rtx'
   and is a `symbol_ref'.

   This macro need not be defined if it does not need to output
   anything.  The GNU assembler and most Unix assemblers don't
   require anything.  */

#undef ASM_OUTPUT_EXTERNAL_LIBCALL

/* Output of Assembler Instructions.  */

/* A C initializer containing the assembler's names for the machine
   registers, each one as a C string constant.  This is what
   translates register numbers in the compiler into assembler
   language.  */

#define REGISTER_NAMES							\
{									\
  "r0",   "r1",   "r2",   "r3",   "r4",   "r5",   "r6",   "r7",		\
  "r8",   "r9",   "r10",  "r11",  "r12",  "r13",  "r14",  "sp",		\
  "cr0",  "cr1",  "cr2",  "cr3",  "cr4",  "cr5",  "cr6",  "cr7",	\
  "cr8",  "cr9",  "cr10", "cr11", "cr12", "cr13", "cr14", "cr15",	\
  "a0",   "a1",								\
  "ap",									\
  "f0",   "f1",   "c",							\
}

/* If defined, a C initializer for an array of structures containing
   a name and a register number.  This macro defines additional names
   for hard registers, thus allowing the `asm' option in declarations
   to refer to registers using alternate names.  */
#define ADDITIONAL_REGISTER_NAMES					\
{									\
  { "r15",	STACK_POINTER_REGNUM },					\
  { "repeat",	REPEAT_REGNUM },					\
  { "carry",	CARRY_REGNUM },						\
}

/* A C compound statement to output to stdio stream STREAM the
   assembler syntax for an instruction operand X.  X is an RTL
   expression.

   CODE is a value that can be used to specify one of several ways of
   printing the operand.  It is used when identical operands must be
   printed differently depending on the context.  CODE comes from the
   `%' specification that was used to request printing of the
   operand.  If the specification was just `%DIGIT' then CODE is 0;
   if the specification was `%LTR DIGIT' then CODE is the ASCII code
   for LTR.

   If X is a register, this macro should print the register's name.
   The names can be found in an array `reg_names' whose type is `char
   *[]'.  `reg_names' is initialized from `REGISTER_NAMES'.

   When the machine description has a specification `%PUNCT' (a `%'
   followed by a punctuation character), this macro is called with a
   null pointer for X and the punctuation character for CODE.  */

#define PRINT_OPERAND(STREAM, X, CODE) print_operand (STREAM, X, CODE)

/* A C expression which evaluates to true if CODE is a valid
     punctuation character for use in the `PRINT_OPERAND' macro.  If
     `PRINT_OPERAND_PUNCT_VALID_P' is not defined, it means that no
     punctuation characters (except for the standard one, `%') are used
     in this way.  */

#define PRINT_OPERAND_PUNCT_VALID_P(CODE) ((CODE) == '|' || (CODE) == '.')

/* A C compound statement to output to stdio stream STREAM the
   assembler syntax for an instruction operand that is a memory
   reference whose address is X.  X is an RTL expression.

   On some machines, the syntax for a symbolic address depends on the
   section that the address refers to.  On these machines, define the
   macro `ENCODE_SECTION_INFO' to store the information into the
   `symbol_ref', and then check for it here.  */

#define PRINT_OPERAND_ADDRESS(STREAM, X) print_operand_address (STREAM, X)

/* A C expression to output to STREAM some assembler code which will
   push hard register number REGNO onto the stack.  The code need not
   be optimal, since this macro is used only when profiling.  */

#define ASM_OUTPUT_REG_PUSH(STREAM, REGNO)				\
  fprintf (STREAM, "\tsubi %s,-2\n\tst %s,@-%s\n",			\
	   reg_names[STACK_POINTER_REGNUM],				\
	   reg_names[REGNO],						\
	   reg_names[STACK_POINTER_REGNUM])

/* A C expression to output to STREAM some assembler code which will
   pop hard register number REGNO off of the stack.  The code need
   not be optimal, since this macro is used only when profiling.  */

#define ASM_OUTPUT_REG_POP(STREAM, REGNO)				\
  fprintf (STREAM, "\tld %s,@%s+\n\taddi %s,2\n",			\
	   reg_names[REGNO],						\
	   reg_names[STACK_POINTER_REGNUM],				\
	   reg_names[STACK_POINTER_REGNUM])


/* Output of Dispatch Tables.  */

/* A C statement to output to the stdio stream STREAM an assembler
   pseudo-instruction to generate a difference between two labels.
   vALUE and REL are the numbers of two internal labels.  The
   definitions of these labels are output using
   `targetm.asm_out.internal_label', and they must be printed in the same
   way here.  For example,

          fprintf (STREAM, "\t.word L%d-L%d\n",
                   VALUE, REL)

     You must provide this macro on machines where the addresses in a
     dispatch table are relative to the table's own address.  If
     defined, GCC will also use this macro on all machines when
     producing PIC.  */
#define ASM_OUTPUT_ADDR_DIFF_ELT(STREAM, BODY, VALUE, REL) \
  fprintf (STREAM, "\t.word .L%d@word-.L%d@word\n", VALUE, REL)

/* This macro should be provided on machines where the addresses in a
   dispatch table are absolute.

   The definition should be a C statement to output to the stdio
   stream STREAM an assembler pseudo-instruction to generate a
   reference to a label.  VALUE is the number of an internal label
   whose definition is output using `targetm.asm_out.internal_label'.  */
#define ASM_OUTPUT_ADDR_VEC_ELT(STREAM, VALUE) \
  fprintf (STREAM, "\t.word .L%d@word\n", VALUE)


/* Assembler Commands for Exception Regions.  */

/* Don't use __builtin_setjmp for unwinding, since it's tricky to get
   at the high 16 bits of an address.  */
#define DONT_USE_BUILTIN_SETJMP 
#define JMP_BUF_SIZE  14 


/* Assembler Commands for Alignment.  */

/* A C statement to output to the stdio stream STREAM an assembler
   command to advance the location counter to a multiple of 2 to the
   POWER bytes.  POWER will be a C expression of type `int'.  */

#define ASM_OUTPUT_ALIGN(STREAM, POWER)			\
  do							\
    {							\
      if ((POWER) != 0)					\
        fprintf (STREAM, "\t.align %d\n", (POWER));	\
    }							\
  while (0)


/* Macros Affecting All Debugging Formats.  */

/* A C expression that returns the DBX register number for the
   compiler register number REGNO.  In simple cases, the value of this
   expression may be REGNO itself.  But sometimes there are some
   registers that the compiler knows about and DBX does not, or vice
   versa.  In such cases, some register may need to have one number in
   the compiler and another for DBX.

   If two registers have consecutive numbers inside GCC, and they
   can be used as a pair to hold a multiword value, then they *must*
   have consecutive numbers after renumbering with
   `DBX_REGISTER_NUMBER'.  Otherwise, debuggers will be unable to
   access such a pair, because they expect register pairs to be
   consecutive in their own numbering scheme.

   If you find yourself defining `DBX_REGISTER_NUMBER' in way that
   does not preserve register pairs, then what you must do instead is
   redefine the actual register numbering scheme.  */
#define DBX_REGISTER_NUMBER(REGNO) (REGNO)

/* A C expression that returns the type of debugging output GCC
   produces when the user specifies `-g' or `-ggdb'.  Define this if
   you have arranged for GCC to support more than one format of
   debugging output.  Currently, the allowable values are `DBX_DEBUG',
   `SDB_DEBUG', `DWARF_DEBUG', and `XCOFF_DEBUG'.

   The value of this macro only affects the default debugging output;
   the user can always get a specific type of output by using
   `-gstabs', `-gcoff', `-gdwarf', or `-gxcoff'.  */
#undef	PREFERRED_DEBUGGING_TYPE
#define PREFERRED_DEBUGGING_TYPE DBX_DEBUG

/* Disable Dwarf2 right now, since it wants a fixed size for DOUBLE and INT.  */
#undef DWARF2_DEBUGGING_INFO


/* Miscellaneous Parameters.  */

/* An alias for a machine mode name.  This is the machine mode that
   elements of a jump-table should have.  */
#define CASE_VECTOR_MODE Pmode

/* Define this macro if operations between registers with integral
   mode smaller than a word are always performed on the entire
   register.  Most RISC machines have this property and most CISC
   machines do not.  */
#define WORD_REGISTER_OPERATIONS 1

/* Define this macro to be a C expression indicating when insns that
   read memory in MODE, an integral mode narrower than a word, set the
   bits outside of MODE to be either the sign-extension or the
   zero-extension of the data read.  Return `SIGN_EXTEND' for values
   of MODE for which the insn sign-extends, `ZERO_EXTEND' for which
   it zero-extends, and `NIL' for other modes.

   This macro is not called with MODE non-integral or with a width
   greater than or equal to `BITS_PER_WORD', so you may return any
   value in this case.  Do not define this macro if it would always
   return `NIL'.  On machines where this macro is defined, you will
   normally define it as the constant `SIGN_EXTEND' or `ZERO_EXTEND'.  */
#define LOAD_EXTEND_OP(MODE) (((MODE) == QImode) ? ZERO_EXTEND : UNKNOWN)

/* The maximum number of bytes that a single instruction can move
   quickly from memory to memory.  */
#define MOVE_MAX 4

/* A C expression which is nonzero if on this machine it is safe to
   "convert" an integer of INPREC bits to one of OUTPREC bits (where
   OUTPREC is smaller than INPREC) by merely operating on it as if it
   had only OUTPREC bits.

   On many machines, this expression can be 1.

   When `TRULY_NOOP_TRUNCATION' returns 1 for a pair of sizes for
   modes for which `MODES_TIEABLE_P' is 0, suboptimal code can result.
   If this is the case, making `TRULY_NOOP_TRUNCATION' return 0 in
   such cases may improve things.  */
#define TRULY_NOOP_TRUNCATION(OUTPREC, INPREC) 1

/* A C expression describing the value returned by a comparison
   operator with an integral mode and stored by a store-flag
   instruction (`sCOND') when the condition is true.  This
   description must apply to *all* the `sCOND' patterns and all the
   comparison operators whose results have a `MODE_INT' mode.

   A value of 1 or -1 means that the instruction implementing the
   comparison operator returns exactly 1 or -1 when the comparison is
   true and 0 when the comparison is false.  Otherwise, the value
   indicates which bits of the result are guaranteed to be 1 when the
   comparison is true.  This value is interpreted in the mode of the
   comparison operation, which is given by the mode of the first
   operand in the `sCOND' pattern.  Either the low bit or the sign
   bit of `STORE_FLAG_VALUE' be on.  Presently, only those bits are
   used by the compiler.

   If `STORE_FLAG_VALUE' is neither 1 or -1, the compiler will
   generate code that depends only on the specified bits.  It can also
   replace comparison operators with equivalent operations if they
   cause the required bits to be set, even if the remaining bits are
   undefined.  For example, on a machine whose comparison operators
   return an `SImode' value and where `STORE_FLAG_VALUE' is defined as
   `0x80000000', saying that just the sign bit is relevant, the
   expression

          (ne:SI (and:SI X (const_int POWER-OF-2)) (const_int 0))

   can be converted to

          (ashift:SI X (const_int N))

   where N is the appropriate shift count to move the bit being
   tested into the sign bit.

   There is no way to describe a machine that always sets the
   low-order bit for a true value, but does not guarantee the value
   of any other bits, but we do not know of any machine that has such
   an instruction.  If you are trying to port GCC to such a
   machine, include an instruction to perform a logical-and of the
   result with 1 in the pattern for the comparison operators and let
   us know.
     
   Often, a machine will have multiple instructions that obtain a
   value from a comparison (or the condition codes).  Here are rules
   to guide the choice of value for `STORE_FLAG_VALUE', and hence the
   instructions to be used:

        * Use the shortest sequence that yields a valid definition for
          `STORE_FLAG_VALUE'.  It is more efficient for the compiler to
          "normalize" the value (convert it to, e.g., 1 or 0) than for
          the comparison operators to do so because there may be
          opportunities to combine the normalization with other
          operations.

        * For equal-length sequences, use a value of 1 or -1, with -1
          being slightly preferred on machines with expensive jumps and
          1 preferred on other machines.

        * As a second choice, choose a value of `0x80000001' if
          instructions exist that set both the sign and low-order bits
          but do not define the others.

        * Otherwise, use a value of `0x80000000'.

   Many machines can produce both the value chosen for
   `STORE_FLAG_VALUE' and its negation in the same number of
   instructions.  On those machines, you should also define a pattern
   for those cases, e.g., one matching

          (set A (neg:M (ne:M B C)))

   Some machines can also perform `and' or `plus' operations on
   condition code values with less instructions than the corresponding
   `sCOND' insn followed by `and' or `plus'.  On those machines,
   define the appropriate patterns.  Use the names `incscc' and
   `decscc', respectively, for the patterns which perform `plus' or
   `minus' operations on condition code values.  See `rs6000.md' for
   some examples.  The GNU Superoptizer can be used to find such
   instruction sequences on other machines.

   You need not define `STORE_FLAG_VALUE' if the machine has no
   store-flag instructions.  */
#define STORE_FLAG_VALUE 1

/* An alias for the machine mode for pointers.  On most machines,
   define this to be the integer mode corresponding to the width of a
   hardware pointer; `SImode' on 32-bit machine or `DImode' on 64-bit
   machines.  On some machines you must define this to be one of the
   partial integer modes, such as `PSImode'.

   The width of `Pmode' must be at least as large as the value of
   `POINTER_SIZE'.  If it is not equal, you must define the macro
   `POINTERS_EXTEND_UNSIGNED' to specify how pointers are extended to
   `Pmode'.  */
#define Pmode HImode

/* An alias for the machine mode used for memory references to
   functions being called, in `call' RTL expressions.  On most
   machines this should be `QImode'.  */
#define FUNCTION_MODE Pmode

/* Define this macro if the system header files support C++ as well as C.  This
   macro inhibits the usual method of using system header files in C++, which
   is to pretend that the file's contents are enclosed in `extern "C" {...}'.  */
#define NO_IMPLICIT_EXTERN_C

/* Define this macro to handle System V style pragmas.  */
#undef  HANDLE_SYSV_PRAGMA
#define HANDLE_SYSV_PRAGMA 1


/* Hack to omit compiling L_shtab in libgcc2.  Shtab is an array mapping
   n -> 1<<n that is used in the Tahoe, but it assumes that int can handle
   32-bits.  */

#undef L_shtab


/* Haifa scheduler options.  */

/* Indicate that issue rate is defined for this machine
   (no need to use the default).  */
#define ISSUE_RATE 2

/* If defined, C string expressions to be used for the `%R', `%L', `%U', and
   `%I' options of `asm_fprintf' (see `final.c').  */
#define REGISTER_PREFIX     "%"
#define LOCAL_LABEL_PREFIX  "."
#define USER_LABEL_PREFIX   ""
#define IMMEDIATE_PREFIX    ""

/* A C expression that is nonzero if hard register number REGNO2
   can be considered for use as a rename register for REGNO1.  */
#define HARD_REGNO_RENAME_OK(REGNO1,REGNO2) \
  (RETURN_ADDRESS_REGNUM == REGNO2 ? 0 : 1)
