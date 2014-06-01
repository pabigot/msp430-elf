/*
 * cpu.h -- CPU specific header for Cygnus BSP.
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
#ifndef __CPU_H__
#define __CPU_H__ 1

/*
 * __NEED_UNDERSCORE__ is not defined for any M68K targets
 */
#ifdef __NEED_UNDERSCORE__
#   error __NEED_UNDERSCORE__ is defined and should not be.
#endif

/*
 * Macros to glue together two tokens.
 */
#  ifdef __STDC__
#    define XGLUE(a,b) a##b
#  else
#    define XGLUE(a,b) a/**/b
#  endif

#  define GLUE(a,b) XGLUE(a,b)

/*
 * Symbol Names with leading underscore if necessary
 */
#  ifdef __NEED_UNDERSCORE__
#    define SYM_NAME(name) GLUE(_,name)
#  else
#    define SYM_NAME(name) name
#  endif /* __NEED_UNDERSCORE__ */

/*
 * Various macros to better handle assembler/object format differences
 */
#if defined(__ASSEMBLER__)

/*
 * Assembly function start definition
 */
#ifdef __NEED_UNDERSCORE__
.macro FUNC_START name
	.global _\name
	.align  4
    _\name:
.endm
#else
.macro FUNC_START name
	.global \name
	.align  4
    \name:
.endm
#endif

/*
 * Assembly function end definition
 */
#ifdef __NEED_UNDERSCORE__
.macro FUNC_END name
.endm
#else
.macro FUNC_END name
.endm
#endif

/*
 * Register Prefix
 */
#  ifndef __REGISTER_PREFIX__
#    define __REGISTER_PREFIX__
#  endif /* __REGISTER_PREFIX__ */

/*
 * Immediate Prefix
 */
#  ifndef __IMM_PREFIX__
#    define __IMM_PREFIX__ #
#  endif /* __IMM_PREFIX__ */

/*
 * use the right prefix for registers.
 */
#  define REG(x) GLUE(__REGISTER_PREFIX__,x)

/*
 * use the right prefix for immediate values.
 */
#  define IMM(x) GLUE(__IMM_PREFIX__,x)

#endif /* defined(__ASSEMBLER__) */


/*
 * Setup register defines and such
 */
#if defined(__ASSEMBLER__)

#  define d0   REG (d0)
#  define d1   REG (d1)
#  define d2   REG (d2)
#  define d3   REG (d3)
#  define d4   REG (d4)
#  define d5   REG (d5)
#  define d6   REG (d6)
#  define d7   REG (d7)
#  define a0   REG (a0)
#  define a1   REG (a1)
#  define a2   REG (a2)
#  define a3   REG (a3)
#  define a4   REG (a4)
#  define a5   REG (a5)
#  define a6   REG (a6)
#  define a7   REG (a7)
#  define fp   REG (fp)
#  define fp0  REG (fp0)
#  define fp1  REG (fp1)
#  define fp2  REG (fp2)
#  define fp3  REG (fp3)
#  define fp4  REG (fp4)
#  define fp5  REG (fp5)
#  define fp6  REG (fp6)
#  define fp7  REG (fp7)
#  define sp   REG (sp)
#  define usp  REG (usp)
#  define vbr  REG (vbr)
#  define sr   REG (sr)
#  define fpcr REG (fpcr)
#  define fpsr REG (fpsr)
#  define fpi  REG (fpi)

/*
 * Register offset definitions
 * These numbers are offsets into the ex_regs_t struct.
 */
#  define d0_o 0
#  define d1_o 4
#  define d2_o 8
#  define d3_o 12
#  define d4_o 16
#  define d5_o 20
#  define d6_o 24
#  define d7_o 28
#  define a0_o 32
#  define a1_o 36
#  define a2_o 40
#  define a3_o 44
#  define a4_o 48
#  define a5_o 52
#  define a6_o 56
#  define a7_o 60
#  define sp_o a7_o
#  define sr_o 64
#  define pc_o 68

#  define M68K_EX_REGS_T_SIZE 72

#else /* !defined(__ASSEMBLER__) */

  /*
   * Register name that is used in help strings and such
   */
# define REGNAME_EXAMPLE "d0"

  /*
   *  Register numbers. These are assumed to match the
   *  register numbers used by GDB.
   */
  enum __regnames {
    REG_D0,
    REG_D1,
    REG_D2,
    REG_D3,
    REG_D4,
    REG_D5,
    REG_D6,
    REG_D7,
    REG_A0,
    REG_A1,
    REG_A2,
    REG_A3,
    REG_A4,
    REG_A5,
    REG_A6,
    REG_A7,
    REG_SP=REG_A7,
    REG_SR,
    REG_PC,
    REG_MAX=REG_PC
  };

  /*
   *  How registers are stored for exceptions.
   */
  typedef struct
  {
      unsigned long _d0;
      unsigned long _d1;
      unsigned long _d2;
      unsigned long _d3;
      unsigned long _d4;
      unsigned long _d5;
      unsigned long _d6;
      unsigned long _d7;

      unsigned long _a0;
      unsigned long _a1;
      unsigned long _a2;
      unsigned long _a3;
      unsigned long _a4;
      unsigned long _a5;
      unsigned long _a6;
      unsigned long _a7;
#define _sp _a7
      unsigned long _sr;
      unsigned long _pc;
  } ex_regs_t;

#endif /* __ASSEMBLER__ */


/*
 * Status Register Bitmasks
 */
#define MC68000_SR_TRACEMODE_ANY  0x8000  /* Enable/disable Trace mode on any instruction*/
#define MC68000_SR_TRACEMODE_FLOW 0x4000  /* Enable/disable Trace mode on change of flow */
#define MC68000_SR_STATE          0x2000  /* Supervisor/User state                       */
#define MC68000_SR_ACTIVESTACK    0x1000  /* User/Interrupt Stack                        */
#define MC68000_SR_INT_MASK       0x0700  /* Interrupt Priority Mask                     */

/*
 * Condition Code Register Bitmasks
 */
#define MC68000_CR_EXTEND         0x0010  /* Sign Extend Bit                             */
#define MC68000_CR_NEGATIVE       0x0008  /* Negative Bit                                */
#define MC68000_CR_ZERO           0x0004  /* Zero Bit                                    */
#define MC68000_CR_OVERFLOW       0x0002  /* Overflow Bit                                */
#define MC68000_CR_CARRY          0x0001  /* Carry Bit                                   */

/*
 * Core Exception vectors.
 */
#define BSP_CORE_EXC_INITIAL_INT_STACK_PTR      0
#define BSP_CORE_EXC_INITIAL_PROGRAM_COUNTER    1
#define BSP_CORE_EXC_BUS_ERROR                  2
#define BSP_CORE_EXC_ADDR_ERROR                 3
#define BSP_CORE_EXC_ILL_INSTRUCTION            4
#define BSP_CORE_EXC_DIV_ZERO                   5
#define BSP_CORE_EXC_CHK                        6
#define BSP_CORE_EXC_TRAP                       7
#define BSP_CORE_EXC_PRIV_VIOLATION             8
#define BSP_CORE_EXC_TRACE                      9
#define BSP_CORE_EXC_LINE_1010                 10
#define BSP_CORE_EXC_LINE_1111                 11
#define BSP_CORE_EXC_RSV1                      12
#define BSP_CORE_EXC_COPROC_PROTOCOL_VIOLATION 13
#define BSP_CORE_EXC_FORMAT_ERROR              14
#define BSP_CORE_EXC_UNINITIALIZED_INTERRUPT   15
#define BSP_CORE_EXC_RSV2                      16
#define BSP_CORE_EXC_RSV3                      17
#define BSP_CORE_EXC_RSV4                      18
#define BSP_CORE_EXC_RSV5                      19
#define BSP_CORE_EXC_RSV6                      20
#define BSP_CORE_EXC_RSV7                      21
#define BSP_CORE_EXC_RSV8                      22
#define BSP_CORE_EXC_RSV9                      23
#define BSP_CORE_EXC_SPURIOUS_INT              24
#define BSP_CORE_EXC_LEVEL_1_AUTO              25
#define BSP_CORE_EXC_LEVEL_2_AUTO              26
#define BSP_CORE_EXC_LEVEL_3_AUTO              27
#define BSP_CORE_EXC_LEVEL_4_AUTO              28
#define BSP_CORE_EXC_LEVEL_5_AUTO              29
#define BSP_CORE_EXC_LEVEL_6_AUTO              30
#define BSP_CORE_EXC_LEVEL_7_AUTO              31
#define BSP_CORE_EXC_TRAP_0                    32
#define BSP_CORE_EXC_TRAP_1                    33
#define BSP_CORE_EXC_TRAP_2                    34
#define BSP_CORE_EXC_TRAP_3                    35
#define BSP_CORE_EXC_TRAP_4                    36
#define BSP_CORE_EXC_TRAP_5                    37
#define BSP_CORE_EXC_TRAP_6                    38
#define BSP_CORE_EXC_TRAP_7                    39
#define BSP_CORE_EXC_TRAP_8                    40
#define BSP_CORE_EXC_TRAP_9                    41
#define BSP_CORE_EXC_TRAP_10                   42
#define BSP_CORE_EXC_TRAP_11                   43
#define BSP_CORE_EXC_TRAP_12                   44
#define BSP_CORE_EXC_TRAP_13                   45
#define BSP_CORE_EXC_TRAP_14                   46
#define BSP_CORE_EXC_TRAP_15                   47
#define BSP_CORE_EXC_FP_UNORDERED_COND         48
#define BSP_CORE_EXC_FP_INEXACT                49
#define BSP_CORE_EXC_FP_DIV_ZERO               50
#define BSP_CORE_EXC_FP_UNDERFLOW              51
#define BSP_CORE_EXC_FP_OPERAND_ERROR          52
#define BSP_CORE_EXC_FP_OVERFLOW               53
#define BSP_CORE_EXC_FP_NAN                    54
#define BSP_CORE_EXC_FP_UNIMP_DATA_TYPE        55
#define BSP_CORE_EXC_MMU_CONFIG_ERROR          56
#define BSP_CORE_EXC_MMU_ILL_OPERATION         57
#define BSP_CORE_EXC_MMU_ACCESS_VIOLATION      58
#define BSP_CORE_EXC_RSV10                     59
#define BSP_CORE_EXC_RSV11                     60
#define BSP_CORE_EXC_RSV12                     61
#define BSP_CORE_EXC_RSV13                     62
#define BSP_CORE_EXC_RSV14                     63

#define BSP_MAX_EXCEPTIONS                     256

#define BSP_CORE_EXC(vec_num)                  (unsigned long*)(vec_num << 2)

#ifdef __ASSEMBLER__

.macro BREAKPOINT
         trap IMM(1)
.endm
.macro SYSCALL
         trap IMM(15)
.endm
.macro __CLI
         andiw  IMM(~0x0700),sr
.endm
.macro __STI
         oriw   IMM(0x0700),sr
.endm

#define STKFRM_SIZE 6

#else

#define BREAKPOINT() asm volatile("         trap #1")
#define SYSCALL()    asm volatile("         trap #15")
#define __cli()      asm volatile("         andiw  #~0x0700,%sr")
#define __sti()      asm volatile("         oriw   #0x0700,%sr")

/*
 * Exception Stack Frame for group 1 & 2 exceptions
 */
typedef struct {
    unsigned short sr;
    unsigned short pc_h;
    unsigned short pc_l;
} m68k_grp_1_2_stkfrm;

/*
 * Exception Stack Frame for address and bus error exceptions
 */
typedef struct {
    unsigned short code;
    unsigned short access_h;
    unsigned short access_l;
    unsigned short ir;
    unsigned short sr;
    unsigned short pc_h;
    unsigned short pc_l;    
} m68k_bus_addr_stkfrm;

static inline void __set_sr(unsigned val)
{
    asm volatile (
        "movew   %0,%%sr"
	: /* no outputs */
	: "d" (val)  );
}

static inline unsigned __get_sr(void)
{
    unsigned retval;

    asm volatile (
        "  move   %%sr, %0"
	: "=d" (retval)
	: /* no inputs */  );

    return retval;
}


static inline void __set_usp(void *val)
{
    asm volatile (
        "movel   %0,%%usp"
	: /* no outputs */
	: "a" (val)  );
}

static inline void *__get_usp(void)
{
    void *retval;

    asm volatile (
        "movel   %%usp,%0"
	: "=a" (retval)
	: /* no inputs */  );

    return retval;
}

static inline void __set_sp(void *val)
{
    asm volatile (
        "movel %0, %%a7"
        : /* no outputs */ 
        : "a" (val)  );
}

static inline void *__get_sp(void)
{
    void *retval;

    asm volatile (
        "movel %%a7, %0" 
        : "=a" (retval) 
        : /* no inputs */  );
    return retval;
}


#endif /* !__ASSEMBLER__ */

#define GDB_BREAKPOINT_VECTOR BSP_CORE_EXC_TRAP_1
#define GDB_SYSCALL_VECTOR    BSP_CORE_EXC_TRAP_15

#define GDB_BREAKPOINT_INST_SIZE 2

#ifdef __CPU_MC68328__

#ifdef __BOARD_328ADS__
#define USERSTACK 0x180000
#define SUPERSTACK 0x20000
#endif /* __BOARD_328ADS__ */

/*
 * MC68328 Register Base address
 */
#define MC68328_REG_BASE       0xFFFFF000

/*
 * These are offsets from the REG_BASE defined above.
 *
 * Use these in assembly w/ the appropriate REG_BASE
 * in an address register
 */
#define MC68328_SCR_o               0x000
#define MC68328_GRPBASEA_o          0x100
#define MC68328_GRPBASEB_o          0x102
#define MC68328_GRPBASEC_o          0x104
#define MC68328_GRPBASED_o          0x106
#define MC68328_GRPMASKA_o          0x108
#define MC68328_GRPMASKB_o          0x10A
#define MC68328_GRPMASKC_o          0x10C
#define MC68328_GRPMASKD_o          0x10E
#define MC68328_CSA0_o              0x110
#define MC68328_CSA1_o              0x114
#define MC68328_CSA2_o              0x118
#define MC68328_CSA3_o              0x11C
#define MC68328_CSB0_o              0x120
#define MC68328_CSB1_o              0x124
#define MC68328_CSB2_o              0x128
#define MC68328_CSB3_o              0x12C
#define MC68328_CSC0_o              0x130
#define MC68328_CSC1_o              0x134
#define MC68328_CSC2_o              0x138
#define MC68328_CSC3_o              0x13C
#define MC68328_CSD0_o              0x140
#define MC68328_CSD1_o              0x144
#define MC68328_CSD2_o              0x148
#define MC68328_CSD3_o              0x14C
#define MC68328_IVR_o               0x300
#define MC68328_ICR_o               0x302
#define MC68328_IMR_o               0x304
#define MC68328_IWR_o               0x308
#define MC68328_ISR_o               0x30C
#define MC68328_IPR_o               0x310
#define MC68328_PJDIR_o             0x438
#define MC68328_PJDATA_o            0x439
#define MC68328_PJSEL_o             0x43B
#define MC68328_WDOG_CTL_o          0x618
#define MC68328_USTCNT_o            0x900
#define MC68328_UBAUD_o             0x902
#define MC68328_URX_o               0x904
#define MC68328_URX_DATA_o          0x905
#define MC68328_UTX_o               0x906
#define MC68328_UTX_DATA_o          0x907
#define MC68328_UMISC_o             0x908

#ifndef __ASSEMBLER__
/*
 * These are absolute addresses to the registers.
 *
 * Use these in C.
 */
#define MC68328_SCR             (volatile unsigned char *)(MC68328_REG_BASE + MC68328_SCR_o)
#define MC68328_GRPBASEA        (volatile unsigned short*)(MC68328_REG_BASE + MC68328_GRPBASEA_o)
#define MC68328_GRPBASEB        (volatile unsigned short*)(MC68328_REG_BASE + MC68328_GRPBASEB_o)
#define MC68328_GRPBASEC        (volatile unsigned short*)(MC68328_REG_BASE + MC68328_GRPBASEC_o)
#define MC68328_GRPBASED        (volatile unsigned short*)(MC68328_REG_BASE + MC68328_GRPBASED_o)
#define MC68328_GRPMASKA        (volatile unsigned short*)(MC68328_REG_BASE + MC68328_GRPMASKA_o)
#define MC68328_GRPMASKB        (volatile unsigned short*)(MC68328_REG_BASE + MC68328_GRPMASKB_o)
#define MC68328_GRPMASKC        (volatile unsigned short*)(MC68328_REG_BASE + MC68328_GRPMASKC_o)
#define MC68328_GRPMASKD        (volatile unsigned short*)(MC68328_REG_BASE + MC68328_GRPMASKD_o)
#define MC68328_CSA0            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSA0_o)
#define MC68328_CSA1            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSA1_o)
#define MC68328_CSA2            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSA2_o)
#define MC68328_CSA3            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSA3_o)
#define MC68328_CSB0            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSB0_o)
#define MC68328_CSB1            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSB1_o)
#define MC68328_CSB2            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSB2_o)
#define MC68328_CSB3            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSB3_o)
#define MC68328_CSC0            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSC0_o)
#define MC68328_CSC1            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSC1_o)
#define MC68328_CSC2            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSC2_o)
#define MC68328_CSC3            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSC3_o)
#define MC68328_CSD0            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSD0_o)
#define MC68328_CSD1            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSD1_o)
#define MC68328_CSD2            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSD2_o)
#define MC68328_CSD3            (volatile unsigned long*) (MC68328_REG_BASE + MC68328_CSD3_o)
#define MC68328_IVR             (volatile unsigned char*) (MC68328_REG_BASE + MC68328_IVR_o)
#define MC68328_ICR             (volatile unsigned short*)(MC68328_REG_BASE + MC68328_ICR_o)
#define MC68328_IMR             (volatile unsigned long*) (MC68328_REG_BASE + MC68328_IMR_o)
#define MC68328_IWR             (volatile unsigned long*) (MC68328_REG_BASE + MC68328_IWR_o)
#define MC68328_ISR             (volatile unsigned long*) (MC68328_REG_BASE + MC68328_ISR_o)
#define MC68328_IPR             (volatile unsigned long*) (MC68328_REG_BASE + MC68328_IPR_o)
#define MC68328_PJDIR           (volatile unsigned char*) (MC68328_REG_BASE + MC68328_PJDIR_o)
#define MC68328_PJDATA          (volatile unsigned char*) (MC68328_REG_BASE + MC68328_PJDATA_o)
#define MC68328_PJSEL           (volatile unsigned char*) (MC68328_REG_BASE + MC68328_PJSEL_o)
#define MC68328_WDOG_CTL        (volatile unsigned short*)(MC68328_REG_BASE + MC68328_WDOG_CTL_o)
#define MC68328_USTCNT          (volatile unsigned short*)(MC68328_REG_BASE + MC68328_USTCNT_o)
#define MC68328_UBAUD           (volatile unsigned short*)(MC68328_REG_BASE + MC68328_UBAUD_o)
#define MC68328_URX             (volatile unsigned short*)(MC68328_REG_BASE + MC68328_URX_o)
#define MC68328_URX_DATA        (volatile unsigned char*) (MC68328_REG_BASE + MC68328_URX_DATA_o)
#define MC68328_UTX             (volatile unsigned short*)(MC68328_REG_BASE + MC68328_UTX_o)
#define MC68328_UTX_DATA        (volatile unsigned char*) (MC68328_REG_BASE + MC68328_UTX_DATA_o)
#define MC68328_UMISC           (volatile unsigned short*)(MC68328_REG_BASE + MC68328_UMISC_o)
#endif /* __ASSEMBLER__ */          

/*
 * Port IO Bit Definitions
 */
#define MC68328_PJ0             (unsigned char)0x01
#define MC68328_PJ1             (unsigned char)0x02
#define MC68328_PJ2             (unsigned char)0x04
#define MC68328_PJ3             (unsigned char)0x08
#define MC68328_PJ4             (unsigned char)0x10
#define MC68328_PJ5             (unsigned char)0x20
#define MC68328_PJ6             (unsigned char)0x40
#define MC68328_PJ7             (unsigned char)0x80

/*
 * Dragonball specific interrupt numbers
 */
#define BSP_68328_IRQ_FIRST     BSP_MAX_EXCEPTIONS

#define BSP_68328_IRQ_SPIM      (BSP_68328_IRQ_FIRST +  0)
#define BSP_68328_IRQ_TMR2      (BSP_68328_IRQ_FIRST +  1)
#define BSP_68328_IRQ_UART      (BSP_68328_IRQ_FIRST +  2)
#define BSP_68328_IRQ_WDT       (BSP_68328_IRQ_FIRST +  3)
#define BSP_68328_IRQ_RTC       (BSP_68328_IRQ_FIRST +  4)
#define BSP_68328_IRQ_RSVD1     (BSP_68328_IRQ_FIRST +  5)
#define BSP_68328_IRQ_KB        (BSP_68328_IRQ_FIRST +  6)
#define BSP_68328_IRQ_PWM       (BSP_68328_IRQ_FIRST +  7)
#define BSP_68328_IRQ_INT0      (BSP_68328_IRQ_FIRST +  8)
#define BSP_68328_IRQ_INT1      (BSP_68328_IRQ_FIRST +  9)
#define BSP_68328_IRQ_INT2      (BSP_68328_IRQ_FIRST + 10)
#define BSP_68328_IRQ_INT3      (BSP_68328_IRQ_FIRST + 11)
#define BSP_68328_IRQ_INT4      (BSP_68328_IRQ_FIRST + 12)
#define BSP_68328_IRQ_INT5      (BSP_68328_IRQ_FIRST + 13)
#define BSP_68328_IRQ_INT6      (BSP_68328_IRQ_FIRST + 14)
#define BSP_68328_IRQ_INT7      (BSP_68328_IRQ_FIRST + 15)
#define BSP_68328_IRQ_IRQ1      (BSP_68328_IRQ_FIRST + 16)
#define BSP_68328_IRQ_IRQ2      (BSP_68328_IRQ_FIRST + 17)
#define BSP_68328_IRQ_IRQ3      (BSP_68328_IRQ_FIRST + 18)
#define BSP_68328_IRQ_IRQ6      (BSP_68328_IRQ_FIRST + 19)
#define BSP_68328_IRQ_PEN       (BSP_68328_IRQ_FIRST + 20)
#define BSP_68328_IRQ_SPIS      (BSP_68328_IRQ_FIRST + 21)
#define BSP_68328_IRQ_TMR1      (BSP_68328_IRQ_FIRST + 22)
#define BSP_68328_IRQ_IRQ7      (BSP_68328_IRQ_FIRST + 23)
                                
#define BSP_68328_IRQ_LAST      BSP_68328_IRQ_IRQ7

/*
 * Map the Dragonball specific interrupt numbers to IMR bit positions
 */
#define BSP_68328_INT_MASK(irq_nr)    (1 << (irq_nr))

#ifdef __ASSEMBLER__
/*
 * Use the following code for debugging by toggling a port pin
 */
.macro PORT_TOGGLE_DEBUG
    moveal  IMM(MC68328_REG_BASE), a5
    /*
     * Setup PJ3 as GPIO output
     */
    bset    IMM(3), a5@(MC68328_PJDIR_o)
    bset    IMM(3), a5@(MC68328_PJSEL_o)

    /*
     * Toggle a Port PIN Continually
     */
0:  eorib   IMM(0x08), a5@(MC68328_PJDATA_o)
    movel   IMM(0xFFFFF), d1
1:  dbra    d1, 1b
    bra     0b
.endm
#else
#define PORT_TOGGLE_DEBUG() asm volatile ("
    moveal  #0xFFFFF000, %a5
    /*
     * Setup PJ3 as GPIO output
     */
    bset    #3, %a5@(0x438)
    bset    #3, %a5@(0x43B)

    /*
     * Toggle a Port PIN Continually
     */
0:  eorib   #0x08, %a5@(0x439)
    movel   #0xFFFFF, %d1
1:  dbra    %d1, 1b
    bra     0b
");
#endif /* __ASSEMBLER__ */

#endif /* __CPU_MC68328__ */

#endif /* __CPU_H__ */
