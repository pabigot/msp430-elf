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

#include <bsp/bsp.h>
#include <bsp/defs.h>

/*
 * __NEED_UNDERSCORE__ is not defined for any FR30 targets
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
    _\name:
.endm
#else
.macro FUNC_START name
	.global \name
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

#endif /* __ASSEMBLER__ */

#ifndef __ASSEMBLER__

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
    REG_PC,     REG_PS,     REG_TBR,    REG_RP,
    REG_SSP,    REG_USP,    REG_MDH,    REG_MDL,
    REG_LAST
};
#endif


/*
 *  breakpoint opcode  "INT #9".
 */
#define BREAKPOINT_OPCODE	0x1f09

#ifdef __ASSEMBLER__

.macro BREAKPOINT
         .short 0x1f09
.endm
.macro SYSCALL
         int #10
.endm
.macro __CLI
        mov     PS,r0
        ldi:8   #0x10,r1
        or      r1,r0
        mov     r0,PS
        stilm   #30
.endm
.macro __STI
        mov     PS,r0
        ldi:32  #~0x10,r1
        and     r1,r0
        mov     r0,PS
        stilm   #30
.endm

#define STKFRM_SIZE 6

#else

#define BREAKPOINT() asm volatile("         .short 0x1f09")
#define SYSCALL()    asm volatile("         int #10")
#define __cli()      asm volatile("         mov     PS,r0
                                            ldi:8   #0x10,r1
                                            or      r1,r0
                                            mov     r0,PS
                                            stilm   #30")
#define __sti()      asm volatile("         mov     PS,r0
                                            ldi:32  #~0x10,r1
                                            and     r1,r0
                                            mov     r0,PS
                                            stilm   #30")

#endif /* __ASSEMBLER__ */


/*
 * Core Exception vectors.
 */
#define BSP_EXC_RESET      0
#define BSP_EXC_INT9	   9
#define BSP_EXC_INT10     10
#define BSP_EXC_TRACE     12
#define BSP_EXC_ILL       14
#define BSP_EXC_NMI       15
#define BSP_EXC_EXTINT0   16
#define BSP_EXC_EXTINT1   17
#define BSP_EXC_EXTINT2   18
#define BSP_EXC_EXTINT3   19
#define BSP_EXC_UART0_RX  20
#define BSP_EXC_UART1_RX  21
#define BSP_EXC_UART2_RX  22
#define BSP_EXC_UART0_TX  23
#define BSP_EXC_UART1_TX  24
#define BSP_EXC_UART2_TX  25
#define BSP_EXC_DMAC0     26
#define BSP_EXC_DMAC1     27
#define BSP_EXC_DMAC2     28
#define BSP_EXC_DMAC3     29
#define BSP_EXC_DMAC4     30
#define BSP_EXC_DMAC5     31
#define BSP_EXC_DMAC6     32
#define BSP_EXC_DMAC7     33
#define BSP_EXC_ADC       34
#define BSP_EXC_RELOAD0   35
#define BSP_EXC_RELOAD1   36
#define BSP_EXC_RELOAD2   37
#define BSP_EXC_PWM0      38
#define BSP_EXC_PWM1      39
#define BSP_EXC_PWM2      40
#define BSP_EXC_PWM3      41
#define BSP_EXC_UTIMER0   42
#define BSP_EXC_UTIMER1   43
#define BSP_EXC_UTIMER2   44
#define BSP_EXC_DELAY     63
#define BSP_EXC_INT66     66

#define BSP_MAX_EXCEPTIONS 256


#define FRAME_MDH  56
#define FRAME_MDL  60
#define FRAME_TBR  64
#define FRAME_USP  68
#define FRAME_SSP  72
#define FRAME_R14  76
#define FRAME_R0   80
#define FRAME_RP   84
#define FRAME_PC   88
#define FRAME_PS   92

#define FRAME_SIZE 96

#ifndef __ASSEMBLER__
/*
 *  How registers are stored for exceptions.
 */
typedef struct
{
    unsigned long _r15;
    unsigned long _r1;
    unsigned long _r2;
    unsigned long _r3;
    unsigned long _r4;
    unsigned long _r5;
    unsigned long _r6;
    unsigned long _r7;
    unsigned long _r8;
    unsigned long _r9;
    unsigned long _r10;
    unsigned long _r11;
    unsigned long _r12;
    unsigned long _r13;
    unsigned long _mdh;
    unsigned long _mdl;
    unsigned long _tbr;
    unsigned long _usp;
    unsigned long _ssp;
    unsigned long _r14;
    unsigned long _r0;
    unsigned long _rp;
    unsigned long _pc;
    unsigned long _ps;
} ex_regs_t;


extern void __icache_flush(void *addr, int nbytes);
extern void __icache_disable(void);
extern void __icache_enable(void);


typedef volatile unsigned char  reg8_t;
typedef volatile unsigned short reg16_t;
typedef volatile unsigned long  reg32_t;

/*
 * Layout of internal FR30 UART.
 */
typedef struct {
    reg8_t	ssr;	/* status register  */
#define UART_SSR_TIE       0x01
#define UART_SSR_RIE       0x02
#define UART_SSR_TDRE      0x08
#define UART_SSR_RDRF      0x10
#define UART_SSR_FRE       0x20
#define UART_SSR_ORE       0x40
#define UART_SSR_PE        0x80

    reg8_t	data;   /* data register    */
    reg8_t	scr;	/* control register */
#define UART_SCR_TXE	   0x01
#define UART_SCR_RXE	   0x02
#define UART_SCR_REC	   0x04
#define UART_SCR_AD	   0x08
#define UART_SCR_7BIT	   0x00
#define UART_SCR_8BIT	   0x10
#define UART_SCR_1STOP	   0x00
#define UART_SCR_2STOP	   0x20
#define UART_SCR_EVEN      0x80
#define UART_SCR_ODD       0xC0
#define UART_SCR_NONE      0x00

    reg8_t	smr;	/* mode register    */
#define UART_SMR_SOE	   0x01
#define UART_SMR_SCKE	   0x02
#define UART_SMR_CS0       0x08
#define UART_SMR_ASYNC     0x00
#define UART_SMR_ASYNC_MP  0x40
#define UART_SMR_SYNC      0x80
} fr30_uart_t;


/*
 * Layout of internal FR30 reload timer.
 */
typedef struct {
    reg16_t	tmrlr;	/* 16-bit reload register */
    reg16_t	tmr;	/* 16-bit count register  */
    reg16_t	rsrvd;
    reg16_t	tmcsr;  /* control/status register */
#define RTIMER_TRG      0x0001
#define RTIMER_CNTE     0x0002
#define RTIMER_UF       0x0004
#define RTIMER_INTE     0x0008
#define RTIMER_RELD     0x0010
#define RTIMER_CLK2     0x0000
#define RTIMER_CLK8     0x0400
#define RTIMER_CLK32    0x0800
} fr30_rtimer_t;

/*
 * Layout of internal FR30 uart timer.
 */
typedef struct {
    reg16_t	utim;	/* 16-bit reload(w)/cnt(r) register */
    reg8_t	rsrvd;
    reg8_t	utimc;  /* control register */
#define UTIMER_UTCR     0x01
#define UTIMER_UTST     0x02
#define UTIMER_UNDR     0x08
#define UTIMER_UTIE     0x10
#define UTIMER_UCC1     0x80
} fr30_utimer_t;


/*
 * Layout of internal FR30 A/D converter.
 */
typedef struct {
    reg16_t	adcr;   /* data register */
    reg16_t	adcs;	/* control/status register  */
#define ADC_ANE0    0x00
#define ADC_ANE1    0x01
#define ADC_ANE2    0x02
#define ADC_ANE3    0x03
#define ADC_ANS0    0x00
#define ADC_ANS1    0x08
#define ADC_ANS2    0x10
#define ADC_ANS3    0x18
} fr30_adc_t;


/*
 * Layout of internal FR30 PWM timer.
 */
typedef struct {
    reg16_t	ptmr;
    reg16_t     pcsr;
    reg16_t	pdut;
    reg8_t	pcnh;
    reg8_t	pcnl;
} fr30_ptimer_t;

/*
 * Layout of internal FR30 DMA controller.
 */
typedef struct {
    reg32_t	dpdp;
    reg32_t     dacsr;
    reg32_t	datcr;
} fr30_dmac_t;


/*
 * Layout of internal FR30 Bus I/F controller.
 */
typedef struct {
    reg16_t	asr1;
    reg16_t	amr1;
    reg16_t	asr2;
    reg16_t	amr2;
    reg16_t	asr3;
    reg16_t	amr3;
    reg16_t	asr4;
    reg16_t	amr4;
    reg16_t	asr5;
    reg16_t	amr5;
    reg8_t	amd0;
    reg8_t	amd1;
    reg8_t	amd32;
    reg8_t	amd4;
    reg8_t	amd5;
    reg8_t	dscr;
    reg16_t	rfcr;
    reg16_t	epcr0;
    reg16_t	epcr1;
    reg16_t	dmcr4;
    reg16_t	dmcr5;
} fr30_busif_t;


/*
 * Internal memory mapped registers on Fujitsu MB91101 CPU.
 */
typedef struct {
    reg8_t	  rsrvd0;
    reg8_t	  pdr2;
    reg16_t	  rsrvd2;
    reg8_t	  rsrvd4;
    reg8_t	  pdr6;
    reg16_t	  rsrvd6;
    reg8_t	  pdrb;
    reg8_t	  pdra;
    reg8_t	  rsrvd10;
    reg8_t	  pdr8;
    reg32_t	  rsrvd12;
    reg16_t	  rsrvd16;
    reg8_t	  pdre;
    reg8_t	  pdrf;
    reg32_t	  rsrvd20;
    reg32_t	  rsrvd24;
    fr30_uart_t   uart0;
    fr30_uart_t   uart1;
    fr30_uart_t   uart2;
    fr30_rtimer_t rtimer0;
    fr30_rtimer_t rtimer1;
    fr30_adc_t    adc;
    fr30_rtimer_t rtimer2;
    reg8_t        rsrvd68[0x34];
    fr30_utimer_t utimer0;
    fr30_utimer_t utimer1;
    fr30_utimer_t utimer2;
    reg8_t        rsrvd132[0x10];
    reg8_t	  eirr;
    reg8_t	  enir;
    reg16_t	  rsrvd150;
    reg8_t	  rsrvd152;
    reg8_t	  elvr;
    reg16_t	  rsrvd154;
    reg8_t        rsrvd156[0x36];
    reg8_t	  ddre;
    reg8_t	  ddrf;
    reg8_t        rsrvd212[3];
    reg8_t	  aic;
    reg32_t	  rsrvd216;
    reg16_t	  pwm_gcn1;
    reg8_t        rsrvd222;
    reg8_t	  pwm_gcn2;
    fr30_ptimer_t pwm0;
    fr30_ptimer_t pwm1;
    fr30_ptimer_t pwm2;
    fr30_ptimer_t pwm3;
    reg8_t        rsrvd256[0x100];
    fr30_dmac_t   dmac;
    reg8_t        rsrvd524[0x1db];
    reg8_t	  ichcr;
    reg32_t	  rsrvd1000;
    reg32_t	  rsrvd1004;
    reg32_t	  bsd0;
    reg32_t	  bsd1;
    reg32_t	  bsdc;
    reg32_t	  bsrr;
    reg8_t	  icr[48];
    reg8_t	  dicr;
    reg8_t	  hrcl;
    reg8_t	  rsrvd1074[0x4e];
    reg8_t	  wtcr;
    reg8_t	  stcr;
    reg8_t	  pdrr;
    reg8_t	  ctbr;
    reg8_t	  gcr;
    reg8_t	  wpr;
    reg16_t       rsrvd1158;
    reg8_t	  pctr;
    reg8_t	  rsrvd1161[0x178];
    reg8_t	  ddr2;
    reg8_t	  rsrvd1538[3];
    reg8_t	  ddr6;
    reg16_t	  rsrvd1542;
    reg8_t	  ddrb;
    reg8_t	  ddra;
    reg8_t	  rsrvd1545;
    reg8_t	  ddr8;
    fr30_busif_t  bus;
    reg8_t	  rsrvd1584[0x1ce];
    reg8_t	  ler;
    reg8_t	  modr;
} fr30_map_t;


#define fr30_io ((fr30_map_t *)0)


static inline unsigned __get_ps(void)
{
    unsigned retval;

    asm volatile (
        "mov    ps,%0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

static inline void __set_ps(unsigned val)
{
    asm volatile (
        "mov    %0,ps\n"
	: /* no outputs */
	: "r" (val) );
}

#define IRQ_FLAG unsigned
#define DISABLE_IRQ(x) { x = __get_ps(); __set_ps(x & ~0x10); }
#define RESTORE_IRQ(x) __set_ps(x)
 
static inline unsigned __get_ssp(void)
{
    unsigned retval;

    asm volatile (
        "mov    ssp,%0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

static inline unsigned __get_usp(void)
{
    unsigned retval;

    asm volatile (
        "mov    usp,%0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}



#endif /* !__ASSEMBLER__ */


#endif /* __CPU_H__ */
