/*
 * mem285.S -- SDRAM Initialization for Intel(R) SA-110 21285 Companion Chip
 *
 * Copyright (c) 1999 Cygnus Support
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
 * Intel is a Registered Trademark of Intel Corporation.
 * Other Brands and Trademarks are the property of their respective owners.
 */
	.file "mem285.S"
        .title "SDRAM Init for Intel(R) SA-110 21285 Companion Chip"

#include <bsp/sa-110.h>
#include <bsp/cpu.h>
        
	.text
        .align     4


#define ARRAY_0_MODE_REGISTER     (SA110_SDRAM_ARRAY_0_MODE_REGISTER_BASE + 8)
#define ARRAY_1_MODE_REGISTER     (SA110_SDRAM_ARRAY_1_MODE_REGISTER_BASE + 8)
#define ARRAY_2_MODE_REGISTER     (SA110_SDRAM_ARRAY_2_MODE_REGISTER_BASE + 8)
#define ARRAY_3_MODE_REGISTER     (SA110_SDRAM_ARRAY_3_MODE_REGISTER_BASE + 8)

#define SDRAM_TIMING_VALUE_MIN    (SA110_SDRAM_ROW_PRECHARGE_2_CYCLES    | \
                                   SA110_SDRAM_LAST_DATA_IN_3_CYCLES     | \
                                   SA110_SDRAM_RAS_TO_CAS_DELAY_2_CYCLES | \
                                   SA110_SDRAM_CAS_LATENCY_2_CYCLES      | \
                                   SA110_SDRAM_ROW_CYCLE_TIME_6_CYCLES   | \
                                   SA110_SDRAM_COMMAND_DRIVE_1_CYCLE     | \
                                   SA110_SDRAM_REFRESH_INTERVAL_MIN)
#define SDRAM_TIMING_VALUE_NORMAL (SA110_SDRAM_ROW_PRECHARGE_2_CYCLES    | \
                                   SA110_SDRAM_LAST_DATA_IN_3_CYCLES     | \
                                   SA110_SDRAM_RAS_TO_CAS_DELAY_2_CYCLES | \
                                   SA110_SDRAM_CAS_LATENCY_2_CYCLES      | \
                                   SA110_SDRAM_ROW_CYCLE_TIME_6_CYCLES   | \
                                   SA110_SDRAM_COMMAND_DRIVE_1_CYCLE     | \
                                   SA110_SDRAM_REFRESH_INTERVAL_NORMAL)

#define PACKED_OFFSET              0x0c080400	

	/*
	 * This subroutine sizes and configures up to four banks of SDRAM DIMMs.
	 * It runs early without a stack.
	 *
	 * R0 - R9 are destroyed. All others preserved.
	 *
	 */
FUNC_START __mem285_init

        /*
         * Force an all-banks recharge on all four SDRAM arrays
         *
         * This code came from the SA-IOP ver 1.0 (3-16-98) spec pg 22
         *
         * You must access all four arrays regardless of whether there is
         * memory there because the 21285 counts the precharge accesses and
         * inhibits access to the SDRAM until all four have been done.
         *
         * An all banks rechargs is initiated by a read from any address
         * in the mode register space.
         */
        ldr     r0, =ARRAY_0_MODE_REGISTER
        ldr     r0, [r0]
        ldr     r0, =ARRAY_1_MODE_REGISTER
        ldr     r0, [r0]
        ldr     r0, =ARRAY_2_MODE_REGISTER
        ldr     r0, [r0]
        ldr     r0, =ARRAY_3_MODE_REGISTER
        ldr     r0, [r0]

        /*
         * Now we need to write to the SDRAM Mode Register.
         * The address is important, not the data.  The mode register
         * should be configured for a burst size of 4 with linear addressing
         */
        ldr     r0, =ARRAY_0_MODE_REGISTER
        str     r0, [r0, IMM(SA110_SDRAM_CAS_LATENCY_2_CYCLES)]
        ldr     r0, =ARRAY_1_MODE_REGISTER
        str     r0, [r0, IMM(SA110_SDRAM_CAS_LATENCY_2_CYCLES)]
        ldr     r0, =ARRAY_2_MODE_REGISTER
        str     r0, [r0, IMM(SA110_SDRAM_CAS_LATENCY_2_CYCLES)]
        ldr     r0, =ARRAY_3_MODE_REGISTER
        str     r0, [r0, IMM(SA110_SDRAM_CAS_LATENCY_2_CYCLES)]

        /*
         * Write to the SDRAM Timing Register in the 21285.  Set the
         * refresh interval to the minimum because we have to wait for
         * 8 refresh cycles to complete before we can rely on the SDRAMs
         * to be operating normally
         */
	ldr	r0, =SA110_CONTROL_STATUS_BASE
        ldr     r1, =SDRAM_TIMING_VALUE_MIN
        str     r1, [r0, IMM(SA110_SDRAM_TIMING_o)]

	/* Disable each array */
	ldr	r1, =0
        str     r1, [r0, IMM(SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o)]
        str     r1, [r0, IMM(SA110_SDRAM_ADDRESS_SIZE_ARRAY_1_o)]
        str     r1, [r0, IMM(SA110_SDRAM_ADDRESS_SIZE_ARRAY_2_o)]
        str     r1, [r0, IMM(SA110_SDRAM_ADDRESS_SIZE_ARRAY_3_o)]
	
        /*
         * Wait for 8 refresh cycles to complete
         */
        ldr     r1, =(8 * 32)
    1:  subs    r1, r1, IMM(1)
        bgt     1b

        /*
         * Now reset the Refresh interval to a sensible value
         * SDRAM_TIMING address should be in R1
         */
        ldr     r1, =SDRAM_TIMING_VALUE_NORMAL
        str     r1, [r0, IMM(SA110_SDRAM_TIMING_o)]
	
	/* start out assuming 64M part with MUX mode 2 */
	ldr	r1, =(SA110_SDRAM_SIZE_64MB | SA110_SDRAM_MUX_MODE2)
	ldr	r2, =(64 << 20)
        str     r1, [r0, IMM(SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o)]
	add	r1, r1, r2
        str     r1, [r0, IMM(SA110_SDRAM_ADDRESS_SIZE_ARRAY_1_o)]
	add	r1, r1, r2
        str     r1, [r0, IMM(SA110_SDRAM_ADDRESS_SIZE_ARRAY_2_o)]
	add	r1, r1, r2
        str     r1, [r0, IMM(SA110_SDRAM_ADDRESS_SIZE_ARRAY_3_o)]
	
	/*
	 * First, try to figure out which banks are populated and
	 * the real mux mode for those banks.
	 *
	 * At this point:
	 *   r1 - Base address of last SDRAM array.
	 *   r0 - Base of control/status registers
	 *
	 * Register usage:
	 *   r8 - offset to SDRAM addr/size register
	 *   r7 - result accumulator
	 *   r6 - loop count
	 *   r5 - pattern
	 *   r4 - inverse pattern
	 *   r3 - scratch
	 */	
	ldr	r8, =SA110_SDRAM_ADDRESS_SIZE_ARRAY_3_o
	ldr	r7, =0
	ldr	r6, =4
		
	ldr	r5, =0x12345678
	mvn	r4, r5
    1:
	str	r5, [r1]
	str	r4, [r1, IMM(4)]  /* put something else on the data bus */
	ldr	r3, [r1]
	subs	r3, r3, r5

	/* If we didn't read pattern, then no memory present */
	ldrne	r3, =SA110_SDRAM_SIZE_64MB
	bne	2f

	/*
	 * This bank is populated, so try to determine mux mode.
	 * All banks are currently set for mux mode 2.
	 */

	/* A21 distinguishes mux mode 0. */
	add	r3, r1, IMM((1 << 21))
	str	r4, [r3]
	ldr	r3, [r1]
	subs	r3, r3, r5
	/* If we don't read back pattern, then its mux mode 0 */
	ldrne	r3, =(SA110_SDRAM_SIZE_64MB | SA110_SDRAM_MUX_MODE0)
	bne	2f

	/* A23 distinguishes mux mode 2. */
	str	r5, [r1]		/* write pattern */
	add	r3, r1, IMM((1 << 23))
	str	r4, [r3]		/* write inverse pattern */
	ldr	r3, [r1]		/* read back pattern */
	subs	r3, r3, r5
	/* if pattern still there, then mode 2 */
	ldreq	r3, =(SA110_SDRAM_SIZE_64MB | SA110_SDRAM_MUX_MODE2)
	beq	2f

	/* A22 distinguishes mux mode 4. */
	str	r5, [r1]		/* write pattern */
	add	r3, r1, IMM((1 << 22))
	str	r4, [r3]		/* write inverse pattern */
	ldr	r3, [r1]		/* read back pattern */
	subs	r3, r3, r5
	/* if pattern A still there, its mode 4 */
	ldreq	r3, =(SA110_SDRAM_SIZE_64MB | SA110_SDRAM_MUX_MODE4)
	beq	2f

	/*
	 * At this point it is either mode 1 or 3. There is no clear cut
	 * test to differentiate the two, so make a best guess now, then
	 * correct later (if necessary) while sizing the bank.
	 */
	str	r5, [r1]		/* write pattern */
	add	r3, r1, IMM((1 << 24))
	str	r4, [r3]		/* write inverse pattern */
	ldr	r3, [r1]		/* read back pattern */
	subs	r3, r3, r5
	/* If pattern, try mode 1 */
	ldreq	r3, =(SA110_SDRAM_SIZE_64MB | SA110_SDRAM_MUX_MODE1)
	/* otherwise, mode 3 */
	ldrne	r3, =(SA110_SDRAM_SIZE_64MB | SA110_SDRAM_MUX_MODE3)
    2:
	add	r7, r3, r7, lsl IMM(8)		/* include this bank in overall result */
	/* if 64M/mode0, force to 32M */
	cmp	r3, IMM((SA110_SDRAM_SIZE_64MB | SA110_SDRAM_MUX_MODE0))
	subeq	r3, r3, IMM(1)
	add	r3, r3, r1			/* add in base address */
	str	r3, [r0, r8]			/* write to addr/size register */
	sub	r8, r8, IMM(4)			/* previous addr/size register */
	sub	r1, r1, IMM((64<<20))		/* previous array */
	subs	r6, r6, IMM(1)			/* decrement loop count */
	bgt	1b

	/*
	 * Now that mux modes are (hopefully) setup, we can try to size
	 * the SDRAM arrays.
	 *
	 * At this point:
	 *   r7 - packed size/mode bytes for each array
	 *   r5 - pattern
	 *   r0 - Base of control/status registers
	 *
	 * Register usage:
	 *   r8 - mask shift value
	 *   r6 - result accumulator
	 *   r4 - scratch
	 *   r3 - address increment/decrement size (1M)
	 *   r2 - inner loop count/temp size result
	 *   r1 - address for testing
	 */
	ldr	r1, =(1 << 28)		/* presumed end address for bank3 */
	ldr	r8, =24
	ldr	r6, =0
    9:
	ldr	r2, =64			/* 64 steps */
	ldr	r3, =(1 << 20)		/* of 1M each */
    1:
	sub	r1, r1, r3
	subs	r2, r2, IMM(1)
	str	r2, [r1]
	bgt	1b
	mov	r4, r1
	str	r5, [r4, IMM(4)]	/* change pattern on data bus */
    2:
	/* search for first unexpected data */
	ldr	r5, [r4]
	subs	r5, r5, r2
	bne	3f
	add	r4, r4, r3	/* next address */
	add	r2, r2, IMM(1)
	cmp	r2, IMM(64)
	blt	2b
    3:	
	mov	r4, r7, lsr r8	/* current size/mode bits */
	and	r4, r4, IMM(SA110_SDRAM_MUX_MODE_MASK)
	/* if its not mode3, then it must be 8M or misconfigured */
	cmp	r4, IMM(SA110_SDRAM_MUX_MODE3)
	bne	4f
	cmp	r2, IMM(8)
	beq	4f
	/*
	 * Must be misconfigured mux mode. Set to mode 1 and retry
	 */
	ldr	r4, =0xff       /* byte mask */
	mov	r4, r4, lsl r8  /* shift to appropriate bank */
	bic	r7, r7, r4	/* clear current mask */
	ldr	r4, =SA110_SDRAM_MUX_MODE1
	mov	r4, r4, lsl r8  /* shift to appropriate bank */
	add	r7, r7, r4

	mov	r3, r8, lsr IMM(1)
	add	r3, r3, IMM(SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o)
	ldr	r4, [r0, r3]	  /* get current addr/size/mode value */
	bic	r4, r4, IMM(0xff) /* clear current size/mode */
	add	r4, r4, IMM(SA110_SDRAM_MUX_MODE1)
	str	r4, [r0, r3]	  /* store new value */
	add	r1, r1, IMM((64 << 20))
	b	9b
    4:
	/* convert to MB size to register size val */
	ldr	r4, =0
	cmp	r2, r4
	beq	6f
    5:
	add	r4, r4, IMM(1)
	tst	r2, IMM(1)
	mov	r2, r2, lsr IMM(1)
	beq	5b
    6:
	mov	r4, r4, lsl r8	/* shift size to appropriate byte */
	add	r6, r6, r4	/* accumulate it */
	subs	r8, r8, IMM(8)  /* adjust shift count for next array */
	bge	9b

	/*
	 * At this point, r6 has packed size values for all banks
	 *
	 * We want to set memory up to be contiguous. Since the
	 * banks' base address needs to be naturally aligned, we
	 * need to sort the bank sizes from large to small.
	 *
	 * Register usage:
	 *  
	 *  r1-r4 - bank sizes
	 *  r5    - bank ordering
	 *  r7    - scratch
	 *  r8    - scratch
	 *
	 */
	and	r1, r6, IMM(7)
	mov	r2, r6, lsr IMM(8)
	and	r2, r2, IMM(7)
	mov	r3, r6, lsr IMM(16)
	and	r3, r3, IMM(7)
	mov	r4, r6, lsr IMM(24)
	and	r4, r4, IMM(7)
	
	ldr	r5, =PACKED_OFFSET

	cmp	r1, r2
	bge	1f
	/* swap r1 and r2 */
	mov	r7, r1
	mov	r1, r2
	mov	r2, r7
	/* swap register offset values */
	bic	r6, r6, IMM(0xff00)
	add	r6, r6, IMM(8)
    1:
	cmp	r1, r3
	bge	1f
	/* swap r1 and r3 */
	mov	r7, r1
	mov	r1, r3
	mov	r3, r7
	/* swap register offset values */
	and	r7, r6, IMM(0xff)		
	and	r8, r6, IMM(0xff0000)
	bic	r6, r6, IMM(0xff)
	bic	r6, r6, IMM(0xff0000)
	add	r6, r6, r8, lsr IMM(16)
	add	r6, r6, r7, lsl IMM(16)
    1:
	cmp	r1, r4
	bge	1f
	/* swap r1 and r4 */
	mov	r7, r1
	mov	r1, r4
	mov	r4, r7
	/* swap register offset values */
	and	r7, r6, IMM(0xff)		
	and	r8, r6, IMM(0xff000000)
	bic	r6, r6, IMM(0xff)
	bic	r6, r6, IMM(0xff000000)
	add	r6, r6, r8, lsr IMM(24)
	add	r6, r6, r7, lsl IMM(24)
    1:
	cmp	r2, r3
	bge	1f
	/* swap r2 and r3 */
	mov	r7, r2
	mov	r2, r3
	mov	r3, r7
	/* swap register offset values */
	and	r7, r6, IMM(0xff00)
	and	r8, r6, IMM(0xff0000)
	bic	r6, r6, IMM(0xff00)
	bic	r6, r6, IMM(0xff0000)
	add	r6, r6, r8, lsr IMM(8)
	add	r6, r6, r7, lsl IMM(8)
    1:
	cmp	r2, r4
	bge	1f
	/* swap r2 and r4 */
	mov	r7, r2
	mov	r2, r4
	mov	r4, r7
	/* swap register offset values */
	and	r7, r6, IMM(0xff00)
	and	r8, r6, IMM(0xff000000)
	bic	r6, r6, IMM(0xff00)
	bic	r6, r6, IMM(0xff000000)
	add	r6, r6, r8, lsr IMM(16)
	add	r6, r6, r7, lsl IMM(16)
    1:
	cmp	r3, r4
	bge	1f
	/* swap r3 and r4 */
	mov	r7, r3
	mov	r3, r4
	mov	r4, r7
	/* swap register values */
	and	r7, r6, IMM(0xff0000)
	and	r8, r6, IMM(0xff000000)
	bic	r6, r6, IMM(0xff0000)
	bic	r6, r6, IMM(0xff000000)
	add	r6, r6, r8, lsl IMM(8)
	add	r6, r6, r7, lsr IMM(8)
    1:
	/*
	 * Now we update the registers.
	 *
	 * Register usage:
	 *  
	 *  r1-r4 - bank sizes sorted large to small
	 *  r5    - packed register byte offsets
	 *  r7    - base address
	 *  r8    - scratch
	 *  r9    - scratch
	 */
	ldr	r7, =0
	and	r8, r5, IMM(0x0f)
	add	r8, r8, IMM(SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o)
	ldr	r9, [r0, r8]
	and	r9, r9, IMM(SA110_SDRAM_MUX_MODE_MASK)
	add	r9, r9, r1	/* add size */
	add	r9, r9, r7	/* add base address */
	str	r9, [r0, r8]	/* write it back */
	cmp	r1, IMM(0)
	beq	1f
	sub	r1, r1, IMM(1)
	ldr	r9, =(1 << 20)
	mov	r9, r9, lsl r1
	add	r7, r7, r9
	
	mov	r5, r5, lsr IMM(8)
	and	r8, r5, IMM(0x0f)
	add	r8, r8, IMM(SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o)
	ldr	r9, [r0, r8]
	and	r9, r9, IMM(SA110_SDRAM_MUX_MODE_MASK)
	add	r9, r9, r2	/* add size */
	add	r9, r9, r7	/* add base address */
	str	r9, [r0, r8]	/* write it back */
	cmp	r2, IMM(0)
	beq	1f
	sub	r2, r2, IMM(1)
	ldr	r9, =(1 << 20)
	mov	r9, r9, lsl r2
	add	r7, r7, r9
	
	mov	r5, r5, lsr IMM(8)
	and	r8, r5, IMM(0x0f)
	add	r8, r8, IMM(SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o)
	ldr	r9, [r0, r8]
	and	r9, r9, IMM(SA110_SDRAM_MUX_MODE_MASK)
	add	r9, r9, r3	/* add size */
	add	r9, r9, r7	/* add base address */
	str	r9, [r0, r8]	/* write it back */
	cmp	r3, IMM(0)
	beq	1f
	sub	r3, r3, IMM(1)
	ldr	r9, =(1 << 20)
	mov	r9, r9, lsl r3
	add	r7, r7, r9
	
	mov	r5, r5, lsr IMM(8)
	and	r8, r5, IMM(0x0f)
	add	r8, r8, IMM(SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o)
	ldr	r9, [r0, r8]
	and	r9, r9, IMM(SA110_SDRAM_MUX_MODE_MASK)
	add	r9, r9, r4	/* add size */
	add	r9, r9, r7	/* add base address */
	str	r9, [r0, r8]	/* write it back */
	cmp	r4, IMM(0)
	beq	1f
	sub	r4, r4, IMM(1)
	mov	r9, IMM((1 << 20))
	mov	r9, r9, lsl r4
	add	r7, r7, r9
    1:
	/*
	 * Whew! That was fun.
	 * R7 now points just past the end of SDRAM.
	 */
	mov	r0, r7
	mov	pc, lr	
FUNC_END __mem285_init
