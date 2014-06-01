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
#if !defined(__CPU_H__)
#define __CPU_H__ 1

/*
 * __NEED_UNDERSCORE__ is not defined for any PPC targets
 */
#ifdef __NEED_UNDERSCORE__
#   error __NEED_UNDERSCORE__ is defined and should not be.
#endif /* __NEED_UNDERSCORE__ */

/*
 * Macros to glue together two tokens.
 */
#  ifdef __STDC__
#    define XGLUE(a,b) a##b
#  else /* __STDC__ */
#    define XGLUE(a,b) a/**/b
#  endif /* __STDC__ */

#  define GLUE(a,b) XGLUE(a,b)

/*
 * Symbol Names with leading underscore if necessary
 */
#  ifdef __NEED_UNDERSCORE__
#    define SYM_NAME(name) GLUE(_,name)
#  else /* __NEED_UNDERSCORE__ */
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
	.type _\name,@function
	.globl _\name
_\name:
.endm
#else /* __NEED_UNDERSCORE__ */
.macro FUNC_START name
	.type \name,@function
	.globl \name
\name:
.endm
#endif /* __NEED_UNDERSCORE__ */

/*
 * Assembly function end definition
 */
#ifdef __NEED_UNDERSCORE__
.macro FUNC_END name
.L_\name: 
	.size _\name,.L_\name-_\name
.endm
#else /* __NEED_UNDERSCORE__ */
.macro FUNC_END name
.L\name: 
	.size \name,.L\name-\name
.endm
#endif /* __NEED_UNDERSCORE__ */

/*
 * Macros to begin and end a function written in assembler.  If -mcall-aixdesc
 * or -mcall-nt, create a function descriptor with the given name, and create
 * the real function with one or two leading periods respectively.
 */

#ifdef _RELOCATABLE
#  define DESC_SECTION ".got2"
#else /* _RELOCATABLE */
#  define DESC_SECTION ".got1"
#endif /* _RELOCATABLE */

/*
 * Macro to load immediate word.
 */
.macro	lwi	reg,val
	lis	\reg,\val@h
	ori	\reg,\reg,\val@l
.endm

#endif /* defined(__ASSEMBLER__) */


/* Easy to read register names. */
#define r0	0
#define r1	1
#define r2	2
#define r3	3
#define r4	4
#define r5	5
#define r6	6
#define r7	7
#define r8	8
#define r9	9
#define r10	10
#define r11	11
#define r12	12
#define r13	13
#define r14	14
#define r15	15
#define r16	16
#define r17	17
#define r18	18
#define r19     19
#define r20	20
#define r21	21
#define r22	22
#define r23	23
#define r24	24
#define r25	25
#define r26	26
#define r27	27
#define r28	28
#define r29	29
#define r30	30
#define r31	31

#define sp	r1

#define cr0	0
#define cr1	1
#define cr2	2
#define cr3	3
#define cr4	4
#define cr5	5
#define cr6	6
#define cr7	7

#define f0	0
#define f1	1
#define f2	2
#define f3	3
#define f4	4
#define f5	5
#define f6	6
#define f7	7
#define f8	8
#define f9	9
#define f10	10
#define f11	11
#define f12	12
#define f13	13
#define f14	14
#define f15	15
#define f16	16
#define f17	17
#define f18	18
#define f19     19
#define f20	20
#define f21	21
#define f22	22
#define f23	23
#define f24	24
#define f25	25
#define f26	26
#define f27	27
#define f28	28
#define f29	29
#define f30	30
#define f31	31

/* Special Purpose Registers. */
#define xer     	1	/* fixed-point exception cause */
#define lr		8	/* link register */
#define ctr		9	/* count register */
#define dsisr		18	/* data storage interrupt status */
#define dar		19	/* data address register */
#define dec		22	/* decrementer register */
#define sdr1		25	/* page table address base & mask */
#define srr0		26	/* PC save/restore */
#define srr1		27	/* machine status save/restore */
#define eie		80
#define eid		81
#define nri		82
#define cmpa		144
#define cmpb		145
#define cmpc		146
#define cmpd		147
#define icr		148
#define der		149
#define counta		150
#define countb		151
#define cmpe		152
#define cmpf		153
#define cmpg		154
#define cmph		155
#define lctrl1		156
#define lctrl2		157
#define ictrl		158
#define bar		159
#define tblr		268     /* time base lower read */
#define tbur		269     /* time base upper read */
#define sprg0   	272     /* scratch register 0 */
#define sprg1   	273     /* scratch register 1 */
#define sprg2   	274     /* scratch register 2 */
#define sprg3   	275     /* scratch register 3 */
#define ear		282     /* external address register */
#if defined(__CPU_PPC4XX__)
#  define icdbdr        979
#  define esr           980
#  define dear          981
#  define evpr          982
#  define cdbcr         983
#  define tsr           984
#  define tcr           986
#  define pit           987
#  define tblw		988     /* time base    write */
#  define tbuw		989     /* time base    write */
#  define srr2		990	/* critical irq PC    restore */
#  define srr3		991	/* critical irq machine status  restore */
#  define dbsr         1008
#  define dbcr         1010
#  define iac1         1012
#  define iac2         1013
#  define dac1         1014
#  define dac2         1015
#  define dccr         1018
#  define iccr         1019
#  define pbl1         1020
#  define pbu1         1021
#  define pbl2         1022
#  define pbu2         1023
#else /* defined(__CPU_PPC4XX__) */
#  define tblw		284     /* time base    write */
#  define tbuw		285     /* time base    write */
#endif /* defined(__CPU_PPC4XX__) */
#define pvr     	287	/* processor version */

#define ibat0u		528	/* instruction bat registers */
#define ibat0l		529
#define ibat1u		530
#define ibat1l		531
#define ibat2u		532
#define ibat2l		533
#define ibat3u		534
#define ibat3l		535
#define dbat0u		536	/* data bat registers */
#define dbat0l		537
#define dbat1u		538
#define dbat1l		539
#define dbat2u		540
#define dbat2l		541
#define dbat3u		542
#define dbat3l		543

#define ic_cst		560
#define ic_adr		561
#define ic_dat		562

#define dc_cst		568
#define dc_adr		569
#define dc_dat		570

#define dpdr		630
#define dpir		631

#define immr		638

#define mi_ctr		784
#define mi_ap		786
#define mi_epn		787
#define mi_twc		789
#define mi_l1dl2p	789
#define mi_rpn		790

#define md_ctr		792
#define m_casid		793
#define md_ap		794
#define md_epn		795
#define m_twb		796
#define md_l1p		796
#define md_twc		797
#define md_l1dl2p	797
#define md_rpn		798
#define m_tw		799
#define m_save		799

#define mi_dbcam	816
#define mi_dbram0	817
#define mi_dbram1	818

#define md_dbcam	824
#define md_dbram0	825
#define md_dbram1	826

#define mmcr0		952
#define pmc1		953
#define pmc2		954
#define sia		955	/* sampled instruction address */
#define sda		959	/* sampled data address */
#define hid0		1008	/* hardware implementation register */
#define iabr		1010	/* instruction address breakpoint register */
#define dabr		1013	/* data address breakpoint register */
#define pir		1023	/* processor identification register */

/* Bit encodings for Machine State Register (MSR) */
#if defined(__CPU_PPC4XX__)
#  define MSR_WE	(1<<18)		/* Wait state enable */
#  define MSR_CE	(1<<17)		/* Critical interrupt enable */
#else /* defined(__CPU_PPC4XX__) */
#  define MSR_POW	(1<<18)		/* Enable Power Management */
#  define MSR_TGPR      (1<<17)		/* TLB Update registers in use */
#endif /* defined(__CPU_PPC4XX__) */
#define MSR_ILE		(1<<16)		/* Interrupt Little-Endian enable */
#define MSR_EE		(1<<15)		/* External Interrupt enable */
#define MSR_PR		(1<<14)		/* Supervisor/User privilege */
#define MSR_FP		(1<<13)		/* Floating Point enable */
#define MSR_ME		(1<<12)		/* Machine Check enable */
#define MSR_FE0		(1<<11)		/* Floating Exception mode 0 */
#define MSR_SE		(1<<10)		/* Single Step */
#if defined(__CPU_PPC4XX__)
#  define MSR_DE	(1<<9)		/* Debug exception enable */
#else /* defined(__CPU_PPC4XX__) */
#  define MSR_BE	(1<<9)		/* Branch Trace */
#endif /* defined(__CPU_PPC4XX__) */
#define MSR_FE1		(1<<8)		/* Floating Exception mode 1 */
#define MSR_IP		(1<<6)		/* Exception prefix 0x000/0xFFF */
#define MSR_IR		(1<<5)		/* Instruction MMU enable */
#define MSR_DR		(1<<4)		/* Data MMU enable */
#if defined(__CPU_PPC4XX__)
#  define MSR_PE	(1<<3)		/* Protect enable. */
#  define MSR_PX        (1<<2)		/* Protect enable. */
#endif /* defined(__CPU_PPC4XX__) */
#define MSR_RI		(1<<1)		/* Recoverable Exception */
#define MSR_LE		(1<<0)		/* Little-Endian enable */


/* Bit encodings for Hardware Implementation Register (HID0) (604 specific) */
#define HID0_EMCP	(1<<31)		/* Enable Machine Check pin */
#define HID0_EBA	(1<<29)		/* Enable Bus Address Parity */
#define HID0_EBD	(1<<28)		/* Enable Bus Data Parity */
#define HID0_SBCLK	(1<<27)
#define HID0_EICE	(1<<26)
#define HID0_ECLK	(1<<25)
#define HID0_PAR	(1<<24)
#define HID0_DOZE	(1<<23)
#define HID0_NAP	(1<<22)
#define HID0_SLEEP	(1<<21)
#define HID0_DPM	(1<<20)
#define HID0_ICE	(1<<15)		/* Instruction Cache Enable */
#define HID0_DCE	(1<<14)		/* Data Cache Enable */
#define HID0_ILOCK	(1<<13)		/* Instruction Cache Lock */
#define HID0_DLOCK	(1<<12)		/* Data Cache Lock */
#define HID0_ICFI	(1<<11)		/* Instruction Cache Flash Invalidate */
#define HID0_DCI	(1<<10)		/* Data Cache Invalidate */
#define HID0_SIED	(1<<7)		/* Serial Instruction Execution [Disable] */
#define HID0_BHTE	(1<<2)		/* Branch History Table Enable */

#if !defined(__ASSEMBLER__)

/*
 * Register name that is used in help strings and such
 */
# define REGNAME_EXAMPLE "r0"

/*
 *  Register numbers. These are assumed to match the
 *  register numbers used by GDB.
 */
enum __regnames {
    REG_R0,     REG_R1,     REG_R2,     REG_R3,
    REG_R4,     REG_R5,     REG_R6,     REG_R7,
    REG_R8,     REG_R9,     REG_R10,    REG_R11,
    REG_R12,    REG_R13,    REG_R14,    REG_R15,
    REG_R16,    REG_R17,    REG_R18,    REG_R19,
    REG_R20,    REG_R21,    REG_R22,    REG_R23,
    REG_R24,    REG_R25,    REG_R26,    REG_R27,
    REG_R28,    REG_R29,    REG_R30,    REG_R31,
    REG_F0,     REG_F1,     REG_F2,     REG_F3,
    REG_F4,     REG_F5,     REG_F6,     REG_F7,
    REG_F8,     REG_F9,     REG_F10,    REG_F11,
    REG_F12,    REG_F13,    REG_F14,    REG_F15,
    REG_F16,    REG_F17,    REG_F18,    REG_F19,
    REG_F20,    REG_F21,    REG_F22,    REG_F23,
    REG_F24,    REG_F25,    REG_F26,    REG_F27,
    REG_F28,    REG_F29,    REG_F30,    REG_F31,

    REG_PC, REG_PS, REG_CND, REG_LR, REG_CNT, REG_XER, REG_MQ
};
#endif /* __ASSEMBLER__ */

#if defined(__ASSEMBLER__)
  .macro BREAKPOINT
         .long 0x7d821008
  .endm
  .macro SYSCALL
         sc
  .endm
  .macro __CLI
         mfmsr   3
         ori     3,3,0x8000
         mtmsr   3
  .endm
  .macro __STI
         mfmsr   0
         rlwinm  1,0,0,17,15
         mtmsr   1
  .endm

#else /* !defined(__ASSEMBLER__) */

#  define BREAKPOINT()	asm volatile (".long 0x7d821008\n")
#  define SYSCALL()     asm volatile ("sc")
#  define __cli()       asm volatile ("mfmsr   3
                                       ori     3,3,0x8000
                                       mtmsr   3" : /* no outputs */ : /* no inputs */ )
#  define __sti()       asm volatile ("mfmsr   0
                                       rlwinm  1,0,0,17,15
                                       mtmsr   1" : /* no outputs */ : /* no inputs */ : "1" )
#endif /* !defined(__ASSEMBLER__) */

/*
 *  breakpoint opcode.
 */
#define BREAKPOINT_OPCODE	0x7d821008

/*
 * Core Exception vectors.
 */
#define BSP_EXC_RESET	    1
#define BSP_EXC_CRITICAL    1	/* on 403, this is critical interrupt */
#define BSP_EXC_MCHK	    2
#define BSP_EXC_DSI	    3
#define BSP_EXC_PROTECT	    3	/* on 403, this is protection violation */
#define BSP_EXC_ISI	    4
#define BSP_EXC_EXTIRQ	    5
#define BSP_EXC_ALIGN	    6
#define BSP_EXC_TRAP	    7
#define BSP_EXC_NOFP	    8
#define BSP_EXC_DECR	    9
#define BSP_EXC_SYSCALL    12
#define BSP_EXC_TRACE      13
#define BSP_EXC_FPASSIST   14

#if defined(__CPU_PPC4XX__)
#  define BSP_EXC_PIT      16
#  define BSP_EXC_FIT      17
#  define BSP_EXC_WATCHDOG 18
#  define BSP_EXC_DEBUG    32
#endif /* defined(__CPU_PPC4XX__) */

#if defined(__CPU_MPC8XX__)
/*
 * MPC8xx defined exceptions.
 */
#  define BSP_EXC_EMUL     16
#  define BSP_EXC_IMISS    17
#  define BSP_EXC_DMISS    18
#  define BSP_EXC_IERR     19
#  define BSP_EXC_DERR     20
#  define BSP_EXC_DATABP   28
#  define BSP_EXC_INSNBP   29
#  define BSP_EXC_PERIPHBP 30
#  define BSP_EXC_DEVPORT  31
#endif /* defined(__CPU_MPC8XX__) */

/* PPC architectural limitation */
#define BSP_MAX_EXCEPTIONS 64

/*
 *  Core interrupt.
 */
#define BSP_IRQ_CORE	0

#if defined(__CPU_MPC8XX__)

#  include <bsp/ppc8xx.h>

/*
 *  MPC8XX SIU Interrupts.
 */
#  define BSP_IRQ_LVL7	1
#  define BSP_IRQ_HW7	2
#  define BSP_IRQ_LVL6	3
#  define BSP_IRQ_HW6	4
#  define BSP_IRQ_LVL5	5
#  define BSP_IRQ_HW5	6
#  define BSP_IRQ_LVL4	7
#  define BSP_IRQ_HW4	8
#  define BSP_IRQ_LVL3  9
#  define BSP_IRQ_HW3	10
#  define BSP_IRQ_LVL2  11
#  define BSP_IRQ_HW2	12
#  define BSP_IRQ_LVL1  13
#  define BSP_IRQ_HW1	14
#  define BSP_IRQ_LVL0  15
#  define BSP_IRQ_HW0	16

/*
 *  Mapping of SIU levels to actual sources.
 */
#  define BSP_IRQ_CPM	BSP_IRQ_LVL4
#  define BSP_IRQ_PCMIA	BSP_IRQ_LVL3
#  define BSP_IRQ_TB	BSP_IRQ_LVL2
#  define BSP_IRQ_RTC	BSP_IRQ_LVL1
#  define BSP_IRQ_PIT	BSP_IRQ_LVL0

/*
 *  MPC8XX CPM Interrupts.
 */
#  define BSP_IRQ_RESERVED1	17
#  define BSP_IRQ_PC4		18
#  define BSP_IRQ_PC5	       	19
#  define BSP_IRQ_SMC2       	20
#  define BSP_IRQ_SMC1       	21
#  define BSP_IRQ_SPI       	22
#  define BSP_IRQ_PC6	       	23
#  define BSP_IRQ_TIMER4     	24
#  define BSP_IRQ_RESERVED2	25
#  define BSP_IRQ_PC7	       	26
#  define BSP_IRQ_PC8	       	27
#  define BSP_IRQ_PC9	       	28
#  define BSP_IRQ_TIMER3     	29
#  define BSP_IRQ_RESERVED3	30
#  define BSP_IRQ_PC10       	31
#  define BSP_IRQ_PC11       	32
#  define BSP_IRQ_I2C       	33
#  define BSP_IRQ_RISCTIMER  	34
#  define BSP_IRQ_TIMER2        35
#  define BSP_IRQ_RESERVED4	36
#  define BSP_IRQ_IDMA2		37
#  define BSP_IRQ_IDMA1		38
#  define BSP_IRQ_SDMABUS   	39
#  define BSP_IRQ_PC12       	40
#  define BSP_IRQ_PC13       	41
#  define BSP_IRQ_TIMER1	42
#  define BSP_IRQ_PC14       	43
#  define BSP_IRQ_SCC4		44
#  define BSP_IRQ_SCC3		45
#  define BSP_IRQ_SCC2		46
#  define BSP_IRQ_SCC1		47
#  define BSP_IRQ_PC15		48
#endif /* defined(__CPU_MPC8XX__) */


#if defined(__CPU_PPC4XX__)

#  include <bsp/ppc4xx.h>

#  define BSP_IRQ_RSRVD1	1
#  define BSP_IRQ_RSRVD2	2
#  define BSP_IRQ_RSRVD3	3
#  define BSP_IRQ_SERIAL_RX	4
#  define BSP_IRQ_SERIAL_TX	5
#  define BSP_IRQ_JTAG_RX	6
#  define BSP_IRQ_JTAG_TX	7
#  define BSP_IRQ_DMA0		8
#  define BSP_IRQ_DMA1		9
#  define BSP_IRQ_DMA2	       10
#  define BSP_IRQ_DMA3	       11
#  define BSP_IRQ_RSRVD12      12
#  define BSP_IRQ_RSRVD13      13
#  define BSP_IRQ_RSRVD14      14
#  define BSP_IRQ_RSRVD15      15
#  define BSP_IRQ_RSRVD16      16
#  define BSP_IRQ_RSRVD17      17
#  define BSP_IRQ_RSRVD18      18
#  define BSP_IRQ_RSRVD19      19
#  define BSP_IRQ_RSRVD20      20
#  define BSP_IRQ_RSRVD21      21
#  define BSP_IRQ_RSRVD22      22
#  define BSP_IRQ_RSRVD23      23
#  define BSP_IRQ_RSRVD24      24
#  define BSP_IRQ_RSRVD25      25
#  define BSP_IRQ_RSRVD26      26
#  define BSP_IRQ_EXT0	       27
#  define BSP_IRQ_EXT1	       28
#  define BSP_IRQ_EXT2	       29
#  define BSP_IRQ_EXT3	       30
#  define BSP_IRQ_EXT4	       31

#endif /* defined(__CPU_PPC4XX__) */

#define OVERHEAD_SIZE   64
#define UNDERHEAD_SIZE  16

#define GPR0	OVERHEAD_SIZE
#define GPR1	GPR0+4
#define GPR2	GPR1+4
#define GPR3	GPR2+4
#define GPR4	GPR3+4
#define GPR5	GPR4+4
#define GPR6	GPR5+4
#define GPR7	GPR6+4
#define GPR8	GPR7+4
#define GPR9	GPR8+4
#define GPR10	GPR9+4
#define GPR11	GPR10+4
#define GPR12	GPR11+4
#define GPR13   GPR12+4
#define GPR14	GPR13+4
#define GPR15	GPR14+4
#define GPR16	GPR15+4
#define GPR17	GPR16+4
#define GPR18	GPR17+4
#define GPR19	GPR18+4
#define GPR20	GPR19+4
#define GPR21	GPR20+4
#define GPR22	GPR21+4
#define GPR23	GPR22+4
#define GPR24	GPR23+4
#define GPR25	GPR24+4
#define GPR26	GPR25+4
#define GPR27	GPR26+4
#define GPR28	GPR27+4
#define GPR29	GPR28+4
#define GPR30	GPR29+4
#define GPR31	GPR30+4
#define FPR0	GPR0+(32*4)
#define NIP	FPR0+(32*8)
#define MSR	NIP+4
#define CCR	MSR+4
#define LR	CCR+4
#define CTR	LR+4
#define XER	CTR+4
#define MQ	XER+4
#define TRAP	MQ+4
#define UNDERHEAD TRAP+4

#define EX_STACK_SIZE UNDERHEAD+UNDERHEAD_SIZE


#if !defined(__ASSEMBLER__)
/*
 *  How registers are stored for exceptions.
 *  Since this is going on the stack, *CARE MUST BE TAKEN* to insure
 *  that the overall structure is a multiple of 16 bytes in length.
 */
typedef struct
{
    unsigned long _overhead[OVERHEAD_SIZE / sizeof(unsigned long)];
    unsigned long _gpr[32];
    double	  _fpr[32];
    unsigned long _nip;
    unsigned long _msr;
    unsigned long _ccr;
    unsigned long _lr;
    unsigned long _cnt;
    unsigned long _xer;
    unsigned long _mq;
    unsigned long _trap;
    unsigned long _underhead[UNDERHEAD_SIZE / sizeof(unsigned long)];
} ex_regs_t;


/*
 *  Board specific initialization sets this to value to use for
 *  a rough millisecond timeout for the DEC register.
 */
extern unsigned int  __decr_cnt;

static inline void __set_decr(unsigned val)
{
    asm volatile (
        "mtdec   %0\n"
	: /* no outputs */
	: "r" (val)  );
}

static inline unsigned __get_decr(void)
{
    unsigned retval;

    asm volatile (
        "mfdec   %0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

static inline void __set_msr(unsigned val)
{
    asm volatile (
        "mtmsr   %0\n"
	: /* no outputs */
	: "r" (val)  );
}

static inline unsigned __get_msr(void)
{
    unsigned retval;

    asm volatile (
        "mfmsr   %0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}


#define __sync()     asm volatile ("sync\n")
#define __isync()    asm volatile ("isync\n")
#define __eieio()    asm volatile ("eieio\n")


extern void __dcache_flush(void *addr, int nbytes);
extern void __icache_flush(void *addr, int nbytes);
extern int  __dcache_disable(void);
extern void __dcache_enable(void);
extern void __icache_disable(void);
extern void __icache_enable(void);

#endif /* !defined(__ASSEMBLER__) */


#endif /* __CPU_H__ */
