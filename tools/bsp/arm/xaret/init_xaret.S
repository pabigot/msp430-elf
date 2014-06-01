/*
 * init_xaret.S -- Low-level Board Initialization.
 *
 * Copyright (c) 2000 Red Hat, Inc.
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
 *
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */
	.file "init_xaret.S"

#include <bsp/xaret.h>

#ifdef __ARMEB__
#define LOCAL_ARM_ENDIAN        ARM_BIG_ENDIAN
#else
#define LOCAL_ARM_ENDIAN        ARM_LITTLE_ENDIAN
#endif

#define MMU_ENABLE_VALUE       (ARM_MMU_ENABLED                                | \
                                ARM_ADDRESS_FAULT_DISABLED                     | \
                                ARM_DATA_CACHE_ENABLED                         | \
                                ARM_WRITE_BUFFER_ENABLED                       | \
                                LOCAL_ARM_ENDIAN                               | \
                                ARM_ACCESS_CHECKS_ROM                          | \
                                ARM_INSTRUCTION_CACHE_ENABLED                  | \
                                ARM_VIRTUAL_IVR_BASE_00000000)


/* Companion chip MCU registers */
#define    SDIR_ADDR        0x00001500
#define    SDCR_ADDR        0x00001504
#define    SDBR_ADDR        0x00001508
#define    SBR0_ADDR        0x0000150C
#define    SBR1_ADDR        0x00001510
#define    ECCR_ADDR        0x00001534
#define    FEBR1_ADDR       0x00001550
#define    FBSR1_ADDR       0x00001558
#define    FWSR0_ADDR       0x0000155C
#define    FWSR1_ADDR       0x00001560
#define    RFR_ADDR         0x00001568

/* MCU Register Values  */
#define    MRS_CAS_LAT_2    0x00000000
#define    MRS_CAS_LAT_3    0x00000001
#define    MRS_PRECHRG      0x00000002       
#define    MRS_NO_OP        0x00000003
#define    MRS_AUTO_RFRSH   0x00000004
#define    MRS_NORM_OP      0x00000006
#define    MRS_NOP_DELAY    0x00004000
#define    SDRC_INIT_VAL    0x00000014  /* don't care (drive strength N/A in model) */
/* next line changed from 0x00000004 */
#define    SBR0_INIT_VAL    0x00000010  /* 64 Meg Boundary (64 mbit device)*/
/* next line changed from 0x00000014 */
#define    SBR1_INIT_VAL    0x00000020  /* 128 Meg Boundary (64 mbit device)*/
#define    ECCR_INIT_VAL    0x0000000C  /* ECC enabled, correction on and no reporting */
#define    RFR_INIT_VAL     0x00000100  /* Initial Refresh Rate - 0x300 is not often enough for emul speed */
#define    FBSR1_INIT_VAL   0x00000004  /* 512K Bank Size */
#define    FWSR0_INIT_VAL   0x00000001  /* 1ws add-data (needed for PP state machine), 0ws recovery */
#define    FWSR1_INIT_VAL   0x00000000  /* 0ws add-data, 0ws recovery */

#define    SZ_0		0x00000000
#define    SZ_1		0x00000001
#define    SZ_2		0x00000002
#define    SZ_4		0x00000004
#define    SZ_8		0x00000008
#define    SZ_16	0x00000010
#define    SZ_32	0x00000020
#define    SZ_64	0x00000040
#define    SZ_128	0x00000080
#define    SZ_256	0x00000100
#define    SZ_512	0x00000200
#define    SZ_1K	0x00000400
#define    SZ_2K	0x00000800
#define    SZ_4K	0x00001000
#define    SZ_8K	0x00002000
#define    SZ_16K	0x00004000
#define    SZ_32K	0x00008000
#define    SZ_64K	0x00010000
#define    SZ_128K 	0x00020000
#define    SZ_256K 	0x00040000
#define    SZ_512K 	0x00080000
#define    SZ_1M	0x00100000
#define    SZ_2M	0x00200000
#define    SZ_4M	0x00400000
#define    SZ_8M	0x00800000
#define    SZ_16M	0x01000000
#define    SZ_32M	0x02000000
#define    SZ_64M	0x04000000
#define    SZ_128M	0x08000000
#define    SZ_256M	0x10000000
#define    SZ_512M 	0x20000000
#define    SZ_1G	0x40000000
#define    SZ_2G	0x80000000

#define		DRAM_SIZE	SZ_1M	/* size of available SDRAM */
#define		DCACHE_SIZE	SZ_32K	/* size of the Dcache */




#define DebugUART	        0x00600000

#define	UART_Receive		IMM(0x0)
#define	UART_Transmit		IMM(0x0)
#define	UART_InterruptEnable	IMM(0x1)
#define	UART_InterruptID	IMM(2)
#define	UART_FIFOControl	IMM(2)
#define	UART_LineControl	IMM(3)
#define	UART_ModemControl	IMM(4)
#define	UART_LineStatus		IMM(5)
#define	UART_ModemStatus	IMM(6)
#define	UART_Scratch		IMM(7)
#define	UART_DivisorLatchLSB	IMM(0)
#define	UART_DivisorLatchMSB	IMM(4)
#define BaudRateDivisor_19200	6
	

	.macro START_UART rd, reg
	LDR		\reg, =DebugUART
	LDR		\rd, =0 /*UARTLCR_DivisorLatchAccess		; enable access to divisor registers*/
	STRB		\rd, [\reg, UART_LineControl]

;	LDR		\rd, =BaudRateDivisor_600			; select baud rate
;	LDR		\rd, =BaudRateDivisor_1200			; select baud rate
	LDR		\rd, =BaudRateDivisor_19200			; select baud rate

	STRB	\rd, [\reg, UART_DivisorLatchLSB]
	MOV		\rd, \rd, LSR #8
	STRB	\rd, [\reg, UART_DivisorLatchMSB]

	MOV		\rd, #UARTLCR_CharLength8 :OR: UARTLCR_StopBits1	; 8 data, 1 stop bit, no parity
	STRB	\rd, [\reg, UART_LineControl]		; also disables access to divisor regs

	ldr		\rd, =0
	STRB	\rd, [\reg, UART_InterruptEnable]

	MOV		\rd, #UARTFCR_Enable :OR: UARTFCR_RXReset :OR: UARTFCR_TXReset :OR: UARTFCR_Mode0RXRDYTXRDY :OR: UARTFCR_RXTrigger1
	STRB	\rd, [\reg, UART_FIFOControl]

	ldr		\rd, =UARTMCR_DTRActive
	STRB	\rd, [\reg, UART_ModemControl]
	.endm
	

	.text

        .align     4
FUNC_START _bsp_init_stack
	/* Drain write and fill buffer */
	mcr	p15,0,r0,c7,c10,4
	CPWAIT	r0

	/* Disable write coalescing */
	mrc	p15,0,r0,c1,c0,1
	orr     r0, r0, IMM(1)				/* set the disable bit */
	mcr	p15,0,r0,c1,c0,1
	CPWAIT	r0

	/*
	 * SDRAM setup
	 */

	/* turn off refresh */
	ldr	r0, =RFR_ADDR
	ldr	r1, =0
	str	r1, [r0]

	/* issue NOP cmd */
	ldr	r0, =SDIR_ADDR
	ldr	r1, =MRS_NO_OP
	str	r1, [r0]

	/* delay at least 200uS */
	ldr     r0, =MRS_NOP_DELAY
    0:
	subs    r0, r0, IMM(1)
	bne     0b

	/* set refresh rate */
	ldr	r0, =RFR_ADDR
	ldr	r1, =RFR_INIT_VAL
	str	r1, [r0]

	/* issue "precharge all" */
	ldr	r0, =SDIR_ADDR
	ldr	r1, =MRS_PRECHRG
	str	r1, [r0]
	
	/* issue 8 auto refreshes */
	ldr	r1, =MRS_AUTO_RFRSH
	str	r1, [r0]
	str	r1, [r0]
	str	r1, [r0]
	str	r1, [r0]
	str	r1, [r0]
	str	r1, [r0]
	str	r1, [r0]
	str	r1, [r0]
	
	/* issue CAS latency 2 */
	ldr	r1, =MRS_CAS_LAT_2
	str	r1, [r0]
	
#if 0
	/* program SDRAM control register */
	ldr	r0, =SDCR_ADDR
	ldr	r1, =SDRC_INIT_VAL
	str	r1, [r0]
#endif

	/* SDRAM base */
	ldr	r0, =SDBR_ADDR
	ldr	r1, =RAM_ACTUAL_BASE
	str	r1, [r0]
	
	/* SDRAM bank0 boundary */
	ldr	r0, =SBR0_ADDR
	ldr	r1, =SBR0_INIT_VAL
	str	r1, [r0]

	/* SDRAM bank1 boundary */
	ldr	r0, =SBR1_ADDR
	ldr	r1, =SBR1_INIT_VAL
	str	r1, [r0]

        BOARD_BOOT_LED_DEBUG 2

        /*
         * Save the Return Address
         */
        mov     r8, lr

	/*
	 * Set the TTB register
         */
	ldr	r0, =SYM_NAME(page1)
        mcr     ARM_CACHE_COPROCESSOR_NUM,          \
                ARM_COPROCESSOR_OPCODE_DONT_CARE,   \
                r0,                                 \
                ARM_TRANSLATION_TABLE_BASE_REGISTER,               \
                ARM_COPROCESSOR_RM_DONT_CARE,       \
                ARM_COPROCESSOR_OPCODE_DONT_CARE

	ldr	r0, =0x55555555
        mcr     ARM_CACHE_COPROCESSOR_NUM,          \
                ARM_COPROCESSOR_OPCODE_DONT_CARE,   \
                r0,                                 \
                ARM_DOMAIN_ACCESS_CONTROL_REGISTER,               \
                ARM_COPROCESSOR_RM_DONT_CARE,       \
                ARM_COPROCESSOR_OPCODE_DONT_CARE
	
        ldr     sp, =0xA00FFFF0
     
        BOARD_BOOT_LED_DEBUG 3

        /*
         * Absolute address of 0: in r1.
         * This is so we can jump there after remapping
         * the ROM
         */
        ldr     r1, =0f

        /*
         * Now set the MMU Control Register to actually
         * enable the MMU, etc.
         */
        ldr     r0, =MMU_ENABLE_VALUE
        mcr     ARM_CACHE_COPROCESSOR_NUM,          \
                ARM_COPROCESSOR_OPCODE_DONT_CARE,   \
                r0,                                 \
                ARM_CONTROL_REGISTER,               \
                ARM_COPROCESSOR_RM_DONT_CARE,       \
                ARM_COPROCESSOR_OPCODE_DONT_CARE

        /*
         * Jump to the absolute address of the next instruction
         */
        mov     pc, r1
0:

/* BBtheEE 4-4-2000 BEGIN, added Coyanosa init. from BootROM */

	/*	ENABLE_CP_ALL			enable access to all coprocessor registers */
	ldr	r0, =0x2001			/* enable acess to all coprocessors */
	mcr	p15, 0, r0, c15, c1, 0

	/*	DRAIN_WRITE_FILL 		# drain the write and fill buffers */
	mcr p15, 0, r0, c7, c10, 4
	CPWAIT r0

	
	/*	FLUSH_ICACHE_DCACHE_BTB 	# flush the Icache, Dcache, & BTB */
	mcr p15, 0, r0, c7, c7, 0
	CPWAIT r0

	/*	FLUSH_I_D_TLB 			# flush the instruction & data TLBs	*/
	mcr p15, 0, r0, c8, c7, 0
	CPWAIT r0

	/*	ENABLE_ICACHE */
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, IMM(0x1000)			/* enable Icache */
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0
	
	/* scrub/init SDRAM if enabled/present */
	mov	r0, #0				/* scrub with 0x0000:0000 */
	mov	r1, #0
	mov	r2, #0				
	mov	r3, #0
	mov	r4, #0					
	mov	r5, #0
	mov	r6, #0					
	mov	r7, #0
	ldr	r11, =RAM_ACTUAL_BASE	/* base address of SDRAM */
	ldr	r12, =DRAM_SIZE		/* size of memory to scrub (1MB) */
10: /*fastScrubLoop */
	subs	r12, r12, #SZ_32	/* 32 bytes per whack */
	stmia	r11!, {r0-r7}
	bne	10b

/*	CLEAN_DRAIN_FLUSH r0, r1, r2	# flush the Dcaches */
/*	CLEAN_DRAIN_FLUSH	$t1, $t2, $t3 */
/*	CLEAN_DCACHE		$t1, $t2, $t3 */
	ldr	r1, =(RAM_ACTUAL_BASE+DRAM_SIZE)	/* use a CACHEABLE area of the memory map above SDRAM */
	mov	r0, #SZ_1K				/* number of lines in the Dcache */
20:
/*	ALLOCATE_DCACHE_LINE $t2 */
	mcr	p15, 0, r1, c7, c2, 5
	add	r1, r1, #SZ_32				/* increment the address to the next cache line */
	subs	r0, r0, #1				/* decrement the loop count */
	bne	20b

/*; CLEAN THE MINI-DATA CACHE */
/*; R2 contains the virtual address of a region of memory configured to access */
/*; Mini-Data Cache */
/*; R0 is the loop count; Iterate 64 times which is the number of lines in the */
/*; mini-data cache */
	ldr	r2, =(RAM_ACTUAL_BASE+DRAM_SIZE+DCACHE_SIZE)	/* use a CACHEABLE area of the memory map above SDRAM */
	mov	r0, #SZ_64
30:
/*	ALLOCATE_DCACHE_LINE $t3 */
	mcr	p15, 0, r2, c7, c2, 5
	add	r2, r2, #SZ_32 			/* Increment the address in $t3 to the next cache line. */
	subs	r0, r0, #1			/* Decrement loop count */
	bne	30b

/*	FLUSH_DCACHE $t1 */
	mcr p15, 0, r0, c7, c6, 0
	CPWAIT r0

/*	DRAIN_WRITE_FILL	$t1 */
	mcr p15, 0, r0, c7, c10, 4
	CPWAIT r0

/* BBtheEE 4-4-2000 END, added Coyanosa init. from BootROM */

/* BBtheEE 4-4-2000 BEGIN enable ECC if configured */
/*	ENABLE_ECC_COY r0				# enable ECC generation & checking for Coyanosa */
/*	BCU_WAIT \rd					# wait until the BCU isn't busy */
	mrc	p13, 0, r0, c0, c1, 0			/* read BCUCTL, update condition code flags */
	submi	pc, pc, #8
	
	ldr	r0, =0x0C				/* enable ECC w/single-bit correction */
	MCR	p13, 0, r0, c0, c1, 0

/* BBtheEE 4-4-2000 BEGIN enable ECC if configured */

#if 0
	/*	DISABLE_ICACHE */
	mrc	p15, 0, r0, c1, c0, 0
	ldr	r1,=0xffffefff
	and	r0, r0, r1
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0
#endif
	
        /*
         * Tell c_start where the top of ram is
         */
        BOARD_BOOT_LED_DEBUG 4
        mov     r0, sp

	/*
	 * Zero frame pointer to mark end of chain
	 */
	mov	r11,IMM(0)

        /*
         * Return
         */
        mov     pc, r8
FUNC_END _bsp_init_stack
