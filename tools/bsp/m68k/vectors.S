/*
 * vectors.S -- Exception vector code for M68K BSP
 *
 * Copyright (c) 1998, 1999 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include <bsp/cpu.h>

	.file	"vectors.S"

/*
 * Macro Definition for an EXCEPTION vector
 *
 * This will simply push the vector number and jump to the
 * generic exception handler.
 */
.macro EXCEPTION_VECTOR label, vect_num
FUNC_START \label
        pea   \vect_num
        bra   SYM_NAME(generic_except)
FUNC_END \label
.endm

/*
 * Setup the exception vectors for all the 68k exceptions
 */
        EXCEPTION_VECTOR m68k_addr_error_asm, BSP_CORE_EXC_ADDR_ERROR
        EXCEPTION_VECTOR m68k_bus_error_asm,  BSP_CORE_EXC_BUS_ERROR
        EXCEPTION_VECTOR m68k_ill_instruction_asm, BSP_CORE_EXC_ILL_INSTRUCTION
        EXCEPTION_VECTOR m68k_div_zero_asm, BSP_CORE_EXC_DIV_ZERO
        EXCEPTION_VECTOR m68k_chk_asm, BSP_CORE_EXC_CHK
        EXCEPTION_VECTOR m68k_ftrap_asm, BSP_CORE_EXC_TRAP
        EXCEPTION_VECTOR m68k_priv_violation_asm, BSP_CORE_EXC_PRIV_VIOLATION
        EXCEPTION_VECTOR m68k_trace_asm, BSP_CORE_EXC_TRACE
        EXCEPTION_VECTOR m68k_line_1010_asm, BSP_CORE_EXC_LINE_1010
        EXCEPTION_VECTOR m68k_line_1111_asm, BSP_CORE_EXC_LINE_1111
        EXCEPTION_VECTOR m68k_coproc_protocol_violation_asm, BSP_CORE_EXC_COPROC_PROTOCOL_VIOLATION
        EXCEPTION_VECTOR m68k_format_error_asm, BSP_CORE_EXC_FORMAT_ERROR
        EXCEPTION_VECTOR m68k_uninitialized_interrupt_asm, BSP_CORE_EXC_UNINITIALIZED_INTERRUPT
        EXCEPTION_VECTOR m68k_spurious_int_asm, BSP_CORE_EXC_SPURIOUS_INT
        EXCEPTION_VECTOR m68k_level_1_auto_asm, BSP_CORE_EXC_LEVEL_1_AUTO
        EXCEPTION_VECTOR m68k_level_2_auto_asm, BSP_CORE_EXC_LEVEL_2_AUTO
        EXCEPTION_VECTOR m68k_level_3_auto_asm, BSP_CORE_EXC_LEVEL_3_AUTO
        EXCEPTION_VECTOR m68k_level_4_auto_asm, BSP_CORE_EXC_LEVEL_4_AUTO
        EXCEPTION_VECTOR m68k_level_5_auto_asm, BSP_CORE_EXC_LEVEL_5_AUTO
        EXCEPTION_VECTOR m68k_level_6_auto_asm, BSP_CORE_EXC_LEVEL_6_AUTO
        EXCEPTION_VECTOR m68k_level_7_auto_asm, BSP_CORE_EXC_LEVEL_7_AUTO
        EXCEPTION_VECTOR m68k_trap_0_asm, BSP_CORE_EXC_TRAP_0
        EXCEPTION_VECTOR m68k_trap_1_asm, BSP_CORE_EXC_TRAP_1
        EXCEPTION_VECTOR m68k_trap_2_asm, BSP_CORE_EXC_TRAP_2
        EXCEPTION_VECTOR m68k_trap_3_asm, BSP_CORE_EXC_TRAP_3
        EXCEPTION_VECTOR m68k_trap_4_asm, BSP_CORE_EXC_TRAP_4
        EXCEPTION_VECTOR m68k_trap_5_asm, BSP_CORE_EXC_TRAP_5
        EXCEPTION_VECTOR m68k_trap_6_asm, BSP_CORE_EXC_TRAP_6
        EXCEPTION_VECTOR m68k_trap_7_asm, BSP_CORE_EXC_TRAP_7
        EXCEPTION_VECTOR m68k_trap_8_asm, BSP_CORE_EXC_TRAP_8
        EXCEPTION_VECTOR m68k_trap_9_asm, BSP_CORE_EXC_TRAP_9
        EXCEPTION_VECTOR m68k_trap_10_asm, BSP_CORE_EXC_TRAP_10
        EXCEPTION_VECTOR m68k_trap_11_asm, BSP_CORE_EXC_TRAP_11
        EXCEPTION_VECTOR m68k_trap_12_asm, BSP_CORE_EXC_TRAP_12
        EXCEPTION_VECTOR m68k_trap_13_asm, BSP_CORE_EXC_TRAP_13
        EXCEPTION_VECTOR m68k_trap_14_asm, BSP_CORE_EXC_TRAP_14
        EXCEPTION_VECTOR m68k_trap_15_asm, BSP_CORE_EXC_TRAP_15
        EXCEPTION_VECTOR m68k_fp_unordered_cond_asm, BSP_CORE_EXC_FP_UNORDERED_COND
        EXCEPTION_VECTOR m68k_fp_inexact_asm, BSP_CORE_EXC_FP_INEXACT
        EXCEPTION_VECTOR m68k_fp_div_zero_asm, BSP_CORE_EXC_FP_DIV_ZERO
        EXCEPTION_VECTOR m68k_fp_underflow_asm, BSP_CORE_EXC_FP_UNDERFLOW
        EXCEPTION_VECTOR m68k_fp_operand_error_asm, BSP_CORE_EXC_FP_OPERAND_ERROR
        EXCEPTION_VECTOR m68k_fp_overflow_asm, BSP_CORE_EXC_FP_OVERFLOW
        EXCEPTION_VECTOR m68k_fp_nan_asm, BSP_CORE_EXC_FP_NAN
        EXCEPTION_VECTOR m68k_fp_unimp_data_type_asm, BSP_CORE_EXC_FP_UNIMP_DATA_TYPE
        EXCEPTION_VECTOR m68k_mmu_config_error_asm, BSP_CORE_EXC_MMU_CONFIG_ERROR
        EXCEPTION_VECTOR m68k_mmu_ill_operation_asm, BSP_CORE_EXC_MMU_ILL_OPERATION
        EXCEPTION_VECTOR m68k_mmu_access_violation_asm, BSP_CORE_EXC_MMU_ACCESS_VIOLATION

/*
 * This is the generic exception handler.
 * All exceptions will be routed through this routine.
 *
 * Once we are here, the vector number is pushed onto the stack.
 * 
 * We will set up the parameters for a _bsp_exc_dispatch() call
 *
 */
FUNC_START generic_except
	/*
         * Save the address of the stack frame on the stack
         */
        pea     sp@(4)

        /*
         * See if we need to switch stacks
         */
      	cmp	IMM(SYM_NAME(_ex_stack)),sp
        ble	1f

        /*
	 * Need to switch stacks.
         */
        movel   a0, sp@-                        /* Save a0                          */

        /*
         * At this point, the main (old) stack will look as follows:
         *              .               <-- High Mem
         *              .
         *              .
         *       |              |
         *       |   Program    |
         *       |    Stack     |
         *       |              |
         *              .       
         *              .
         *              .
         *       |              |
         *       | Stack Frame  |
         *       +--------------+
         *       | vect_num (4) |
         *       +--------------+
         *       | &Stack Frame |
         *       +--------------+
         *       |   a0 saved   |
         *       +--------------+
         *
         *
         * Let's make the new stack (a0) look as follows:
         *
         *  ex_stack:   +--------------+
         *  (in bss)    | vect_num (4) |
         *              +--------------+
         *              | &Stack Frame |
         *              +--------------+
         *              |   a0 saved   |
         *              +--------------+
         */
        moveal  IMM(SYM_NAME(_ex_stack)), a0    /* a0 is the new stack              */
        movel   sp@(8), a0@-                    /* Copy vect # to new stack         */
        movel   sp@(4), a0@-                    /* Copy stkfrm address to new stack */
        movel   sp@, a0@-                       /* Copy saved a0 to new stack       */

        /*
         * Now we switch to the new stack
         * and restore a0
         */
        movel  a0, sp                           /* Start using the new stack        */
        moveal  sp@+, a0                        /* Restore a0                       */

        /*
         * Now we are using the right stack.
         */
1:
        lea    sp@(-12), sp            /* save space for SP/SR/PC in the register image        */
        moveml d0-d7/a0-a6,sp@-        /* save all the registers before C can modify them      */

        /*
         * Call into C code
         *     processException(uint16 *stkfrm, uint32 *registers, uint32 vect_num)
         *
         * At this point, we want the stack to look as follows:
         *              .
         *              .             <-- High Mem
         *              .
         *       |              |
         *       +--------------+
         *       | vect_num (4) |
         *       +--------------+
         *       | &Stk Frm(4)  |
         *       +--------------+
         *       |              |
         *       |              |   (16 data & address registers (4 bytes wide)
         *       | Reg Img (72) |    + PC (4 bytes wide)
         *       |              |    + SR (4 bytes wide))
         *       |              |
         *       +--------------+
         *       | vect_num (4) |   *(SP + 72 + 4)
         *       +--------------+
         *       | &Reg Img (4) |   (SP + 4)
         *       +--------------+
         *       | &StkFrm (4)  |   *(SP + 72 + 4 + 4)
         *       +--------------+
         */
        movel  sp@(72+4),sp@-             /* Last Parm is the vect_num                    */
        pea    sp@(4)                     /* Second Parm is the address of register image */
        movel  sp@(72+4+4),sp@-           /* First Parm is the stkfrm                     */
        bsr    SYM_NAME(processException) /* Go to C code                                 */
        movel  d0, a1                     /* &StkFrm in a1                                */
        movel  sp@(4), a6                 /* &regimage in a6                              */
        
        btstb  IMM(5), a1@(0)             /* Test the super/user state bit in saved SR    */
        beq    SYM_NAME(return_to_user)   /* If bit == 0;  return to user state           */
        bra    SYM_NAME(return_to_super)  /* else;         return to system state         */
FUNC_END generic_except

/*
 * Exception return code
 * Call this if returning to supervisor state
 *
 * Calling Sequence:
 *        return_to_super(unsigned short *stkfrm, ex_regs_t *registers)
 */
FUNC_START return_to_super
        /*
         * get new stack pointer
         */
        movel   a6@(sp_o), sp

        /*
         * Adjust the stkfrm pointer to point at the end of the struct,
         * so the copy by predecrement below will work.
         */
        addql   IMM(STKFRM_SIZE), a1

        /*
         * copy the stack frame
         */
        movew   a1@-, sp@-       /* copy the SR                       */
        movel   a1@-, sp@-       /* copy the PC                       */

        bra     SYM_NAME(return_to_any)
FUNC_END return_to_super

/*
 * Exception return code
 * Call this if returning to user state
 *
 * Calling Sequence:
 *        return_to_user(unsigned short *stkfrm, ex_regs_t *registers)
 */
FUNC_START return_to_user
        /*
         * get usp
         */
        movel   a6@(sp_o), a0
        movel   a0, usp              /* set usp                           */

        movel   a1, sp               /* get original superStack pointer   */

        bra     SYM_NAME(return_to_any)
FUNC_END return_to_user

/*
 * Exception return code.  Called by return_to_super and return_to_user
 *
 * Calling Sequence:
 *        return_to_any
 *              a1 = stkfrm
 *              a6 = registers
 */
FUNC_START return_to_any
        /*
         * It is critical that we don't touch the stack pointer
         * now, because it is setup as GDB requested and we don't
         * know the state of the data on either side of the SP
         */
        
        /*
         * Restore from the register image
         */
        moveml  a6@+, d0-d7/a0-a5
        moveal  a6@, a6

        rte                        /* return from exception             */
FUNC_END return_to_any

/*
 *  Provide a stack for use by the exception handler.
 *  Note that this stack is in .bss, so it will be below
 *  the normal user stack which is placed in high memory.
 *  We use this fact to detect whether or not we need to
 *  switch stacks before saving regs.
 */
	.bss
	.align 4
	.rept	16384
	.byte	0
	.endr
SYM_NAME(_ex_stack):
	.long	0

/*
 * Assembly representing the exception vectors.
 */
        .section .ram_vectors
        .global SYM_NAME(_VECTOR_START)
SYM_NAME(_VECTOR_START):
        .long    0                                            /* 0 - Initial Int Stack Pointer   */
        .long    0                                            /* 1 - Reset initial PC            */
        .long    SYM_NAME(m68k_bus_error_asm)                 /* 2 - Access Fault                */
        .long    SYM_NAME(m68k_addr_error_asm)                /* 3 - Address Error               */
        .long    SYM_NAME(m68k_ill_instruction_asm)           /* 4 - Illegal Instruction         */
        .long    SYM_NAME(m68k_div_zero_asm)                  /* 5 - Divide by Zero              */
        .long    SYM_NAME(m68k_chk_asm)                       /* 6 - CHK, CHK2                   */
        .long    SYM_NAME(m68k_ftrap_asm)                     /* 7 - FTRAPcc, TRAPcc, TRAPV      */
        .long    SYM_NAME(m68k_priv_violation_asm)            /* 8 - Privilege Violation         */
        .long    SYM_NAME(m68k_trace_asm)                     /* 9 - Trace                       */
        .long    SYM_NAME(m68k_line_1010_asm)                 /* 10 - Line 1010 Emulator         */
        .long    SYM_NAME(m68k_line_1111_asm)                 /* 11 - Line 1111 Emulator         */
        .long    0                                            /* 12 - Unassigned/Reserved        */
        .long    SYM_NAME(m68k_coproc_protocol_violation_asm) /* 13 - Coproc Protocol Violation  */
        .long    SYM_NAME(m68k_format_error_asm)              /* 14 - Format Error               */
        .long    SYM_NAME(m68k_uninitialized_interrupt_asm)   /* 15 - Uninitialized Interrupt    */
        .long    0                                            /* 16 - Unassigned/Reserved        */
        .long    0                                            /* 17 - Unassigned/Reserved        */
        .long    0                                            /* 18 - Unassigned/Reserved        */
        .long    0                                            /* 19 - Unassigned/Reserved        */
        .long    0                                            /* 20 - Unassigned/Reserved        */
        .long    0                                            /* 21 - Unassigned/Reserved        */
        .long    0                                            /* 22 - Unassigned/Reserved        */
        .long    0                                            /* 23 - Unassigned/Reserved        */
        .long    SYM_NAME(m68k_spurious_int_asm)              /* 24 - Spurious Interrupt         */
        .long    SYM_NAME(m68k_level_1_auto_asm)              /* 25 - Level 1 Autovector         */
        .long    SYM_NAME(m68k_level_2_auto_asm)              /* 26 - Level 2 Autovector         */
        .long    SYM_NAME(m68k_level_3_auto_asm)              /* 27 - Level 3 Autovector         */
        .long    SYM_NAME(m68k_level_4_auto_asm)              /* 28 - Level 4 Autovector         */
        .long    SYM_NAME(m68k_level_5_auto_asm)              /* 29 - Level 5 Autovector         */
        .long    SYM_NAME(m68k_level_6_auto_asm)              /* 30 - Level 6 Autovector         */
        .long    SYM_NAME(m68k_level_7_auto_asm)              /* 31 - Level 7 Autovector         */
        .long    SYM_NAME(m68k_trap_0_asm)                    /* 32 - Trap 0 Instruction         */
        .long    SYM_NAME(m68k_trap_1_asm)                    /* 33 - Trap 1 Instruction         */
        .long    SYM_NAME(m68k_trap_2_asm)                    /* 34 - Trap 2 Instruction         */
        .long    SYM_NAME(m68k_trap_3_asm)                    /* 35 - Trap 3 Instruction         */
        .long    SYM_NAME(m68k_trap_4_asm)                    /* 36 - Trap 4 Instruction         */
        .long    SYM_NAME(m68k_trap_5_asm)                    /* 37 - Trap 5 Instruction         */
        .long    SYM_NAME(m68k_trap_6_asm)                    /* 38 - Trap 6 Instruction         */
        .long    SYM_NAME(m68k_trap_7_asm)                    /* 39 - Trap 7 Instruction         */
        .long    SYM_NAME(m68k_trap_8_asm)                    /* 40 - Trap 8 Instruction         */
        .long    SYM_NAME(m68k_trap_9_asm)                    /* 41 - Trap 9 Instruction         */
        .long    SYM_NAME(m68k_trap_10_asm)                   /* 42 - Trap 10 Instruction        */
        .long    SYM_NAME(m68k_trap_11_asm)                   /* 43 - Trap 11 Instruction        */
        .long    SYM_NAME(m68k_trap_12_asm)                   /* 44 - Trap 12 Instruction        */
        .long    SYM_NAME(m68k_trap_13_asm)                   /* 45 - Trap 13 Instruction        */
        .long    SYM_NAME(m68k_trap_14_asm)                   /* 46 - Trap 14 Instruction        */
        .long    SYM_NAME(m68k_trap_15_asm)                   /* 47 - Trap 15 Instruction        */
        .long    SYM_NAME(m68k_fp_unordered_cond_asm)         /* 48 - FP Unordered Condition     */
        .long    SYM_NAME(m68k_fp_inexact_asm)                /* 49 - FP Inexact Result          */
        .long    SYM_NAME(m68k_fp_div_zero_asm)               /* 50 - FP Divide by Zero          */
        .long    SYM_NAME(m68k_fp_underflow_asm)              /* 51 - FP Underflow               */
        .long    SYM_NAME(m68k_fp_operand_error_asm)          /* 52 - FP Operand Error           */
        .long    SYM_NAME(m68k_fp_overflow_asm)               /* 53 - FP Overflow                */
        .long    SYM_NAME(m68k_fp_nan_asm)                    /* 54 - FP Signaling NAN           */
        .long    SYM_NAME(m68k_fp_unimp_data_type_asm)        /* 55 - FP Unimplemented Data Type */
        .long    SYM_NAME(m68k_mmu_config_error_asm)          /* 56 - MMU Configuration Error    */
        .long    SYM_NAME(m68k_mmu_ill_operation_asm)         /* 57 - MMU Illegal Operation      */
        .long    SYM_NAME(m68k_mmu_access_violation_asm)      /* 58 - MMU Access Level Violation */
        .long    0                                            /* 59 - Unassigned/Reserved        */
        .long    0                                            /* 60 - Unassigned/Reserved        */
        .long    0                                            /* 61 - Unassigned/Reserved        */
        .long    0                                            /* 62 - Unassigned/Reserved        */
        .long    0                                            /* 63 - Unassigned/Reserved        */
        .long    0                                            /* 64 - User Defined #1            */
        .long    0                                            /* 65 - User Defined #2            */
        .long    0                                            /* 66 - User Defined #3            */
        .long    0                                            /* 67 - User Defined #4            */
        .long    0                                            /* 68 - User Defined #5            */
        .long    0                                            /* 69 - User Defined #6            */
        .long    0                                            /* 70 - User Defined #7            */
        .long    0                                            /* 71 - User Defined #8            */
        .long    0                                            /* 72 - User Defined #9            */
        .long    0                                            /* 73 - User Defined #10           */
        .long    0                                            /* 74 - User Defined #11           */
        .long    0                                            /* 75 - User Defined #12           */
        .long    0                                            /* 76 - User Defined #13           */
        .long    0                                            /* 77 - User Defined #14           */
        .long    0                                            /* 78 - User Defined #15           */
        .long    0                                            /* 79 - User Defined #16           */
        .long    0                                            /* 80 - User Defined #17           */
        .long    0                                            /* 81 - User Defined #18           */
        .long    0                                            /* 82 - User Defined #19           */
        .long    0                                            /* 83 - User Defined #20           */
        .long    0                                            /* 84 - User Defined #21           */
        .long    0                                            /* 85 - User Defined #22           */
        .long    0                                            /* 86 - User Defined #23           */
        .long    0                                            /* 87 - User Defined #24           */
        .long    0                                            /* 88 - User Defined #25           */
        .long    0                                            /* 89 - User Defined #26           */
        .long    0                                            /* 90 - User Defined #27           */
        .long    0                                            /* 91 - User Defined #28           */
        .long    0                                            /* 92 - User Defined #29           */
        .long    0                                            /* 93 - User Defined #30           */
        .long    0                                            /* 94 - User Defined #31           */
        .long    0                                            /* 95 - User Defined #32           */
        .long    0                                            /* 96 - User Defined #33           */
        .long    0                                            /* 97 - User Defined #34           */
        .long    0                                            /* 98 - User Defined #35           */
        .long    0                                            /* 99 - User Defined #36           */
        .long    0                                            /* 100 - User Defined #37          */
        .long    0                                            /* 101 - User Defined #38          */
        .long    0                                            /* 102 - User Defined #39          */
        .long    0                                            /* 103 - User Defined #40          */
        .long    0                                            /* 104 - User Defined #41          */
        .long    0                                            /* 105 - User Defined #42          */
        .long    0                                            /* 106 - User Defined #43          */
        .long    0                                            /* 107 - User Defined #44          */
        .long    0                                            /* 108 - User Defined #45          */
        .long    0                                            /* 109 - User Defined #46          */
        .long    0                                            /* 110 - User Defined #47          */
        .long    0                                            /* 111 - User Defined #48          */
        .long    0                                            /* 112 - User Defined #49          */
        .long    0                                            /* 113 - User Defined #50          */
        .long    0                                            /* 114 - User Defined #51          */
        .long    0                                            /* 115 - User Defined #52          */
        .long    0                                            /* 116 - User Defined #53          */
        .long    0                                            /* 117 - User Defined #54          */
        .long    0                                            /* 118 - User Defined #55          */
        .long    0                                            /* 119 - User Defined #56          */
        .long    0                                            /* 120 - User Defined #57          */
        .long    0                                            /* 121 - User Defined #58          */
        .long    0                                            /* 122 - User Defined #59          */
        .long    0                                            /* 123 - User Defined #60          */
        .long    0                                            /* 124 - User Defined #61          */
        .long    0                                            /* 125 - User Defined #62          */
        .long    0                                            /* 126 - User Defined #63          */
        .long    0                                            /* 127 - User Defined #64          */
        .long    0                                            /* 128 - User Defined #65          */
        .long    0                                            /* 129 - User Defined #66          */
        .long    0                                            /* 130 - User Defined #67          */
        .long    0                                            /* 131 - User Defined #68          */
        .long    0                                            /* 132 - User Defined #69          */
        .long    0                                            /* 133 - User Defined #70          */
        .long    0                                            /* 134 - User Defined #71          */
        .long    0                                            /* 135 - User Defined #72          */
        .long    0                                            /* 136 - User Defined #73          */
        .long    0                                            /* 137 - User Defined #74          */
        .long    0                                            /* 138 - User Defined #75          */
        .long    0                                            /* 139 - User Defined #76          */
        .long    0                                            /* 140 - User Defined #77          */
        .long    0                                            /* 141 - User Defined #78          */
        .long    0                                            /* 142 - User Defined #79          */
        .long    0                                            /* 143 - User Defined #80          */
        .long    0                                            /* 144 - User Defined #81          */
        .long    0                                            /* 145 - User Defined #82          */
        .long    0                                            /* 146 - User Defined #83          */
        .long    0                                            /* 147 - User Defined #84          */
        .long    0                                            /* 148 - User Defined #85          */
        .long    0                                            /* 149 - User Defined #86          */
        .long    0                                            /* 150 - User Defined #87          */
        .long    0                                            /* 151 - User Defined #88          */
        .long    0                                            /* 152 - User Defined #89          */
        .long    0                                            /* 153 - User Defined #90          */
        .long    0                                            /* 154 - User Defined #91          */
        .long    0                                            /* 155 - User Defined #92          */
        .long    0                                            /* 156 - User Defined #93          */
        .long    0                                            /* 157 - User Defined #94          */
        .long    0                                            /* 158 - User Defined #95          */
        .long    0                                            /* 159 - User Defined #96          */
        .long    0                                            /* 160 - User Defined #97          */
        .long    0                                            /* 161 - User Defined #98          */
        .long    0                                            /* 162 - User Defined #99          */
        .long    0                                            /* 163 - User Defined #100         */
        .long    0                                            /* 164 - User Defined #101         */
        .long    0                                            /* 165 - User Defined #102         */
        .long    0                                            /* 166 - User Defined #103         */
        .long    0                                            /* 167 - User Defined #104         */
        .long    0                                            /* 168 - User Defined #105         */
        .long    0                                            /* 169 - User Defined #106         */
        .long    0                                            /* 170 - User Defined #107         */
        .long    0                                            /* 171 - User Defined #108         */
        .long    0                                            /* 172 - User Defined #109         */
        .long    0                                            /* 173 - User Defined #110         */
        .long    0                                            /* 174 - User Defined #111         */
        .long    0                                            /* 175 - User Defined #112         */
        .long    0                                            /* 176 - User Defined #113         */
        .long    0                                            /* 177 - User Defined #114         */
        .long    0                                            /* 178 - User Defined #115         */
        .long    0                                            /* 179 - User Defined #116         */
        .long    0                                            /* 180 - User Defined #117         */
        .long    0                                            /* 181 - User Defined #118         */
        .long    0                                            /* 182 - User Defined #119         */
        .long    0                                            /* 183 - User Defined #120         */
        .long    0                                            /* 184 - User Defined #121         */
        .long    0                                            /* 185 - User Defined #122         */
        .long    0                                            /* 186 - User Defined #123         */
        .long    0                                            /* 187 - User Defined #124         */
        .long    0                                            /* 188 - User Defined #125         */
        .long    0                                            /* 189 - User Defined #126         */
        .long    0                                            /* 190 - User Defined #127         */
        .long    0                                            /* 191 - User Defined #128         */
        .long    0                                            /* 192 - User Defined #129         */
        .long    0                                            /* 193 - User Defined #130         */
        .long    0                                            /* 194 - User Defined #131         */
        .long    0                                            /* 195 - User Defined #132         */
        .long    0                                            /* 196 - User Defined #133         */
        .long    0                                            /* 197 - User Defined #134         */
        .long    0                                            /* 198 - User Defined #135         */
        .long    0                                            /* 199 - User Defined #136         */
        .long    0                                            /* 200 - User Defined #137         */
        .long    0                                            /* 201 - User Defined #138         */
        .long    0                                            /* 202 - User Defined #139         */
        .long    0                                            /* 203 - User Defined #140         */
        .long    0                                            /* 204 - User Defined #141         */
        .long    0                                            /* 205 - User Defined #142         */
        .long    0                                            /* 206 - User Defined #143         */
        .long    0                                            /* 207 - User Defined #144         */
        .long    0                                            /* 208 - User Defined #145         */
        .long    0                                            /* 209 - User Defined #146         */
        .long    0                                            /* 210 - User Defined #147         */
        .long    0                                            /* 211 - User Defined #148         */
        .long    0                                            /* 212 - User Defined #149         */
        .long    0                                            /* 213 - User Defined #150         */
        .long    0                                            /* 214 - User Defined #151         */
        .long    0                                            /* 215 - User Defined #152         */
        .long    0                                            /* 216 - User Defined #153         */
        .long    0                                            /* 217 - User Defined #154         */
        .long    0                                            /* 218 - User Defined #155         */
        .long    0                                            /* 219 - User Defined #156         */
        .long    0                                            /* 220 - User Defined #157         */
        .long    0                                            /* 221 - User Defined #158         */
        .long    0                                            /* 222 - User Defined #159         */
        .long    0                                            /* 223 - User Defined #160         */
        .long    0                                            /* 224 - User Defined #161         */
        .long    0                                            /* 225 - User Defined #162         */
        .long    0                                            /* 226 - User Defined #163         */
        .long    0                                            /* 227 - User Defined #164         */
        .long    0                                            /* 228 - User Defined #165         */
        .long    0                                            /* 229 - User Defined #166         */
        .long    0                                            /* 230 - User Defined #167         */
        .long    0                                            /* 231 - User Defined #168         */
        .long    0                                            /* 232 - User Defined #169         */
        .long    0                                            /* 233 - User Defined #170         */
        .long    0                                            /* 234 - User Defined #171         */
        .long    0                                            /* 235 - User Defined #172         */
        .long    0                                            /* 236 - User Defined #173         */
        .long    0                                            /* 237 - User Defined #174         */
        .long    0                                            /* 238 - User Defined #175         */
        .long    0                                            /* 239 - User Defined #176         */
        .long    0                                            /* 240 - User Defined #177         */
        .long    0                                            /* 241 - User Defined #178         */
        .long    0                                            /* 242 - User Defined #179         */
        .long    0                                            /* 243 - User Defined #180         */
        .long    0                                            /* 244 - User Defined #181         */
        .long    0                                            /* 245 - User Defined #182         */
        .long    0                                            /* 246 - User Defined #183         */
        .long    0                                            /* 247 - User Defined #184         */
        .long    0                                            /* 248 - User Defined #185         */
        .long    0                                            /* 249 - User Defined #186         */
        .long    0                                            /* 250 - User Defined #187         */
        .long    0                                            /* 251 - User Defined #188         */
        .long    0                                            /* 252 - User Defined #189         */
        .long    0                                            /* 253 - User Defined #190         */
        .long    0                                            /* 254 - User Defined #191         */
        .long    0                                            /* 255 - User Defined #192         */
