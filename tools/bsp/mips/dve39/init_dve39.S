/*
 * init_dve39.S -- Initialization for Densan R3900 VME board.
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
	.file 1 "init_dve39.S"
	
#include __BOARD_HEADER__
#include <bsp/cpu.h>
	
#define RED \
	lui	k0,0xbfbf ; \
	li	k1,0x40   ; \
	sb	k1,3(k0)  ; \
	 nop

#define YELLOW \
	lui	k0,0xbfbf ; \
	li	k1,0x80	  ; \
	sb	k1,3(k0)  ; \
	 nop

	.text
        .align     2
	.set noreorder
	.set noat

FUNC_START _bsp_init_stack
	/*
	 *  Initialize the DGA-001 Controller.
	 */
	li	t0,CSR0
	li	t1,CSR0_LB3 | CSR0_GPIOE7 | CSR0_GPIOE6
	sw	t1,0(t0)
	nop
	nop
	nop
	
	li	t1,CSR0_GPIO7 | CSR0_GPIO6
	sb	t1,3(t0)

	li	t0,CSR1
	lw	t1,0(t0)
	nop
	sw	t1,0(t0)

#if 1
	li	t0,CSR3
	li	t1,0x100 | CSR3_IFRSUP | CSR3_IFRUSR
	sw	t1,0(t0)

	li	t0,CSR19
	li	t1,0x000810ff
	sw	t1,0(t0)

	li	t0,CSR24
	li	t1,CSR24_ACFIL7 | CSR24_ABTIL7 | CSR24_SFIL6 | CSR24_BERIL6 | \
	           CSR24_IAKIL5 | CSR24_SRQIL5 | CSR24_SAKIL5
	sw	t1,0(t0)

	li	t0,CSR25
	li	t1,CSR25_DMAIL5 | CSR25_TT1IL5 | CSR25_TT0IL5 | CSR25_GP3IL7 | \
	           CSR25_GP2IL4 | CSR25_GP1IL3 | CSR25_GP0IL2
	sw	t1,0(t0)

	li	t0,CSR26
	li	t1,CSR26_SW7IL7 | CSR26_SW6IL6 | CSR26_SW5IL5 | CSR26_SW4IL4 | \
		   CSR26_SW3IL3 | CSR26_SW2IL2 | CSR26_SW1IL1 | CSR26_SW0IL1
	sw	t1,0(t0)

	li	t0,CSR27
	li	t1,CSR27_VM7IL7 | CSR27_VM6IL6 | CSR27_VM5IL5 | CSR27_VM4IL4 | \
		   CSR27_VM3IL3 | CSR27_VM2IL2 | CSR27_VM1IL1
	sw	t1,0(t0)

	li	t0,IFR0
	li	t1,IFR0_SRQF
	sw	t1,0(t0)
	
	li	t0,IFR3
	sw	zero,0(t0)
#endif

	/*
	 *  Clear Icache
	 */
	lui	a0,0xbfbe
	addiu	a1,a0,0x1000 - 4
    1:
	sw	zero,0(a0)
	bne	a0,a1,1b
	 addiu   a0,a0,4
	
	/*
	 *  Clear Dcache
	 */
	lui	a0,0xbfbe
	ori	a0,a0,0x8000
	addiu	a1,a0,0x400 - 4
    1:
	sw	zero,0(a0)
	bne	a0,a1,1b
	 addiu   a0,a0,4
	
	RED
	
	lui	t1,0xa000	/* RAM start */
	lui	t3,0xa100	/* RAM end   */
	lui	t2,0xa5a5
	ori	t2,t2,0xaa55	/* test pattern */
    1:	
	lw	zero,0(t1)
	sw	zero,0(t1)
	lw	a0,0(t1)
	bnez	a0,ram_test_done
	 nop
	sw	t2,0(t1)
	lw	a0,0(t1)
	bne	a0,t2,ram_test_done
	 nop
	li	t0,4096
    2:
	sw	zero,0(t1)
	addiu	t0,t0,-4
	bnez	t0,2b
	addiu	t1,t1,4
	bne	t1,t3,1b
	 nop
    ram_test_done:

	YELLOW

	sll	s0,t1,3
	srl	s0,s0,3		/* physical address of top of memory */
	
	addiu	t1,t1,-4
	li	sp,-32
	and	sp,sp,t1

	sll	sp,sp,3
	srl	sp,sp,3
	lui	t1,0x8000
	or	sp,sp,t1
	sw	s0,0(sp)	/* store pointer to be copied to
				   _bsp_ram_info_ptr, later */
	li	t0,CSR0
	lbu	t4,1(t0)
	andi	t4,t4,0x87
	sb	t4,1(t0)
	nop

#if 1
	/*
	 *  Use Tick Timer 0 as basis for millisecond timer.
	 */
	li	t0,CSR13	# Tick Timer 0 Counter
	lui	t1,0x8000
	sw	t1,0(t0)
	
	li	t0,CSR12	# Tick Timer 0 Control
	li	t1,0x80ffffff
	sw	t1,0(t0)
	 nop
#endif

	j	ra
	 nop
FUNC_END _bsp_init_stack



/*
 *  Invalidate entire Icache.
 */
FUNC_START __icache_init
	mfc0	a2,C0_CONFIG
	nop
	ori	a3,a2,0x20
	xori	a3,a3,0x20
	mtc0	a3,C0_CONFIG
	b	1f
	nop
    1:
	lui	a0,0x8000
	ori	a1,a0,0x1000
	b	1f
	nop
	nop
	nop
	nop
    1:
	cache	INDEX_INVALIDATE_I,0(a0)
	nop
	addi	a0,a0,16
	bne	a0,a1,1b
	nop
	mtc0	a2,C0_CONFIG
	jr	ra
	 nop
FUNC_END __icache_init


/*
 *  Flush Icache. (Entire cache, for now).
 *
 *  void __icache_flush(void *va, int nbytes)
 */
FUNC_START __icache_flush
	mfc0	a2,C0_CONFIG
	nop
	ori	a3,a2,0x20
	xori	a3,a3,0x20
	mtc0	a3,C0_CONFIG
	b	1f
	nop
    1:
	lui	a0,0x8000
	ori	a1,a0,0x1000
	b	1f
	nop
	nop
	nop
	nop
    1:
	cache	INDEX_INVALIDATE_I,0(a0)
	nop
	addi	a0,a0,16
	bne	a0,a1,1b
	nop
	mtc0	a2,C0_CONFIG
	jr	ra
	 nop
FUNC_END __icache_flush


/*
 *  Flush entire Dcache.
 *
 *  void __dcache_flush(void *va, int nbytes)
 */
FUNC_START __dcache_flush
	beqz	a1,2f
	/*
	 * Must make sure its a cached address.
	 *
	 * For tx39, these addresses are uncached:
	 *   x111 1111 xxxx xxxx xxxx xxxx xxxx xxxx
	 *   101x xxxx xxxx xxxx xxxx xxxx xxxx xxxx
	 *
	 * Danger, Will Robinson! The following assumes
	 * the given address range is all cached or all
	 * unached.
	 */
	srl	t2,a0,24
	andi	t3,t2,0x7f
	xori	t3,t3,0x7f
	andi	t2,t2,0xe0
	xori	t2,t2,0xa0
	or	t2,t2,t3
	beqz	t2,2f		/* branch on uncached addresses */
	 andi	t1,a0,3
	li	t2,-4
	and	a0,a0,t2	/* floor va to word address */
	add	a1,a1,t1	/* adjust count for flooring of address */
	addiu	a1,a1,3		/* add for partial word */
    1:
	cache	HIT_INVALIDATE_D,0(a0)
	addiu	a1,a1,-4
	bgez	a1,1b
	 addiu	a0,a0,4
    2:
	jr	ra
	 nop
FUNC_END __dcache_flush
