/*
 * cpu.h -- FR-V CPU specific header for Cygnus BSP.
 *
 * Copyright (c) 1999, 2000 Cygnus Support
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

#ifdef __FRV_UNDERSCORE__
#define __NEED_UNDERSCORE__ 1
#endif

/*
 * Macros to glue together two tokens.
 */
#ifdef __STDC__
#    define XGLUE(a,b) a##b
#else
#    define XGLUE(a,b) a/**/b
#endif

#define GLUE(a,b) XGLUE(a,b)

/*
 * Symbol Names with leading underscore if necessary
 */
#ifdef __NEED_UNDERSCORE__
# define SYM_NAME(name) GLUE(_,name)
#else
# define SYM_NAME(name) name
#endif /* __NEED_UNDERSCORE__ */

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
#else
.macro FUNC_START name
	.globl \name
    \name:
.endm
#endif

/*
 * Assembly function end definition
 */
#ifdef __NEED_UNDERSCORE__
.macro FUNC_END name
.Lend_\name:
    .size  \name,(.Lend_\name - _\name)
.endm
#else
.macro FUNC_END name
.Lend_\name:
    .size  \name,(.Lend_\name - \name)
.endm
#endif

/* convenience macro to load 32-bit immediate value */
.macro seti val,reg
	sethi	%hi(\val),\reg
	setlo	%lo(\val),\reg
.endm

#endif /* defined(__ASSEMBLER__) */


/*
 * PSR bit definitions
 */
#define PSR_IMPL_MASK  (15 << 28)
#define PSR_VERS_MASK  (15 << 24)
#define PSR_ICE        (1 << 16)
#define PSR_SE         (1 << 15)
#define PSR_NEM        (1 << 14)
#define PSR_CM         (1 << 13)
#define PSR_BE         (1 << 12)
#define PSR_ESR        (1 << 11)
#define PSR_ECS        (1 << 10)
#define PSR_EC         (1 <<  9)
#define PSR_EF         (1 <<  8)
#define PSR_EM         (1 <<  7)
#define PSR_PIL_MASK   (15 << 3)
#define PSR_PIL_SHIFT  3
#define PSR_S          (1 <<  2)
#define PSR_PS         (1 <<  1)
#define PSR_ET         (1 <<  0)

#define PSR_INIT  ((15<<PSR_PIL_SHIFT) | PSR_S | PSR_BE | PSR_ESR)

/*
 * HSR0 bit definitions.
 */
#define HSR_ICE		(1 << 31)
#define HSR_DCE		(1 << 30)
#define HSR_ICFI	(1 << 29)
#define HSR_DCFI	(1 << 28)
#define HSR_CBM		(1 << 27)
#define HSR_EIMMU	(1 << 26)
#define HSR_EDMMU	(1 << 25)
#define HSR_UMMU	(1 << 24)
#define HSR_EMEM	(1 << 23)
#define HSR_RME		(1 << 22)
#define HSR_MQV		(1 << 21)
#define HSR_SA		(1 << 12)
#define HSR_FRN		(1 << 11)
#define HSR_GRN		(1 << 10)
#define HSR_FRHE	(1 << 9)
#define HSR_FRLE	(1 << 8)
#define HSR_GRHE	(1 << 7)
#define HSR_GRLE	(1 << 6)
#define HSR_PDM		(1 << 0)


/* Stack Frame Offsets */
#define FR_GR0	  0
#define FR_FR0	  (64*4)
#define FR_SP	  (1*4)

#define FR_PC	  (128*4)
#define FR_PSR	  (129*4)
#define FR_CCR	  (130*4)
#define FR_CCCR	  (131*4)
#define FR_BPCSR  (132*4)
#define FR_BPSR   (133*4)
#define FR_PCSR   (134*4)
#define FR_TBR    (135*4)

#define FR_BRR    (136*4)
#define FR_DBAR0  (137*4)
#define FR_DBAR1  (138*4)
#define FR_DBAR2  (139*4)
#define FR_DBAR3  (140*4)

#define FR_LR	  (145*4)
#define FR_LCR	  (146*4)
#define FR_VSRNUM (147*4)
#define FR_TRAP   (148*4)

#define EX_STACK_SIZE (150*4)

#if defined(__ASSEMBLER__)

	.macro save_all_gr
	sti	gr0,@(sp,FR_GR0)
	stdi	gr2,@(sp,FR_GR0+(2*4))
	stdi	gr4,@(sp,FR_GR0+(4*4))
	stdi	gr6,@(sp,FR_GR0+(6*4))
	stdi	gr8,@(sp,FR_GR0+(8*4))
	stdi	gr10,@(sp,FR_GR0+(10*4))
	stdi	gr12,@(sp,FR_GR0+(12*4))
	stdi	gr14,@(sp,FR_GR0+(14*4))
	stdi	gr16,@(sp,FR_GR0+(16*4))
	stdi	gr18,@(sp,FR_GR0+(18*4))
	stdi	gr20,@(sp,FR_GR0+(20*4))
	stdi	gr22,@(sp,FR_GR0+(22*4))
	stdi	gr24,@(sp,FR_GR0+(24*4))
	stdi	gr26,@(sp,FR_GR0+(26*4))
	stdi	gr28,@(sp,FR_GR0+(28*4))
	stdi	gr30,@(sp,FR_GR0+(30*4))
#if __FRV_GPR__ != 32
	stdi	gr32,@(sp,FR_GR0+(32*4))
	stdi	gr34,@(sp,FR_GR0+(34*4))
	stdi	gr36,@(sp,FR_GR0+(36*4))
	stdi	gr38,@(sp,FR_GR0+(38*4))
	stdi	gr40,@(sp,FR_GR0+(40*4))
	stdi	gr42,@(sp,FR_GR0+(42*4))
	stdi	gr44,@(sp,FR_GR0+(44*4))
	stdi	gr46,@(sp,FR_GR0+(46*4))
	stdi	gr48,@(sp,FR_GR0+(48*4))
	stdi	gr50,@(sp,FR_GR0+(50*4))
	stdi	gr52,@(sp,FR_GR0+(52*4))
	stdi	gr54,@(sp,FR_GR0+(54*4))
	stdi	gr56,@(sp,FR_GR0+(56*4))
	stdi	gr58,@(sp,FR_GR0+(58*4))
	stdi	gr60,@(sp,FR_GR0+(60*4))
	stdi	gr62,@(sp,FR_GR0+(62*4))
#endif  /*  __FRV_GPR__ != 32 */
	.endm

	.macro save_min_gr
	stdi	gr2,@(sp,FR_GR0+(2*4))
	stdi	gr4,@(sp,FR_GR0+(4*4))
	stdi	gr6,@(sp,FR_GR0+(6*4))
	stdi	gr8,@(sp,FR_GR0+(8*4))
	stdi	gr10,@(sp,FR_GR0+(10*4))
	stdi	gr12,@(sp,FR_GR0+(12*4))
	stdi	gr14,@(sp,FR_GR0+(14*4))
#if __FRV_GPR__ != 32
	stdi	gr32,@(sp,FR_GR0+(32*4))
	stdi	gr34,@(sp,FR_GR0+(34*4))
	stdi	gr36,@(sp,FR_GR0+(36*4))
	stdi	gr38,@(sp,FR_GR0+(38*4))
	stdi	gr40,@(sp,FR_GR0+(40*4))
	stdi	gr42,@(sp,FR_GR0+(42*4))
	stdi	gr44,@(sp,FR_GR0+(44*4))
	stdi	gr46,@(sp,FR_GR0+(46*4))
#endif  /*  __FRV_GPR__ != 32 */
	.endm

	.macro save_all_fr
#if __FRV_FPR__ != 0
	stdfi	fr0,@(sp,FR_FR0+(0*4))
	stdfi	fr2,@(sp,FR_FR0+(2*4))
	stdfi	fr4,@(sp,FR_FR0+(4*4))
	stdfi	fr6,@(sp,FR_FR0+(6*4))
	stdfi	fr8,@(sp,FR_FR0+(8*4))
	stdfi	fr10,@(sp,FR_FR0+(10*4))
	stdfi	fr12,@(sp,FR_FR0+(12*4))
	stdfi	fr14,@(sp,FR_FR0+(14*4))
	stdfi	fr16,@(sp,FR_FR0+(16*4))
	stdfi	fr18,@(sp,FR_FR0+(18*4))
	stdfi	fr20,@(sp,FR_FR0+(20*4))
	stdfi	fr22,@(sp,FR_FR0+(22*4))
	stdfi	fr24,@(sp,FR_FR0+(24*4))
	stdfi	fr26,@(sp,FR_FR0+(26*4))
	stdfi	fr28,@(sp,FR_FR0+(28*4))
	stdfi	fr30,@(sp,FR_FR0+(30*4))
#if __FRV_FPR__ != 32
	stdfi	fr32,@(sp,FR_FR0+(32*4))
	stdfi	fr34,@(sp,FR_FR0+(34*4))
	stdfi	fr36,@(sp,FR_FR0+(36*4))
	stdfi	fr38,@(sp,FR_FR0+(38*4))
	stdfi	fr40,@(sp,FR_FR0+(40*4))
	stdfi	fr42,@(sp,FR_FR0+(42*4))
	stdfi	fr44,@(sp,FR_FR0+(44*4))
	stdfi	fr46,@(sp,FR_FR0+(46*4))
	stdfi	fr48,@(sp,FR_FR0+(48*4))
	stdfi	fr50,@(sp,FR_FR0+(50*4))
	stdfi	fr52,@(sp,FR_FR0+(52*4))
	stdfi	fr54,@(sp,FR_FR0+(54*4))
	stdfi	fr56,@(sp,FR_FR0+(56*4))
	stdfi	fr58,@(sp,FR_FR0+(58*4))
	stdfi	fr60,@(sp,FR_FR0+(60*4))
	stdfi	fr62,@(sp,FR_FR0+(62*4))
#endif /* __FRV_FPR__ != 32 */
#endif /* __FRV_FPR__ != 0 */
	.endm

	.macro save_min_fr
#if __FRV_FPR__ != 0
	stdfi	fr0,@(sp,FR_FR0+(0*4))
	stdfi	fr2,@(sp,FR_FR0+(2*4))
	stdfi	fr4,@(sp,FR_FR0+(4*4))
	stdfi	fr6,@(sp,FR_FR0+(6*4))
	stdfi	fr8,@(sp,FR_FR0+(8*4))
	stdfi	fr10,@(sp,FR_FR0+(10*4))
	stdfi	fr12,@(sp,FR_FR0+(12*4))
	stdfi	fr14,@(sp,FR_FR0+(14*4))
#if __FRV_FPR__ != 32
	stdfi	fr32,@(sp,FR_FR0+(32*4))
	stdfi	fr34,@(sp,FR_FR0+(34*4))
	stdfi	fr36,@(sp,FR_FR0+(36*4))
	stdfi	fr38,@(sp,FR_FR0+(38*4))
	stdfi	fr40,@(sp,FR_FR0+(40*4))
	stdfi	fr42,@(sp,FR_FR0+(42*4))
	stdfi	fr44,@(sp,FR_FR0+(44*4))
	stdfi	fr46,@(sp,FR_FR0+(46*4))
#endif /* __FRV_FPR__ != 32 */
#endif /* __FRV_FPR__ != 0 */
        .endm

	.macro save_sprs
	/* save some SPR regs */
	movsg	pcsr,gr4
	sti	gr4,@(sp,FR_PCSR)
	movsg	lr,gr4
	sti	gr4,@(sp,FR_LR)
	movsg	lcr,gr4
	sti	gr4,@(sp,FR_LCR)
	movsg	cccr,gr4
	sti	gr4,@(sp,FR_CCCR)
	movsg	bpcsr,gr4
	sti	gr4,@(sp,FR_BPCSR)
	movsg	bpsr,gr4
	sti	gr4,@(sp,FR_BPSR)
	movsg	tbr,gr4
	sti	gr4,@(sp,FR_TBR)
	.endm

	.macro restore_all_gr
	/* restore gr regs */
#if __FRV_GPR__ == 64
	lddi	@(sp,FR_GR0+(32*4)),gr32
	lddi	@(sp,FR_GR0+(34*4)),gr34
	lddi	@(sp,FR_GR0+(36*4)),gr36
	lddi	@(sp,FR_GR0+(38*4)),gr38
	lddi	@(sp,FR_GR0+(40*4)),gr40
	lddi	@(sp,FR_GR0+(42*4)),gr42
	lddi	@(sp,FR_GR0+(44*4)),gr44
	lddi	@(sp,FR_GR0+(46*4)),gr46
	lddi	@(sp,FR_GR0+(48*4)),gr48
	lddi	@(sp,FR_GR0+(50*4)),gr50
	lddi	@(sp,FR_GR0+(52*4)),gr52
	lddi	@(sp,FR_GR0+(54*4)),gr54
	lddi	@(sp,FR_GR0+(56*4)),gr56
	lddi	@(sp,FR_GR0+(58*4)),gr58
	lddi	@(sp,FR_GR0+(60*4)),gr60
	lddi	@(sp,FR_GR0+(62*4)),gr62
#endif  /*  __FRV_GPR__ == 64 */
	lddi	@(sp,FR_GR0+(2*4)),gr2
	lddi	@(sp,FR_GR0+(4*4)),gr4
	lddi	@(sp,FR_GR0+(6*4)),gr6
	lddi	@(sp,FR_GR0+(8*4)),gr8
	lddi	@(sp,FR_GR0+(10*4)),gr10
	lddi	@(sp,FR_GR0+(12*4)),gr12
	lddi	@(sp,FR_GR0+(14*4)),gr14
	lddi	@(sp,FR_GR0+(16*4)),gr16
	lddi	@(sp,FR_GR0+(18*4)),gr18
	lddi	@(sp,FR_GR0+(20*4)),gr20
	lddi	@(sp,FR_GR0+(22*4)),gr22
	lddi	@(sp,FR_GR0+(24*4)),gr24
	lddi	@(sp,FR_GR0+(26*4)),gr26
	lddi	@(sp,FR_GR0+(28*4)),gr28
	/* lddi	@(sp,FR_GR0+(30*4)),gr30 */
	/* ldi	@(sp,FR_GR0+(1*4)),gr1	*/	/* stack pointer */
	.endm
	
	.macro restore_min_gr
	/* restore gr regs */
#if __FRV_GPR__ == 64
	lddi	@(sp,FR_GR0+(32*4)),gr32
	lddi	@(sp,FR_GR0+(34*4)),gr34
	lddi	@(sp,FR_GR0+(36*4)),gr36
	lddi	@(sp,FR_GR0+(38*4)),gr38
	lddi	@(sp,FR_GR0+(40*4)),gr40
	lddi	@(sp,FR_GR0+(42*4)),gr42
	lddi	@(sp,FR_GR0+(44*4)),gr44
	lddi	@(sp,FR_GR0+(46*4)),gr46
#endif  /*  __FRV_GPR__ == 64 */
	lddi	@(sp,FR_GR0+(2*4)),gr2
	lddi	@(sp,FR_GR0+(4*4)),gr4
	lddi	@(sp,FR_GR0+(6*4)),gr6
	lddi	@(sp,FR_GR0+(8*4)),gr8
	lddi	@(sp,FR_GR0+(10*4)),gr10
	lddi	@(sp,FR_GR0+(12*4)),gr12
	lddi	@(sp,FR_GR0+(14*4)),gr14
	/* ldi	@(sp,FR_GR0+(1*4)),gr1	*/	/* stack pointer */
	.endm
	
	.macro restore_all_fr
#if __FRV_FPR__ != 0
	lddfi	@(sp,FR_FR0+(0*4)),fr0
	lddfi	@(sp,FR_FR0+(2*4)),fr2
	lddfi	@(sp,FR_FR0+(4*4)),fr4
	lddfi	@(sp,FR_FR0+(6*4)),fr6
	lddfi	@(sp,FR_FR0+(8*4)),fr8
	lddfi	@(sp,FR_FR0+(10*4)),fr10
	lddfi	@(sp,FR_FR0+(12*4)),fr12
	lddfi	@(sp,FR_FR0+(14*4)),fr14
	lddfi	@(sp,FR_FR0+(16*4)),fr16
	lddfi	@(sp,FR_FR0+(18*4)),fr18
	lddfi	@(sp,FR_FR0+(20*4)),fr20
	lddfi	@(sp,FR_FR0+(22*4)),fr22
	lddfi	@(sp,FR_FR0+(24*4)),fr24
	lddfi	@(sp,FR_FR0+(26*4)),fr26
	lddfi	@(sp,FR_FR0+(28*4)),fr28
	lddfi	@(sp,FR_FR0+(30*4)),fr30
#if __FRV_FPR__ != 32
	lddfi	@(sp,FR_FR0+(32*4)),fr32
	lddfi	@(sp,FR_FR0+(34*4)),fr34
	lddfi	@(sp,FR_FR0+(36*4)),fr36
	lddfi	@(sp,FR_FR0+(38*4)),fr38
	lddfi	@(sp,FR_FR0+(40*4)),fr40
	lddfi	@(sp,FR_FR0+(42*4)),fr42
	lddfi	@(sp,FR_FR0+(44*4)),fr44
	lddfi	@(sp,FR_FR0+(46*4)),fr46
	lddfi	@(sp,FR_FR0+(48*4)),fr48
	lddfi	@(sp,FR_FR0+(50*4)),fr50
	lddfi	@(sp,FR_FR0+(52*4)),fr52
	lddfi	@(sp,FR_FR0+(54*4)),fr54
	lddfi	@(sp,FR_FR0+(56*4)),fr56
	lddfi	@(sp,FR_FR0+(58*4)),fr58
	lddfi	@(sp,FR_FR0+(60*4)),fr60
	lddfi	@(sp,FR_FR0+(62*4)),fr62
#endif /* __FRV_FPR__ != 32 */
#endif /* __FRV_FPR__ != 0 */
	.endm
	
	.macro restore_min_fr
#if __FRV_FPR__ != 0
	lddfi	@(sp,FR_FR0+(0*4)),fr0
	lddfi	@(sp,FR_FR0+(2*4)),fr2
	lddfi	@(sp,FR_FR0+(4*4)),fr4
	lddfi	@(sp,FR_FR0+(6*4)),fr6
	lddfi	@(sp,FR_FR0+(8*4)),fr8
	lddfi	@(sp,FR_FR0+(10*4)),fr10
	lddfi	@(sp,FR_FR0+(12*4)),fr12
	lddfi	@(sp,FR_FR0+(14*4)),fr14
#if __FRV_FPR__ != 32
	lddfi	@(sp,FR_FR0+(32*4)),fr32
	lddfi	@(sp,FR_FR0+(34*4)),fr34
	lddfi	@(sp,FR_FR0+(36*4)),fr36
	lddfi	@(sp,FR_FR0+(38*4)),fr38
	lddfi	@(sp,FR_FR0+(40*4)),fr40
	lddfi	@(sp,FR_FR0+(42*4)),fr42
	lddfi	@(sp,FR_FR0+(44*4)),fr44
	lddfi	@(sp,FR_FR0+(46*4)),fr46
#endif /* __FRV_FPR__ != 32 */
#endif /* __FRV_FPR__ != 0 */
	.endm

	.macro restore_sprs
	/* restore some SPR regs */
	ldi	@(sp,FR_PCSR),gr4
	movgs	gr4,pcsr
	ldi	@(sp,FR_LR),gr4
	movgs	gr4,lr
	ldi	@(sp,FR_LCR),gr4
	movgs	gr4,lcr
	ldi	@(sp,FR_CCCR),gr4
	movgs	gr4,cccr
	ldi	@(sp,FR_BPCSR),gr4
	movgs	gr4,bpcsr
	ldi	@(sp,FR_BPSR),gr4
	movgs	gr4,bpsr
	ldi	@(sp,FR_TBR),gr4
	movgs	gr4,tbr
	.endm
#else
enum __regnames {
    REG_ZERO,   REG_SP,     REG_FP,     REG_GR3,
    REG_GR4,    REG_GR5,    REG_GR6,    REG_GR7,
    REG_GR8,    REG_GR9,    REG_GR10,   REG_GR11,
    REG_GR12,   REG_GR13,   REG_GR14,   REG_GR15,
    REG_GR16,   REG_GR17,   REG_GR18,   REG_GR19,
    REG_GR20,   REG_GR21,   REG_GR22,   REG_GR23,
    REG_GR24,   REG_GR25,   REG_GR26,   REG_GR27,
    REG_GR28,   REG_GR29,   REG_GR30,   REG_GR31,
    REG_GR32,   REG_GR33,   REG_GR34,   REG_GR35,
    REG_GR36,   REG_GR37,   REG_GR38,   REG_GR39,
    REG_GR40,   REG_GR41,   REG_GR42,   REG_GR43,
    REG_GR44,   REG_GR45,   REG_GR46,   REG_GR47,
    REG_GR48,   REG_GR49,   REG_GR50,   REG_GR51,
    REG_GR52,   REG_GR53,   REG_GR54,   REG_GR55,
    REG_GR56,   REG_GR57,   REG_GR58,   REG_GR59,
    REG_GR60,   REG_GR61,   REG_GR62,   REG_GR63,

    REG_FR0,    REG_FR1,    REG_FR2,    REG_FR3,
    REG_FR4,    REG_FR5,    REG_FR6,    REG_FR7,
    REG_FR8,    REG_FR9,    REG_FR10,   REG_FR11,
    REG_FR12,   REG_FR13,   REG_FR14,   REG_FR15,
    REG_FR16,   REG_FR17,   REG_FR18,   REG_FR19,
    REG_FR20,   REG_FR21,   REG_FR22,   REG_FR23,
    REG_FR24,   REG_FR25,   REG_FR26,   REG_FR27,
    REG_FR28,   REG_FR29,   REG_FR30,   REG_FR31,
    REG_FR32,   REG_FR33,   REG_FR34,   REG_FR35,
    REG_FR36,   REG_FR37,   REG_FR38,   REG_FR39,
    REG_FR40,   REG_FR41,   REG_FR42,   REG_FR43,
    REG_FR44,   REG_FR45,   REG_FR46,   REG_FR47,
    REG_FR48,   REG_FR49,   REG_FR50,   REG_FR51,
    REG_FR52,   REG_FR53,   REG_FR54,   REG_FR55,
    REG_FR56,   REG_FR57,   REG_FR58,   REG_FR59,
    REG_FR60,   REG_FR61,   REG_FR62,   REG_FR63,

    REG_PC,     REG_PSR,    REG_CCR,    REG_CCCR,   
    REG_BPCSR,  REG_BPSR,   REG_PCSR,   REG_TBR,
    REG_BRR,    REG_DBAR0,  REG_DBAR1,  REG_DBAR2,
    REG_DBAR3,  REG_X141,   REG_X142,   REG_X143,
    REG_X144,   REG_LR,     REG_LCR,    REG_VSRNUM,

    REG_TRAP,   REG_X149,

    REG_LAST
};

/*
 *  How registers are stored for exceptions.
 */
typedef struct
{
    unsigned int _gpr[64];
# define _zero _gpr[0]
# define _sp   _gpr[1]
# define _fp   _gpr[2]
    float        _fpr[64];
    unsigned int _pc;
    unsigned int _psr;
    unsigned int _ccr;
    unsigned int _cccr;
    unsigned int _bpcsr;
    unsigned int _bpsr;
    unsigned int _pcsr;
    unsigned int _tbr;
    unsigned int _brr;
    unsigned int _dbar0;
    unsigned int _dbar1;
    unsigned int _dbar2;
    unsigned int _dbar3;
    unsigned int _x141;
    unsigned int _x142;
    unsigned int _x143;
    unsigned int _x144;
    unsigned int _lr;
    unsigned int _lcr;
    unsigned int _vsrnum;
    unsigned int _trap;
    unsigned int _x149;
} ex_regs_t;

typedef ex_regs_t gdb_regs_t;

#if BP_USES_TRAP
#define BREAKPOINT() asm volatile("tira	gr0,#1\n")
#else
#define BREAKPOINT() asm volatile("break\n")
#endif

#define SYSCALL      tira	gr0,0

#define BSP_ENABLE_INTERRUPTS()  \
  asm volatile(                  \
	"movsg	psr,gr8\n"       \
	"setlos	(15<<3),gr9\n"   \
	"not	gr9,gr9\n"       \
	"and	gr9,gr8,gr8\n"   \
	"movgs	gr8,psr\n")

static inline unsigned __get_psr(void)
{
    unsigned retval;

    asm volatile (
        "movsg   psr,%0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

static inline unsigned __get_hsr0(void)
{
    unsigned retval;

    asm volatile (
        "movsg   hsr0,%0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

static inline unsigned __get_dcr(void)
{
    unsigned retval;

    asm volatile (
        "movsg   dcr,%0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

static inline void __set_dcr(unsigned val)
{
    asm volatile (
        "movgs   %0,dcr\n"
	: /* no outputs */
	: "r" (val) );
}


static inline unsigned __get_brr(void)
{
    unsigned retval;

    asm volatile (
        "movsg   brr,%0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

static inline void __set_brr(unsigned val)
{
    asm volatile (
        "movgs   %0,brr\n"
	: /* no outputs */
	: "r" (val) );
}


/* Define MOVE_64 to enable 64-bit data loads/stores in cygmon */
static inline void __move64(void *src, void *dest)
{
    asm volatile (
	"ldd    @(%1,gr0),gr10\n"
	"std    gr10,@(%0,gr0)\n"
	: /* no outputs */
	: "r" (dest), "r"  (src)
	: "gr10", "gr11");
}

#define MOVE_64(s,d) __move64(s,d)

#endif /* !__ASSEMBLER__ */

/*
 *  breakpoint opcode.
 */
#if BP_USES_TRAP
#define BREAKPOINT_OPCODE	0xc0700001  /* tira gr0,#1 */
#else
#define BREAKPOINT_OPCODE	0x801000C0  /* break */
#endif

/*
 * Core Exception vectors.
 */
#define BSP_EXC_RESERVED        0
#define BSP_EXC_DSTORE_ERROR    1
#define BSP_EXC_IACCESS_MMU	2
#define BSP_EXC_IACCESS_ERROR	3
#define BSP_EXC_IACCESS_EXC	4
#define BSP_EXC_IPRIVILEGE      5
#define BSP_EXC_ILLEGAL         6
#define BSP_EXC_FP_DISABLED     7
#define BSP_EXC_MP_DISABLED     8
#define BSP_EXC_REG_EXCEPTION   9
#define BSP_EXC_MEM_ALIGN      10
#define BSP_EXC_FP_EXC         11
#define BSP_EXC_MP_EXC         12
#define BSP_EXC_DACCESS_ERROR  13
#define BSP_EXC_DACCESS_MMU    14
#define BSP_EXC_DACCESS_EXC    15
#define BSP_EXC_DIVIDE         16
#define BSP_EXC_COMMIT         17
#define BSP_EXC_COMPOUND       18
#define BSP_EXC_SYSCALL        19
#define BSP_EXC_TRAP           20
#define BSP_EXC_INT1           21
#define BSP_EXC_INT2           22
#define BSP_EXC_INT3           23
#define BSP_EXC_INT4           24
#define BSP_EXC_INT5           25
#define BSP_EXC_INT6           26
#define BSP_EXC_INT7           27
#define BSP_EXC_INT8           28
#define BSP_EXC_INT9           29
#define BSP_EXC_INT10          30
#define BSP_EXC_INT11          31
#define BSP_EXC_INT12          32
#define BSP_EXC_INT13          33
#define BSP_EXC_INT14          34
#define BSP_EXC_INT15          35
#define BSP_EXC_BREAK          36

#define BSP_MAX_EXCEPTIONS     37

#define BSP_VEC_MT_DEBUG       38
#define BSP_VEC_STUB_ENTRY     39
#define BSP_VEC_BSPDATA	       40
#define BSP_VEC_PAD	       41

#define NUM_VTAB_ENTRIES       42


/* Hardware Trap Types */
#define TT_IACCESS_MMU		0x01
#define TT_IACCESS_ERROR	0x02
#define TT_IACCESS_EXCEPTION	0x03
#define TT_PRIVILEGE		0x06
#define TT_ILLEGAL		0x07
#define TT_REG_EXCEPTION	0x08
#define TT_FP_DISABLED		0x0a
#define TT_MP_DISABLED		0x0b
#define TT_FP_EXCEPTION		0x0d
#define TT_MP_EXCEPTION		0x0e
#define TT_MEM_ALIGN		0x10
#define TT_DACCESS_ERROR	0x11
#define TT_DACCESS_MMU		0x12
#define TT_DACCESS_EXCEPTION	0x13
#define TT_DSTORE_ERROR		0x14
#define TT_DIVISION		0x17
#define TT_COMMIT		0x19
#define TT_COMPOUND		0x20
#define TT_INT1			0x21
#define TT_INT15		0x2f
#define TT_SYSCALL		0x80
#define TT_BREAK		0xff

#endif /* __CPU_H__ */
