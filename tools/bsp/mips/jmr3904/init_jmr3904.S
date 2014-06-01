/*
 * init_jmr3904.S -- Initialization for JMR R3904 PCI board.
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
	.file 1 "init_jmr3904.S"
	
#include <bsp/cpu.h>
	
	.text
        .align     2
	.set noreorder
	.set noat

	.macro port val
	li	k0,\val
	sb	k0,-2812(zero)
	 nop
	.endm

/* External Bus IF registers */
#define SCSAddr		0xffffe010
#define SCSMask		0xffffe014
#define SCSWait		0xffffe018

/* SCSAddr values */
#define CS0_BASE(n)  (((n)>>24)&0xff)
#define CS1_BASE(n)  ((((n)>>24)&0xff) << 8)
#define CS2_BASE(n)  ((((n)>>24)&0xff) << 16)
#define CS3_BASE(n)  ((((n)>>24)&0xff) << 24)

/* SCSMask values */
#define CS0_MASK(n)  (n)
#define CS1_MASK(n)  ((n) << 8)
#define CS2_MASK(n)  ((n) << 16)
#define CS3_MASK(n)  ((n) << 32)

#define ISA_BASE 0x10000000
#define ISA_MASK 0xfc
#define PCI_BASE 0x14000000
#define PCI_MASK 0xfc


/* ROM controller registers */	
#define RCCR0		0xffff9000
#define RBMR0		0xffff9004
#define RCCR1		0xffff9100
#define RBMR1		0xffff9104

#define RBA(n)	(((n)>>20)<<20)
#define RPS_4W	(0 << 13)
#define RPS_8W  (1 << 13)
#define RPWT(n) (((n)&3) << 11)
#define RWT(n)	(((n)&7) << 8)
#define RCS_1M  (0 << 4)	
#define RCS_2M  (1 << 4)	
#define RCS_4M  (2 << 4)	
#define RCS_8M  (3 << 4)	
#define RCS_16M (4 << 4)	
#define RCS_32M (5 << 4)
#define RBUS_16 (1 << 2)
#define RBUS_32 (0 << 2)
#define RPAGE   1

/* DRAM Controller registers */
#define DCCR0		0xffff8000
#define DBMR0		0xffff8004
#define DWR0		0xffff8008
#define DCCR1		0xffff8100
#define DBMR1		0xffff8104
#define DWR1		0xffff8108
#define DREFC		0xffff8800

/* DCCR values */
#define DBA(n)	(((n)>>20)<<20)
#define DRA_512	(0 << 16)
#define DRA_1K	(1 << 16)
#define DRA_2K	(2 << 16)
#define DRA_4K	(3 << 16)
#define DCW_128	(0 << 12)
#define DCW_256	(1 << 12)
#define DCW_512	(2 << 12)
#define DCW_1K	(3 << 12)
#define DCW_2K	(4 << 12)
#define DRP_3   (0 << 11)
#define DRP_2   (1 << 11)
#define DCS_1M  (0 << 3)
#define DCS_2M  (1 << 3)
#define DCS_4M  (2 << 3)
#define DCS_8M  (3 << 3)
#define DCS_16M (4 << 3)
#define DCS_32M (5 << 3)
#define DBUS_16 (1 << 2)
#define DBUS_32 (0 << 2)
#define DPM_EDO (1 << 0)

/* DWR values */
#define PWTE(n) (((n)&7) << 20)
#define WTE(n)  (((n)&7) << 16)
#define PWTD(n) (((n)&7) << 12)
#define WTD(n)  (((n)&7) <<  8)
#define PWTC(n) (((n)&3) <<  4)
#define WTC(n)  (((n)&3) <<  0)




FUNC_START _bsp_init_stack
	/*
	 * Initialize hardware
	 * these mappings need to be setup before we
	 * can use the stack and make calls to other
	 * functions.
	 */
	li	a0,SCSAddr
	li	a1,CS3_BASE(0x20000000)|CS2_BASE(0x20000000)|\
		   CS1_BASE(PCI_BASE)|CS0_BASE(ISA_BASE)
	sw	a1,0(a0)

	li	a0,SCSMask
	li	a1,CS3_MASK(0x00)|CS2_MASK(0x00)|\
		   CS1_MASK(PCI_MASK)|CS0_MASK(ISA_MASK)
	sw	a1,0(a0)

	/*
	 * Use RTC to calculate sysclk
	 */
	move	s0,ra	/* save return address */
	jal	__get_sysclock
	nop	
	move	ra,s0	/* restore return address */

	bnez	v0,1f
	 nop
	/* 50MHz */
	li	a0,RCCR0
	li	a1,RBA(0x1fc00000)|RPS_4W|RWT(4)|RCS_1M|RBUS_32	
	sw	a1,0(a0)
	li	a0,DWR0
	li	a1,PWTE(1)|WTE(1)|PWTD(1)|WTD(1)|PWTC(1)|WTC(1)
	sw	a1,0(a0)
	li	a0,DREFC
	li	a1,0x00000180
	sw	a1,0(a0)
	b	2f	
    1:
	/* 66MHz */
	li	a0,RCCR0
	li	a1,RBA(0x1fc00000)|RPS_4W|RWT(5)|RCS_1M|RBUS_32	
	sw	a1,0(a0)
	li	a0,DWR0
	li	a1,PWTE(3)|WTE(3)|PWTD(2)|WTD(2)|PWTC(2)|WTC(2)
	sw	a1,0(a0)
	li	a0,DREFC
	li	a1,0x00000200
	sw	a1,0(a0)
    2:

	/* SRAM config */
	li	a0,RCCR1
	li	a1,RBA(0x00000000)|RPS_4W|RWT(0)|RCS_1M|RBUS_32
	sw	a1,0(a0)

	/* DRAM Configuration */
	li	a0,DCCR0
	li	a1,DBA(0x08000000)|DRA_2K|DCW_2K|DRP_3|DCS_16M|DBUS_32
	sw	a1,0(a0)
	
	li	a0,DBMR0
	sw	zero,0(a0)
	
	/*
	 * Interrupt Assignment
	 * 0xffffe000 = 0x00001040		# CConR
	 * in using int5, this should be 0x00001440
	 */
	la	a0, 0xffffe000
	la	a1, 0x00001040
	sw	a1, 0(a0)

	/* ISA Controller */
	# ISA Clock Frequency
	la	a0, 0xb2100000
	la	a1, 0x04		# 4 dividing
	sb	a1, 0(a0)
	
	/* Test SRAM */
	lui	t1,0xa000	/* RAM start */
	lui	t3,0xa010	/* RAM end   */
	lui	t2,0xa5a5
	ori	t2,t2,0xaa55	/* test pattern */
    1:	
	sw	zero,0(t1)
	sw	t2,4(t1)
	lw	a0,0(t1)
	nop
	bnez	a0,ram_test_done
	 nop
	sw	t2,0(t1)
	sw	zero,4(t1)
	lw	a0,0(t1)
	nop
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

	sll	s0,t1,3
	srl	s0,s0,3		/* physical address of top of memory */
	
	addiu	t1,t1,-4
	li	sp,-32
	and	sp,sp,t1

	sll	sp,sp,3
	srl	sp,sp,3
	lui	t1,0x8000
	or	sp,sp,t1

	/* enable ICache and Dcache */
	mfc0	k0,C0_CONFIG
	nop
	ori	k0,0x30
	mtc0	k0,C0_CONFIG

	sw	s0,4(sp)	/* sram size */
	sw	v0,8(sp)	/* sysclock flag */
	addiu	t0,sp,4
	
	j	ra
	 sw	t0,0(sp)	/* store pointer to be copied to
				   _bsp_ram_info_ptr, later */
FUNC_END _bsp_init_stack


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
	nop
	nop
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
	nop
	nop
	nop
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
	 * uncached.
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



#define MIN_LOOPS_PER_SEC 0x80000

#define RTC_ADDR 0xb2700000
#define RTC_DATA 0xb3c00000

#define SECS       0
#define SECS_ALARM 1
#define MINS       2
#define MINS_ALARM 3
#define HRS        4
#define HRS_ALARM  5
#define DAYOFWEEK  6
#define DAYOFMONTH 7
#define MONTH      8
#define YEAR       9
#define REG_A      10
#define REG_B      11
#define REG_C      12
#define REG_D      13
	
	.macro wbflush
	nop
	sync
	nop
    1:
	bc0f	1b
	nop
	.endm

	
/*
 * Use RTC to deduce system clock frequency.
 * Returns:
 *    0 if 50MHz	
 *    1 if 66MHz
 */
FUNC_START __get_sysclock
	li	a0,RTC_ADDR
	li	a1,RTC_DATA
	/* check to make sure we're not running on sim */
	li	t0,MONTH
	sb	t0,0(a0)
	nop
	lbu	t0,0(a1)
	nop
	bnez	t0,9f	/* sim always reads as zero */
	 li	v0,0
	jr	ra
	 nop
    9:
	sb	zero,0(a0)
	nop
	lbu	t0,0(a1)
	li	t4,MIN_LOOPS_PER_SEC
    1:
	lbu	t1,0(a1)
	bne	t0,t1,1f
	 addi	t4,t4,-1
	bgtz	t4,1b
	 nop
    1:
	bgtz	t4,9f
	 nop
	/* clock not running */
	li	t0,REG_A
	sb	t0,0(a0)
	li	t0,0x28
	sb	t0,0(a1)
	li	t0,REG_B
	sb	t0,0(a0)
	li	t0,4
	sb	t0,0(a1)
	b	9b
	 nop
    9:
	/* clock is running */
	li	t4,0
    1:
	lbu	t0,0(a1)
	beq	t0,t1,1b
	 addi	t4,t4,1

	/* t4 contains number of loops for 1 second */
	/* For 50MHz ~= 307199 */
	/* For 66MHz ~= 405502 */
	li	t0,310000
	slt	v0,t0,t4

	jr	ra	
	 nop	
FUNC_END   __get_sysclock
	




