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
 * __NEED_UNDERSCORE__ is defined for all MN10300 targets
 */
#ifndef __NEED_UNDERSCORE__
#   error __NEED_UNDERSCORE__ is not defined and should be.
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
	.globl _\name
	.align 4
    _\name:
.endm
#else
.macro FUNC_START name
	.globl \name
	.align 4
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
 *  MN10300 Register addresses.
 */
#define IVAR0   0x20000000
#define IVAR1   0x20000004
#define IVAR2   0x20000008
#define IVAR3   0x2000000c
#define IVAR4   0x20000010
#define IVAR5   0x20000014
#define IVAR6   0x20000018

#define ISR	0x20000034

#define	IOBCTR	0x32000010
#define	MEMCTR0	0x32000020
#define	MEMCTR1	0x32000022
#define	MEMCTR2	0x32000024
#define	MEMCTR3	0x32000026
#define	MEMCTR4	0x32000028
#define	MEMCTR5	0x3200002a
#define	MEMCTR6	0x3200002c
#define	MEMCTR7	0x3200002e
#define	DRAMCTR	0x32000040
#define	REFCNT	0x32000042

#define	P2OUT	0x36008004
#define	P2MD	0x36008024
#define	P2SS	0x36008044
#define	P2DIR	0x36008064

#define ICRBASE 0x34000100
#define IAGR    0x34000200

#define TM1MD	0x34001001
#define TM2MD	0x34001002
#define TM3MD	0x34001003
#define TM1BR   0x34001011
#define TM2BR   0x34001012
#define TM3BR   0x34001013
#define TM2BC   0x34001022
#define TM3BC   0x34001023
#define TMPSCNT	0x34001071
#define P3MD	0x36008025

/*
 *  breakpoint opcode.
 */
#define BREAKPOINT_OPCODE	0xff

#ifdef __ASSEMBLER__

.macro BREAKPOINT
         .byte 0xff
.endm
#define SYSCALL syscall
.macro __CLI
         or     0x0800,psw
.endm
.macro __STI
         and    ~0x0800,psw
.endm

#define STKFRM_SIZE 6

#else

#define BREAKPOINT() asm volatile("         .byte 0xff")
#define SYSCALL()    asm volatile("         syscall")
#define __cli()      asm volatile("         or     0x0800,psw")
#define __sti()      asm volatile("         and    ~0x0800,psw")
#define DISABLE_INTERRUPTS(_old_)       \
        asm volatile (                  \
            "mov        psw,%0;"        \
            "mov        0xF7FF,d0;"     \
            "and        %0,d0;"         \
            "mov        d0,psw;"        \
            : "=d"(_old_)               \
            :                           \
            : "d0"                      \
            );

#define ENABLE_INTERRUPTS()             \
        asm volatile (                  \
            "mov        psw,d0;"        \
            "or         0x0800,d0;"     \
            "mov        d0,psw;"        \
            :                           \
            :                           \
            : "d0"                      \
            );

#define RESTORE_INTERRUPTS(_old_)       \
        asm volatile (                  \
            "mov        %0,psw;"        \
            :                           \
            : "d"(_old_)                \
            );

#endif

/*
 * Core Exception vectors.
 *
 * SYSCALL, BREAKPOINT, BUSERR, MEMERR, and ILLEGAL are pseudo exceptions
 * decoded from a syserr interrupt.
 */
#define BSP_EXC_RESET	    0
#define BSP_EXC_TRAP	    1
#define BSP_EXC_SYSCALL	    2
#define BSP_EXC_BREAKPOINT  3
#define BSP_EXC_BUSERR      5	/* alignment */
#define BSP_EXC_MEMERR      6   /* bad memory space */
#define BSP_EXC_ILLEGAL     7   /* illegal instruction */

#define BSP_EXC_IRQ	    8  /* dummy, used by debug stub */

/*
 *  Maximum number of exception vectors.
 */
#define BSP_MAX_EXCEPTIONS 8


/*
 * Interrupts
 */
#define BSP_IRQ_NMI		0
#define BSP_IRQ_WATCHDOG	1
#define BSP_IRQ_TMR0		8
#define BSP_IRQ_TMR1		12
#define BSP_IRQ_TMR2		16
#define BSP_IRQ_TMR3		20
#define BSP_IRQ_TMR4		24
#define BSP_IRQ_TMR5		28
#define BSP_IRQ_TMR6		32
#define BSP_IRQ_TMR6A		36
#define BSP_IRQ_TMR6B		40
#define BSP_IRQ_DMA0		48
#define BSP_IRQ_DMA1		52
#define BSP_IRQ_DMA2		56
#define BSP_IRQ_DMA3		60
#define BSP_IRQ_SIO0_RX		64
#define BSP_IRQ_SIO0_TX		68
#define BSP_IRQ_SIO1_RX		72
#define BSP_IRQ_SIO1_TX		76
#define BSP_IRQ_SIO2_RX		80
#define BSP_IRQ_SIO2_TX		84
#define BSP_IRQ_EXT0		92
#define BSP_IRQ_EXT1		96
#define BSP_IRQ_EXT2		100
#define BSP_IRQ_EXT3		104
#define BSP_IRQ_EXT4		108
#define BSP_IRQ_EXT5		112
#define BSP_IRQ_EXT6		116
#define BSP_IRQ_EXT7		120


#ifdef __ASSEMBLER__

.macro	led val
    movm        [d2],(sp)
    mov         \val,d2
    asl         4,d2
    movbu       d2,(P2OUT)
    movm        (sp),[d2]
.endm

#else

/*
 *  Internal register map and defines.
 */
typedef volatile unsigned char  reg8;
typedef volatile unsigned short reg16;
typedef volatile unsigned int   reg32;

/*
 * Interrupt Vector Address Register layout.
 */
struct _ivar {
    reg16 ivar;
    reg16 pad;
};

/*
 * Cache control way entry contains up to 4 offsets.
 */
struct _way {
    reg32	offset[4];
};

/*
 *  DMA channel registers.
 */
struct _dma {
    reg32	ctrl;		/* DMA control register */
    reg32	src;		/* DMA source register */
    reg32	dest;		/* DMA destination register */
    reg16	cnt;		/* DMA count register */
    reg8	cycle;		/* DMA intermittant cycle register */
    reg8	pad[0xF1];	/* pad to 256 bytes */
};

/*
 * Interrupt Control Register layout.
 */
struct _icr {
    reg16	icr;
    reg16	pad;
};


/*
 *  Serial channel 0,1 layout.
 */
struct _sio {
    reg16	ctrl;		/* uart control register */
#define SIO_CLK_IOCLK_2  0
#define SIO_CLK_IOCLK_8  1
#define SIO_CLK_IOCLK_32 2
#define SIO_CLK_TMR2_2   3
#define SIO_CLK_TMR3_2   3
#define SIO_CLK_TMR0_8   4
#define SIO_CLK_TMR1_8   4
#define SIO_CLK_TMR2_8   5
#define SIO_CLK_TMR3_8   5
#define SIO_CLK_EXT_8    6
#define SIO_CLK_EXT      7

#define SIO_STOP1	0
#define SIO_STOP2	8

#define SIO_PAR_NONE	0x00
#define SIO_PAR_FIX0	0x40
#define SIO_PAR_FIX1	0x50
#define SIO_PAR_EVEN	0x60
#define SIO_PAR_ODD	0x70

#define SIO_DATA_7	0x00
#define SIO_DATA_8	0x80

#define SIO_TOE		0x100

#define SIO_LSB_FIRST	0x000
#define SIO_MSB_FIRST	0x200

#define SIO_MODE_STOP_SYNC 0x000
#define SIO_MODE_CLK_SYNC1 0x400
#define SIO_MODE_I2C       0x800
#define SIO_MODE_CLK_SYNC2 0xC00

#define SIO_I2C_MODE0	0x0000
#define SIO_I2C_MODE1	0x1000

#define SIO_SEND_BRK	0x2000
#define SIO_RX_ENABLE	0x4000
#define SIO_TX_ENABLE	0x8000

    reg16	pad1;
    reg8	icr;		/* interrupt control register */
    reg8	pad2[3];
    reg8	txd;		/* tx data */
    reg8	rxd;		/* rx data */
    reg16	pad3;
    reg16	sts;		/* 16bit status for channel 0,1 */
#define SIO_STS_OEF	1
#define SIO_STS_PEF	2
#define SIO_STS_FEF	4

#define SIO_STS_RXFULL	16
#define SIO_STS_TXFULL	32
#define SIO_STS_RXBUSY	64
#define SIO_STS_TXBUSY	128

    reg16	pad4;
};

/*
 *  Serial channel 2 layout.
 */
struct _sio2 {
    reg16	ctrl;		/* uart control register */
#define SIO2_CLK_IOCLK	0x0000
#define SIO2_CLK_TMR2	0x0001
#define SIO2_CLK_EXT	0x0002
#define SIO2_CLK_TMR3	0x0003

    reg16	pad1;
    reg8	icr;		/* interrupt control register */
    reg8	pad2[3];
    reg8	txd;		/* tx data */
    reg8	rxd;		/* rx data */
    reg16	pad3;
    reg8	sts;		/* 8bit status for channel 2 */
#define SIO_STS_CTS	8
    reg8	tim;		/* channel 2 internal divisor */
    reg16	pad4;
};


struct am30_map {
    struct _ivar ivar[7];		/* interrupt priority 0-6 vector */
    reg16	ivar_pad[2];
    reg16	cpup;			/* CPU pipeline control */
    reg16	cpup_pad[15];
    reg16	cpum;			/* CPU mode */
    reg16	cpum_pad[23];
    reg16	chctr;			/* Cache control */
    reg16	chctr_pad[7];
    reg8	pad1[0x7ffff80];
    struct _way ic_way0_data[128];	/* icache way0 data */
    reg8	pad2[0x800];
    struct _way ic_way1_data[128];	/* icache way1 data */
    reg8	pad3[0xfe800];
    struct _way ic_way0_tags[128];	/* icache way0 tags */
    reg8	pad4[0x800];
    struct _way ic_way1_tags[128];	/* icache way1 tags */
    reg8	pad5[0xfe800];
    struct _way dc_way0_data[128];	/* dcache way0 data */
    reg8	pad6[0x800];
    struct _way dc_way1_data[128];	/* dcache way1 data */
    reg8	pad7[0xfe800];
    struct _way dc_way0_tags[128];	/* dcache way0 tags */
    reg8	pad8[0x800];
    struct _way dc_way1_tags[128];	/* dcache way1 tags */
    reg8	pad9[0xfe800];
    struct _way dc_way0_flush[128];	/* dcache way0 flush regs */
    reg8	pad10[0x800];
    struct _way dc_way1_flush[128];	/* dcache way1 flush regs */
    reg8	pad11[0x9bfe810];
    reg16	iobctr;			/* I/O bus control */
    reg16	pad12[7];
    reg16	memctr[8];		/* Memory control */
    reg16	pad13[8];
    reg16	dramctr;		/* DRAM control */
    reg16	refcnt;			/* refresh counter */
    reg8	pad14[0xbc];
    struct _dma dma0;			/* DMA Channel 0 */
    struct _dma dma1;			/* DMA Channel 1 */
    reg8	pad15[0x100];
    struct _dma dma2;			/* DMA Channel 2 */
    reg8	pad16[0x300];
    struct _dma dma3;			/* DMA Channel 3 */
    reg8	pad17[0x1fff800];
    struct _icr icr[32];		/* interrupt group control */
    reg8	pad18[0x80];
    reg16	iagr;			/* interrupt acceptance group */
    reg16	pad19[0x3f];
    reg16	extmd;			/* external interrupt mode */
    reg8	pad20[0x57d];
    struct _sio sio0;			/* serial channels */
    struct _sio sio1;
    struct _sio2 sio2;
    reg8	pad21[0x7d0];
    reg8	tm0_md;			/* timer 0-3 mode */
    reg8	tm1_md;
    reg8	tm2_md;
    reg8	tm3_md;
#define TMR8_MD_IOCLK  0x00
#define TMR8_MD_TM0    0x01
#define TMR8_MD_TM1    0x02
#define TMR8_MD_EXT    0x03
#define TMR8_MD_LOAD   0x40
#define TMR8_MD_ENABLE 0x80
    reg8	pad22[12];
    reg8	tm0_br;			/* timer 0-3 base */
    reg8	tm1_br;
    reg8	tm2_br;
    reg8	tm3_br;
    reg8	pad23[12];
    reg8	tm0_bc;			/* timer 0-3 binary count */
    reg8	tm1_bc;
    reg8	tm2_bc;
    reg8	tm3_bc;
    reg8	pad24[0x5c];
    reg8	tm4_md;			/* timer 4 mode */
    reg8	pad25;
    reg8	tm5_md;			/* timer 5 mode */
    reg8	pad26;
    reg16	tm6_md;			/* timer 6 mode */
    reg16	pad27[5];
    reg16	tm4_br;			/* timer 4 base */
    reg16	tm5_br;			/* timer 5 base */
    reg16	pad28[6];
    reg16	tm4_bc;			/* timer 4-6 binary count */
    reg16	tm5_bc;
    reg16	tm6_bc;
    reg16	pad29[7];
    reg8	tm6_mda;
    reg8	tm6_mdb;
    reg16	pad30[7];
    reg16	tm6_ca;
    reg16	pad31[7];
    reg16	tm6_cb;
    reg8	pad32[0x2f2a];
    reg8	wdbc;
    reg8	pad33;
    reg8	wdctr;
    reg8	pad34;
    reg8	wdrst;
    reg8	pad35[0x2003ffb];
    reg8	p0_out;
    reg8	p1_out;
    reg16	pad36;
    reg8	p2_out;
    reg8	p3_out;
    reg16	pad37[13];
    reg8	p0_md;
    reg8	p1_md;
    reg16	pad38;
    reg8	p2_md;
    reg8	p3_md;
    reg16	pad39[15];
    reg8	p2_ss;
    reg8	pad40[3];
    reg8	p4_ss;
    reg8	pad41[23];
    reg8	p0_dir;
    reg8	p1_dir;
    reg8	pad42[2];
    reg8	p2_dir;
    reg8	p3_dir;
    reg16	pad43[13];
    reg8	p0_in;
    reg8	p1_in;
    reg8	pad44[2];
    reg8	p2_in;
    reg8	p3_in;
};


#define am30_regs ((struct am30_map *)0x20000000)

#define am30_sio1 ((struct _sio *)0x34000810)
#define am30_sio2 ((struct _sio2 *)0x34000820)

#define am30_tm1md (*(reg8 *)TM1MD)
#define am30_tm2md (*(reg8 *)TM2MD)
#define am30_tm3md (*(reg8 *)TM3MD)

#define am30_tm1br (*(reg8 *)TM1BR)
#define am30_tm2br (*(reg8 *)TM2BR)
#define am30_tm3br (*(reg8 *)TM3BR)

#define am30_ivar  ((struct _ivar *)IVAR0)
#define am30_icr   ((struct _icr *)ICRBASE)
#define am30_iagr  (*(reg16 *)IAGR)
#define am30_isr  (*(reg8 *)ISR)
#define am30_p3md  (*(reg8 *)P3MD)

/*
 *  Stack frame set up in response to interrupts and
 *  exceptions.
 */
typedef struct {
    unsigned long _dummy;  /* for padding purposes */
    unsigned long _lar;
    unsigned long _lir;
    unsigned long _mdr;
    unsigned long _a1;
    unsigned long _a0;
    unsigned long _d1;
    unsigned long _d0;
    unsigned long _a3;
    unsigned long _a2;
    unsigned long _d3;
    unsigned long _d2;
    unsigned long _sp;
    unsigned long _psw;
    unsigned long _pc;
} ex_regs_t;

/*
 * Register name that is used in help strings and such
 */
# define REGNAME_EXAMPLE "d0"

/*
 *  Register numbers. These are assumed to match the
 *  register numbers used by GDB.
 */
enum __regnames {
    REG_D0,     REG_D1,     REG_D2,     REG_D3,
    REG_A0,     REG_A1,     REG_A2,     REG_A3,
    REG_SP,     REG_PC,     REG_MDR,    REG_PSW,
    REG_LIR,    REG_LAR
};
#endif

#endif /* __CPU_H__ */

