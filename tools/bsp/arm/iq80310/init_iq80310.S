/*
 * init_iq80310.S -- Low-level Board Initialization.
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
	.file "init_iq80310.S"

#include <bsp/iq80310.h>

/* Companion chip MCU registers */
#define    MMR_BASE         0x00001500
#define    SDIR_OFF         0x00000000
#define    SDRC_OFF         0x00000004
#define    SDBR_OFF         0x00000008
#define    SBR0_OFF         0x0000000C
#define    SBR1_OFF         0x00000010
#define    ECCR_OFF         0x00000034
#define    FEBR1_OFF        0x00000050
#define    FBSR1_OFF        0x00000058
#define    FWSR0_OFF        0x0000005C
#define    FWSR1_OFF        0x00000060
#define    RFR_OFF          0x00000068

/* MCU Register Values  */
#define    MRS_CAS_LAT_2    0x00000000
#define    MRS_CAS_LAT_3    0x00000001
#define    MRS_PRECHRG      0x00000002       
#define    MRS_NO_OP        0x00000003
#define    MRS_AUTO_RFRSH   0x00000004
#define    MRS_NORM_OP      0x00000006
#define    MRS_NOP_DELAY    0x00004000

#define    SBR0_INIT_VAL    0x00000008  /* 32 Meg Boundary (64 mbit device)*/
#define    SBR1_INIT_VAL    0x00000008  /* 32 Meg Boundary (64 mbit device)*/
#define    ECCR_INIT_VAL    0x0000000C  /* ECC enabled, correction on and no reporting */
#define    RFR_INIT_VAL     0x00000300  /* Initial Refresh Rate */
#define    FBSR1_INIT_VAL   0x00000040  /* 8MB Bank Size */
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

#define		DRAM_SIZE	SZ_32M	/* size of available SDRAM */
#define		DCACHE_SIZE	SZ_32K	/* size of the Dcache */
#define		DRAM_BASE	0xa0000000	/*  size of available SDRAM */

#define	SDRAM_DEVID	0xA2  /* SDRAM Device ID  */
	
#define DebugUART	        0xFE810000  /* use UART2 */

	
#define	UART_Receive			IMM(0x0)
#define	UART_Transmit			IMM(0x0)
#define	UART_InterruptEnable	IMM(0x1)
#define	UART_InterruptID		IMM(2)
#define	UART_FIFOControl		IMM(2)
#define	UART_LineControl		IMM(3)
#define	UART_ModemControl		IMM(4)
#define	UART_LineStatus			IMM(5)
#define	UART_ModemStatus		IMM(6)
#define	UART_Scratch			IMM(7)
#define	UART_DivisorLatchLSB	IMM(0)
#define	UART_DivisorLatchMSB	IMM(1)
#define BaudRateDivisor_19200	6
#define BaudRateDivisor_38400	3
#define BaudRateDivisor_115200	1

/* 7-segment display addresses */
#define DISPLAY_LEFT	0xFE840000
#define DISPLAY_RIGHT	0xFE850000
	
/********** Cyclone additions (not all used here) **************/





/****************************************
 * Memory Controller Register Addresses *
 *     00001500H through 000015FFH      *
 ****************************************/

#define SDIR_ADDR	0x00001500  /* Address of the SDRAM Initialization Register */

/* SDRAM commands */
#define		MRS2_CMD	0   /* Mode-Register-Set Command where CAS# Latency = 2 */
#define		MRS3_CMD	1   /* Mode-Register-Set Command where CAS# Latency = 3 */
#define		PRECH_CMD	2   /* Precharge-All Command */
#define		NOP_CMD		3   /* NOP Command */
#define		AUTO_CMD	4   /* Auto-Refresh Command */
#define		NML_CMD		6   /* Normal SDRAM Operation */
#define		SDCR_ADDR	0x00001504  /* Address of the Refresh Counter */

/* SDRAM control bits */
#define		BUS_DRV_BIT		3
#define		CE0_DRV_BIT		4
#define		CE1_DRV_BIT		5
#define		DMASK_DRV_BIT	6
#define		AC_DRV_BIT		7
#define		SDBR_ADDR	0x00001508  /* Address of the SDRAM Base Register        */
#define		SBR0_ADDR	0x0000150C  /* Address of the SDRAM Bank 0 Size Register */
#define		SBR1_ADDR	0x00001510  /* Address of the SDRAM Bank 1 Size Register */
#define		ECCR_ADDR	0x00001534  /* Address of the EEC Control Register       */
#define		ELOG0_ADDR	0x00001538  /* Address of the EEC Log 0 Register         */
#define		ELOG1_ADDR	0x0000153C  /* Address of the EEC Log 1 Register         */
#define		ECAR0_ADDR	0x00001540  /* Address of the EEC Address 0 Register     */
#define		ECAR1_ADDR	0x00001544  /* Address of the EEC Address 1 Register     */
#define		ECTST_ADDR	0x00001548  /* Address of the EEC Test Register          */
#define		FEBR0_ADDR	0x0000154C  /* Address of the Flash Base 0 Register      */
#define		FEBR1_ADDR	0x00001550  /* Address of the Flash Base 1 Register      */
#define		FBSR0_ADDR	0x00001554  /* Address of the Flash Bank 0 Size Register */
#define		FBSR1_ADDR	0x00001558  /* Address of the Flash Bank 1 Size Register */

/**************************
 * I2C Bus Interface Unit *
 **************************/

/* Control Register */
#define	ICR_ADDR		0x00001680  /* Address */
#define	ICR_START		0x0001  /* 1:send a Start condition to the I2C when in master mode */
#define	ICR_STOP		0x0002  /* 1:send a Stop condition after next data byte transferred on I2C bus in master mode */
#define	ICR_ACK			0x0004  /* Ack/Nack control: 1:Nack, 0:Ack (negative or positive pulse) */
#define	ICR_TRANSFER	0x0008  /* 1:send/receive byte, 0:cleared by I2C unit when done */
#define	ICR_ABORT		0x0010  /* 1:I2C sends STOP w/out data permission, 0:ICR bit used only */
#define	ICR_SCLENB		0x0020  /* I2C clock output: 1:Enabled, 0:Disabled. ICCR configured before ! */
#define	ICR_ENB			0x0040  /* I2C unit: 1:Enabled, 0:Disabled */
#define	ICR_GCALL		0x0080  /* General Call: 1:Disabled, 0:Enabled */
#define	ICR_IEMPTY		0x0100  /* 1: IDBR Transmit Empty Interrupt Enable */
#define	ICR_IFULL		0x0200  /* 1: IDBR Receive Full Interrupt Enable */
#define	ICR_IERR		0x0400  /* 1: Bus Error Interrupt Enable */
#define	ICR_ISTOP		0x0800  /* 1: Slave Stop Detected Interrupt Enable */
#define	ICR_IARB		0x1000  /* 1: Arbitration Loss Detected Interrupt Enable */
#define	ICR_ISADDR		0x2000  /* 1: Slave Address Detected Interrupt Enable */
#define	ICR_RESET		0x4000  /* 1: I2C unit reset */

/* Status Register */
#define	ISR_ADDR		0x00001684  /* Address */
#define	ISR_RWMODE		0x0001  /* 1: I2C in master receive = slave transmit mode */
#define	ISR_ACK			0x0002  /* 1: I2C received/sent a Nack, 0: Ack */
#define	ISR_BUSY		0x0004  /* 1: Processor's I2C unit busy */
#define	ISR_BUSBUSY		0x0008  /* 1: I2C bus busy. Processor's I2C unit not involved */
#define	ISR_STOP		0x0010  /* 1: Slave Stop detected (when in slave mode: receive or transmit) */
#define	ISR_ARB			0x0020  /* 1: Arbitration Loss Detected */
#define	ISR_EMPTY		0x0040  /* 1: Transfer finished on I2C bus. If enabled in ICR, interrupt signaled */
#define	ISR_FULL		0x0080  /* 1: IDBR received new byte from I2C bus. If ICR, interrupt signaled */
#define	ISR_GCALL		0x0100  /* 1: I2C unit received a General Call address */
#define	ISR_SADDR		0x0200  /* 1: I2C unit detected a 7-bit address matching the general call or ISAR */
#define	ISR_ERROR		0x0400  /* Bit set by unit when a Bus Error detected */

#define	ISAR_ADDR		0x00001688  /* Address of the I2C Slave Address Register */
#define	IDBR_ADDR		0x0000168C  /* Address of the I2C Data Buffer Register */
#define	IDBR_MASK		0x000000ff
#define	IDBR_MODE		0x01
#define	ICCR_ADDR		0x00001690  /* Address of the I2C Clock Control Register */
#define	IBMR_ADDR		0x00001694  /* Address of the I2C Bus Monitor Register */

/* Codes for ROM bank size in bytes	*/
#define		ROM_0K		0
#define		ROM_64K		1
#define		ROM_128K	2
#define		ROM_256K	3
#define		ROM_512K	4
#define		ROM_1M		5
#define		ROM_2M		6
#define		ROM_4M		7
#define		ROM_8M		8
#define		FWSR0_ADDR	0x0000155C  /* Flash Wait State 0 Register */
#define		FWSR1_ADDR	0x00001560  /* Flash Wait State 1 Register */
#define		MCISR_ADDR	0x00001564  /* Memory Controller Interrupt Status Register */
#define		RFR_ADDR	0x00001568  /* Refresh Frequency Register */

/* ROM Memory Base Addresses for the PCI939 board */
#define ROM0_BASE_PCI939	0xFEE00000  

/* Processor I2C Device ID */
#define I2C_DEVID	0x02  /* I2C slave address to which the unit responds when in slave-receive mode */

/* Timeout limit for SDRAM EEPROM to respond */
#define I2C_TIMOUT	0x1000000  /* bumped this way up...used to be 0x100000*/ 

/* Reserved area for battery backup manufacturing test		*/
/* This area is not zeroed out by init code					*/
#define	SDRAM_BATTERY_TEST_BASE		0xA07FFFF0
#define BATTERY_TEST_PATTERN		0xBAEBAEBA

/* UART uses the ROM1 controller */
#define ROM1_BASE			0xE0000000

/**************************************************************************/

/* Macros for diagnostics */
#define UART_SCRATCH		0xe0000007
#define DUMMY_LOC			0xa0001000

#define DRAM_SIZE_LOC	0x000003fc  /* Address where the size of the the SDRAM is saved */
/* Codes for SDRAM size */
#define SDRAM_8M	0x00800000
#define SDRAM_16M	0x01000000
#define SDRAM_32M	0x02000000
#define SDRAM_64M	0x04000000
#define SDRAM_128M	0x08000000
#define SDRAM_256M	0x10000000

/* Definition of the addresses of the 7-segment displays registers */
#define MSB_DISPLAY_ADDR		0xe0040000	/* Write only */
#define LSB_DISPLAY_ADDR		0xe0050000	/* Write only */

/* Values for the 7-segment displays (negative truth) */
#define DISPLAY_OFF		0xff  /* All segments and decimal point off */
#define ZERO			0x40
#define ONE				0x79
#define TWO				0x24
#define THREE			0x30
#define DISPLAY_ERROR	0x06  /* Displays "E" for "Error". */
#define FIRST_TEST_FAILED_ADDR	0x000003fa       /* See cyrn_z.h, line 192 */
#define LAST_TEST_PASSED_ADDR	0x000003fb       /* See cyrn_z.h, line 193 */
#define LED_DELAY		0x2AA9DA

/* SDRAM configuration */
#define MEMBASE_DRAM	0xa0000000  /* Address */
/* SDRAM bank size values (SPD << 2) */
#define		RAM_0MEG	
#define		RAM_4MEG	4
#define		RAM_8MEG	8
#define		RAM_16MEG	16
#define		RAM_32MEG	32
#define		RAM_64MEG	64
#define     RAM_128MEG	128
#define     RAM_256MEG	256

/* SBR register definitions (valid bits are [7:3])*/
#define		SBR_32MEG	0x08
#define		SBR_64MEG	0x10
#define		SBR_128MEG	0x20
#define		SBR_256MEG	0x40

/* Drive Strengths */
#define    SDRC_1BANK    0x1520  /* assume each bank is x8 devices */
#define    SDRC_2BANK	 0x1548


/* SDRAM PD bytes				*/
#define		BANKCNT_BYTE	0x06  /* Byte #5 of SPD: number of module banks */
#define		BANKSZ_BYTE		0x20  /* Byte #31 of SPD: module bank density */
#define		CHECKSUM_BYTE	0x40  /* Byte #63 of SPD: checksum for bytes 0-62 */
#define		CONFIG_BYTE		0x0C  /* Byte #11 of SPD: DIMM configuration type (Parity or not, EEC) */



/***************************************/




/***************************************************************************
 *	macros
 */

/* 
 * Display 'lvalue:rvalue' on the hex display
 * lvalue and rvalue must be of the form 'DISPLAY_x'
 * where 'x' is a hex digit from 0-F.
 */
	.macro HEX_DISPLAY reg0, reg1, lvalue, rvalue	
	ldr	\reg0, =DISPLAY_LEFT		/* display left digit */
	ldr	\reg1, =\lvalue
	str	\reg1, [\reg0]
	ldr	\reg0, =DISPLAY_RIGHT
	ldr	\reg1, =\rvalue			/* display right digit */
	str	\reg1, [\reg0]
	.endm

/* 
 * Trigger the logic analyzer by writing a particular
 * address, and triggering on that address.
 */
	.macro TRIGGER_LA_ON_ADDRESS address, reg0, reg1
	mrc	p15, 0, \reg0, c1, c0, 0     /* read ARM control register */
/*	CPWAIT  \reg0 */
	ldr	\reg1, =\address
	str	\reg0, [\reg1]
	.endm

/*
 * Delay a bit
 */
	.macro DELAY_FOR cycles, reg0
	ldr	\reg0, =\cycles
	subs	\reg0, \reg0, #1
	subne	pc,  pc, #0xc
	.endm
	
/***************************************************************************
 * BSP initialization begins here
 */
	
	
	.text

        .align     4
FUNC_START _bsp_init_stack
	/* Drain write and fill buffer */
	mcr	p15,0,r0,c7,c10,4
	CPWAIT	r0
	CPWAIT	r0

	/* Disable write coalescing */
	mrc	p15,0,r0,c1,c0,1
	orr	r0,r0,#1		/* set the disable bit */
	mcr	p15,0,r0,c1,c0,1
	CPWAIT	r0

	DELAY_FOR 0x20000, r0 /* Delay appx 2 ms to let battery-backup reset complete */
                          /* Eventually we will be able to check a register bit
                             to determine when this is complete */

	/*ldr	r2, =0x1040		 quit PCI retries 
	mov	r3, #0x08
	str	r3, [r2] */

	HEX_DISPLAY r0, r1, DISPLAY_0, DISPLAY_1
		



/*******  I2C interface initialization ****************************************/

	/*  Setup I2C Slave Address Register */  
	ldr	r1, =I2C_DEVID			/* Load slave address r1.*/
	ldr	r2, =ISAR_ADDR		    /* Load address of the I2C Slave Address Register in r2. */
	ldr	r3, =0x0000007f			/* Load mask in r3. */
	and	r1, r3, r3				/* The mask zeroes the 25 MSBs of r1 just to make sure. */
	str	r3, [r2]				/* Save the value 0x02 (I2C_DEVID) in the register. */


	/*  Setup I2C Clock Count Register */  
	ldr	r2, =ICCR_ADDR			/* Load the address of the I2C Clock Control Register in r2. */
	ldr     r3, =0x0000014d			/* Set for 5.05 us transition time at 66MHz (0x14D = 333). */
	str	r3, [r2]	            /* Save the value in the register. */


	/*  Enable I2C Interface Unit - status will be polled */  
	ldr	r2, =ICR_ADDR			/* Load the address of the Control Register in r2. */
	ldr	r1, =ICR_GCALL			/* Disable General Call (will be master) */
	ldr	r3, =ICR_ENB			/* Enable I2C unit ). */
	orr	r1, r3, r1				/* OR the two and store in R1 */
	ldr	r3, =ICR_SCLENB			/* Enable I2C Clock Generator disabled */
	orr	r1, r3, r1				/* OR the two and store in R1 */
	str	r1, [r2]				/* Save the value to the Control Register. */

	ldr	r0, =0xa0200010
	str	r1, [r0]
	

/****** Now read the SPD Data *************************************************/

	/* Pointers to I2C Registers */
	ldr	r11, =ICR_ADDR		/* Load the address of the I2C Control Register in r11. */
	ldr	r12, =ISR_ADDR		/* Load the address of the I2C Status Register in r12. */
	ldr	r13, =IDBR_ADDR		/* Load the address of the I2C Data Buffer Register in r13. */


	/* Initialize byte counters */
	ldr	r6, =0x00000000  /* Counter incremented before byte is read */
	ldr	r7, =0x00000040  /* Number of bytes to read in the Presence Detect EEPROM of SDRAM: 64 bytes */
	ldr	r5, =0x00000000  /* R5 has running checksum calculation */
	ldr	r9, =I2C_TIMOUT  /* Timeout limit in case EEPROM does not respond */


	/* At the end of all this, R4 has DRAM size, R8 has bank count, and R10 has Bank size */
	ldr	r10,=0x00000000  /* Bank size */
	ldr	r8, =0x00000000  /* Bank count */
	ldr	r4, =0x00000000  /* SDRAM size */



	/*  FREE REGISTERS ARE R0 - R3 */


/*** Put out address, with WRITE mode ***/	

	/* Set SDRAM module address and write mode */  
	ldr	r1, =SDRAM_DEVID	/* Load slave address for SDRAM module: 0xA2 (Presence Detect Data) */
	bic	r1, r1, #IDBR_MODE  /* Clear read bit (bit #0) */
	str	r1, [r13]			/* Store to data register */	


	/* Initiate dummy write to set EEPROM pointer to 0 */
	ldr	r1, [r11]			  /* read the current Control Register value */ 
	orr	r1, r1,	#ICR_START	  /* Set start bit */
	orr	r1, r1, #ICR_TRANSFER /* Set transfer bit - bit is self_clearing */
	str	r1, [r11]			  /* Store to control register */


	/* Wait for transmit empty status */
	ldr	r1, =0x00000000		/* Initialize I2C timeout counter */
0:
	add	r1, r1, #1			/* Increment I2C timeout counter (r1 = r1 + 1) */
	cmp	r1, r9
	beq	i2c_error			/* Kick out of SDRAM initialization if timeout occurs */
	ldr	r0, [r12]			/* Load I2C Status Reg into R0 */
	ldr	r2, =ISR_EMPTY		/* Poll status register */
	and	r3, r2, r0			/* Bit #6 is checked: IDBR Transmit Empty */
	cmp	r3, r2				/* If bit = 0 then branch to 0 and check again */
	bne	0b		
	str	r0, [r12]			/* Write back status to clear */


/*** Write pointer register on EEPROM to 0x00000000 ***/

	/*  Set SDRAM module EEPROM address to 0 */  
	ldr	r1, =0x00000000		/* Load base address of SDRAM module EEPROM */
	str	r1, [r13]			/* Store to data register */	

	/*  Send address to EEPROM */  
	ldr	r1, [r11]			  /* read the current Control Register value */ 
	bic	r1, r1,	#ICR_START	  /* No start bit (already started) */
	orr	r1, r1, #ICR_TRANSFER /* Set transfer bit - bit is self_clearing */
	str	r1, [r11]			  /* Store to control register */

	/* Wait for transmit empty status */
	ldr	r1, =0x00000000		/* Initialize I2C timeout counter */
0:
	add	r1, r1, #1			/* Increment I2C timeout counter (r1 = r1 + 1) */
	cmp	r1, r9
	beq	i2c_error			/* Kick out of SDRAM initialization if timeout occurs */
	ldr	r0, [r12]			/* Load I2C Status Reg into R0 -  ld	(r12), r10*/
	ldr	r2, =ISR_EMPTY		/* Poll status register */
	and	r3, r2, r0			/* Bit #6 is checked: IDBR Transmit Empty */
	cmp	r3, r2				/* If bit = 0 then branch to 0 and check again (r3 = 0x00)*/
	bne	0b		                    
	str	r0, [r12]			/* Write back status to clear */



/******* Read SDRAM PD data	************************************************/


/*** Put out address, with READ mode ***/	

	/*  Set SDRAM module address and read mode */  
	ldr	r0, =SDRAM_DEVID				/* Load slave address for SDRAM module (0xA2) */
	orr	r1, r0, #IDBR_MODE				/* Set read bit (bit #0) */
	str	r1, [r13]						/* Store to data register */	

	/*  Send next read request */  
	ldr	r1, [r11]			  /* read the current Control Register value */ 
	orr	r1, r1,	#ICR_START	  /* Set start bit */
	orr	r1, r1, #ICR_TRANSFER /* Set transfer bit - bit is self_clearing */
	str	r1, [r11]			  /* Store to control register */


	/* Wait for transmit empty status */
	ldr	r1, =0x00000000		/* Initialize I2C timeout counter */
0:
	add	r1, r1, #1			/* Increment I2C timeout counter (r1 = r1 + 1) */
	cmp	r1, r9
	beq	i2c_error			/* Kick out of SDRAM initialization if timeout occurs */
	ldr	r0, [r12]			/* Load I2C Status Reg into R0 -  ld	(r12), r10*/
	ldr	r2, =ISR_EMPTY		/* Poll status register */
	and	r3, r2, r0			/* Bit #6 is checked: IDBR Transmit Empty */
	cmp	r3, r2				/* If bit = 0 then branch to 0 and check again (r3 = 0x00)*/
	bne	0b		                    
	str	r0, [r12]			/* Write back status to clear */


sdram_loop:
	add	r6, r6, #1			/* Increment byte counter */
	
/*** READ the next Byte!!! ***/	

	ldr	r1, [r11]			  /* read the current Control Register value */ 
	bic	r1, r1,	#ICR_START	  /* No start bit (already started) */
	orr	r1, r1, #ICR_TRANSFER /* Set transfer bit - bit is self_clearing */

	/* we have to set NACK before reading the last bit */
	cmp	r6, r7	               /* r7 = 64 (decimal) so if r6 = 64, this is the last byte to be read */
	bne	1f					   /* If bytes left, skip ahead */
	orr	r1, r1, #ICR_ACK	   /* Set NACK if this is the last byte */
	orr	r1, r1, #ICR_STOP      /* Set STOP if this is the last byte */
1:
	str	r1, [r11]			   /* Store to control register */



	/* Wait for read full status */
	ldr	r1, =0x00000000		/* Initialize I2C timeout counter */
0:
	add	r1, r1, #1			/* Increment I2C timeout counter (r1 = r1 + 1) */
	cmp	r1, r9
	beq	i2c_error			/* Kick out of SDRAM initialization if timeout occurs */
	ldr	r0, [r12]			/* Load I2C Status Reg into R0 */	
	ldr	r2, =ISR_FULL		/* Poll status register */
	and	r3, r2, r0			/* Bit #6 is checked: IDBR Transmit Empty */
	cmp	r3, r2				/* If bit = 0 then branch to 0 and check again */
	bne	0b		                    
	str	r0, [r12]			/* Write back status to clear */



	/* Read the data byte */
	ldr	r1, [r13] 			/* Read the byte */

	ldr	r2, =CHECKSUM_BYTE 
	cmp	r6, r2				/* is it the CHECKSUM byte??? */
	beq	1f
	add	r5, r5, r1			/* Add it to the checksum if not the checksum byte */
	bal	2f					/* skip checksum comparison */
1:
	ldr	r0, =0xff			/* If this is the checksum byte, compare it */
	and	r5, r5, r0			/*	against the calculated checksum         */
	cmp	r1, r5
	bne	bad_checksum		/* If no match, skip SDRAM controller initialization */


#if 0   /* NO PLACE TO STORE DRAM SIZE LIKE THERE WAS ON i960 (IDR)*/
    ldconst 0x1, g9				/* Indicate that checksum test passed */
	lda     DRAM_SIZE_LOC,g1
	st      r4,(g1)				/* Leave SDRAM size in internal data RAM  */
	mov     r4,g0				/* referenced as data_ram_ptr->dram_size in cyrn_hw.c */
		/* SDRAM size has been calculated and left in r4 prior to this checksum test (see in BANKSZ_BYTE check) */								
#endif


2:
	ldr	r2, =BANKCNT_BYTE	/* Check for bank count byte */
	cmp	r6, r2
	bne	1f
	mov	r8, r1				/* Store bank count */

1:
	ldr	r2, =BANKSZ_BYTE	/* Check for bank size byte */
	cmp	r6, r2
	bne	1f

	ldr	r2, =0x04			/* Store bank size in Mbytes (shift left 2 bits) */
	mul	r10, r1, r2			
	
	mul	r2, r8, r10			/* Multiply by bank count to get DRAM size in MB */
	ldr	r0, =0x100000
	mul	r4, r2, r0			/* Convert size to bytes  - r4 contains DRAM size in bytes */



#if 0 /* RIGHT NOW I DONT THINK I NEED TO LOOK FOR ECC CAPABILITY??? */
1:
	ldr	r2, =CONFIG_BYTE 	/* Check for module configuration byte */
	cmp	r6, r2
	bne	1f
	ldr	r2, =0x02			/* ECC capable configuration */
	cmp	r1, r2, 2f
	ldconst	1, g11				/* Module is ECC capable */					
	b	1f
2:
	ldconst 0, g11				/* Module is not ECC capable */	
#endif

1:
	/* Continue reading bytes if not done */
	cmp	r6, r7
	bne	sdram_loop
	
	b	i2c_disable


bad_checksum:
	HEX_DISPLAY r2, r3, DISPLAY_7, DISPLAY_7	

i2c_error:

	/* hit the leds if an error occurred */
	HEX_DISPLAY r2, r3, DISPLAY_5, DISPLAY_5


i2c_disable:
	/*  Disable I2C Interface Unit */  
	ldr	r1, [r11] 
	bic	r1, r1, #ICR_ENB			/* Disable I2C unit */
	bic	r1, r1, #ICR_SCLENB			/* Disable I2C clock generator */
	str	r1, [r11]					/* Store to control register */
	
	
	/* ADD THIS???: cmpobne	1, g9, test_init	Skip SDRAM controller initialization if checksum test failed */



/***************************************************************************
 * SDRAM setup
 */

	ldr	r9,  =MMR_BASE         /* get base of MMRs */

	ldr	r0, =DRAM_BASE		/* Program SDRAM Base Address register */
	str	r0, [r9, #SDBR_OFF]    


/* Set up bank 0 register */
CHECK_32MB:
	ldr	r1, =RAM_32MEG  /* do we have 32 MB banks? */
	cmp	r10, r1
	bne	CHECK_64MB

	ldr	r0, =SBR_32MEG	/* Program SDRAM Bank0 Boundary register to 32 MB*/
	b	SET_BANK1

CHECK_64MB:
	ldr	r1, =RAM_64MEG  /* do we have 64 MB banks? */
	cmp	r10, r1
	bne	CHECK_128MB

	ldr	r0, =SBR_64MEG	/* Program SDRAM Bank0 Boundary register to 64 MB*/
	b	SET_BANK1

CHECK_128MB:
	ldr	r1, =RAM_128MEG  /* do we have 128 MB banks? */
	cmp	r10, r1
	bne	CHECK_256MB

	ldr	r0, =SBR_128MEG	/* Program SDRAM Bank0 Boundary register to 128 MB*/
	b	SET_BANK1

CHECK_256MB:
	ldr	r1, =RAM_256MEG  /* do we have 256 MB banks? */
	cmp	r10, r1
	bne	dram_error

	ldr	r0, =SBR_256MEG	/* Program SDRAM Bank0 Boundary register to 64 MB*/
	b	SET_BANK1



SET_BANK1:

	str	r0, [r9, #SBR0_OFF]	/* store SBR0 */

	ldr	r2, =0x02			
	cmp	r2, r8				/* do we have 2 banks??? */
	bne	END_DRAM_SIZE

	add	r0, r0, r0			/* SDRAM Bank1 Boundary register is double SBR0 */
	str	r0, [r9, #SBR1_OFF]  

	ldr	r0, =SDRC_2BANK		/* set the drive strength for 2 bank device */
	str	r0, [r9, #SDRC_OFF]

END_DRAM_SIZE:

	ldr	r0, =SDRC_1BANK		/* set the drive strength for 1 bank device */
	str	r0, [r9, #SDRC_OFF]

	b	init_dram

dram_error:

  	HEX_DISPLAY r2, r3, DISPLAY_F, DISPLAY_F
	

init_dram:

	ldr	r0, =0			/* turn off refresh */
	str	r0, [r9, #RFR_OFF]
	
	ldr	r0,   =MRS_NO_OP        /* Issue NOP cmd to SDRAM */
	str	r0, [r9, #SDIR_OFF]
	DELAY_FOR 0x4000, r0
	
	ldr	r0, =MRS_PRECHRG	/* Issue 1 Precharge all */
	str	r0, [r9, #SDIR_OFF]    
	DELAY_FOR 0x4000, r0


	ldr	r0, =MRS_AUTO_RFRSH	/* Issue 1 Auto Refresh command */
	str	r0, [r9, #SDIR_OFF]    
	DELAY_FOR 0x4000, r0


	ldr	r0, =MRS_AUTO_RFRSH
	str	r0, [r9, #SDIR_OFF]    /* Auto Refresh #1 */
	str	r0, [r9, #SDIR_OFF]    /* Auto Refresh #2 */
	str	r0, [r9, #SDIR_OFF]    /* Auto Refresh #3 */
	str	r0, [r9, #SDIR_OFF]    /* Auto Refresh #4 */
	str	r0, [r9, #SDIR_OFF]    /* Auto Refresh #5 */
	str	r0, [r9, #SDIR_OFF]    /* Auto Refresh #6 */
	str	r0, [r9, #SDIR_OFF]    /* Auto Refresh #7 */
	str	r0, [r9, #SDIR_OFF]    /* Auto Refresh #8 */

	ldr	r0, =MRS_CAS_LAT_2	/* set the CAS latency */
	str	r0, [r9, #SDIR_OFF]
	DELAY_FOR 0x4000, r0
	
	ldr	r0, =MRS_NORM_OP	/* Issue a Normal Operation command */
	str	r0, [r9, #SDIR_OFF]     

	ldr	r0, =RFR_INIT_VAL	/* Program Refresh Rate register */
	str	r0, [r9, #RFR_OFF]   

 

/* ldr   r0, =(FLASH_BASE :AND: &FFFF0000) */
/* str   r0, [r9, #FEBR1_OFF]   ; Program Flash Bank1 Base Address register */
/* */
/* ldr   r0, =(FLASH_SIZE :AND: &FFFF0000) */
/* str   r0, [r9, #FBSR1_OFF]   ; Program Flash Bank1 Size register */
/* */
/* ldr   r0, =FWSR0_INIT_VAL */
/* str   r0, [r9, #FWSR0_OFF]   ; Program Flash Bank0 Wait State register */
/* */
/* ldr   r0, =FWSR1_INIT_VAL */
/* str   r0, [r9, #FWSR1_OFF]   ; Program Flash Bank1 Wait State register */

	HEX_DISPLAY r0, r1, DISPLAY_0, DISPLAY_2
/*        BOARD_BOOT_LED_DEBUG 2 */

/***************************************************************************
 * begin initializing the i80310
 */

	/* 
	 * Enable access to all coprocessor registers
	 */
	ldr	r0, =0x2001					/* enable access to all coprocessors */
	mcr	p15, 0, r0, c15, c1, 0
	
	mcr	p15, 0, r0, c7, c10, 4		/* drain the write & fill buffers */
	CPWAIT r0	
	
	mcr	p15, 0, r0, c7, c7, 0		/* flush Icache, Dcache and BTB */
	CPWAIT r0	
	
	mcr	p15, 0, r0, c8, c7, 0		/* flush instuction and data TLBs */
	CPWAIT r0	
	
	/*
	 * Enable the Icache
	 */
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, IMM(ARM_INSTRUCTION_CACHE_ENABLED)
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

	/*
	 * Set the TTB register
     */
	ldr	r0, =SYM_NAME(page1)
        mcr     ARM_CACHE_COPROCESSOR_NUM,          \
                ARM_COPROCESSOR_OPCODE_DONT_CARE,   \
                r0,                                 \
                ARM_TRANSLATION_TABLE_BASE_REGISTER,\
                ARM_COPROCESSOR_RM_DONT_CARE,       \
                ARM_COPROCESSOR_OPCODE_DONT_CARE

        /*
         * Enable permission checks in all domains
         */
	ldr	r0, =0x55555555
        mcr     ARM_CACHE_COPROCESSOR_NUM,          \
                ARM_COPROCESSOR_OPCODE_DONT_CARE,   \
                r0,                                 \
                ARM_DOMAIN_ACCESS_CONTROL_REGISTER, \
                ARM_COPROCESSOR_RM_DONT_CARE,       \
                ARM_COPROCESSOR_OPCODE_DONT_CARE
	
	/*
	 * Enable the MMU
	 */
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, IMM(ARM_MMU_ENABLED)
	orr	r0, r0, IMM(ARM_ACCESS_CHECKS_ROM)
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0
	
	mcr	p15, 0, r0, c7, c10, 4		/* drain the write & fill buffers */
	CPWAIT r0	
	
	/*
	 * Enable the Dcache
	 */
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, IMM(ARM_DATA_CACHE_ENABLED)
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

	/*
	 * Enable the BTB
	 */
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, IMM(ARM_BTB_ENABLED)
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

/* scrub/init SDRAM if enabled/present */
	ldr	r11, =RAM_ACTUAL_BASE	/* base address of SDRAM */
	mov	r12, r4					/* size of memory to scrub (from SPD code!) */
	mov	r8, r12					/* save DRAM size in R8 */
	mov	r0, #0					/* scrub with 0x0000:0000 */
	mov	r1, #0
	mov	r2, #0				
	mov	r3, #0
	mov	r4, #0					
	mov	r5, #0
	mov	r6, #0					
	mov	r7, #0
10: /*fastScrubLoop */
	subs	r12, r12, #SZ_32	/* 32 bytes per whack */
	stmia	r11!, {r0-r7}
	beq	15f
	b	10b
15:	
		
	/* save DRAM size at fixed memory location 0xa0100000 */
	ldr		r0, =0xa0100000
	str		r8, [r0]
	HEX_DISPLAY r0, r1, DISPLAY_1, DISPLAY_0
/*
 * clean/drain/flush the main Dcache
 */
	ldr	r1, =(RAM_ACTUAL_BASE+DRAM_SIZE) /* use a CACHEABLE area of */
										 /* the memory map above SDRAM */
	mov	r0, #SZ_1K						 /* number of lines in the Dcache */
20:
	mcr	p15, 0, r1, c7, c2, 5		/* allocate a Dcache line */
	add	r1, r1, #SZ_32				/* increment the address to */
									/* the next cache line */
	subs	r0, r0, #1				/* decrement the loop count */
	bne	20b

/*
 * clean/drain/flush the mini Dcache
 */
	ldr	r2, =(RAM_ACTUAL_BASE+DRAM_SIZE+DCACHE_SIZE) /* use a CACHEABLE area of */
						/* the memory map above SDRAM */
	mov	r0, #SZ_64			/* number of lines in the Dcache */
21:
	mcr	p15, 0, r2, c7, c2, 5		/* allocate a Dcache line */
	add	r2, r2, #SZ_32				/* increment the address to */
									/* the next cache line */
	subs	r0, r0, #1				/* decrement the loop count */
	bne	21b

	mcr	p15, 0, r0, c7, c6, 0		/* flush Dcache */
	CPWAIT r0
	
	mcr	p15, 0, r0, c7, c10, 4		/* drain the write & fill buffers */
	CPWAIT r0	
	

/* enable ECC stuff here */
	mcr p15, 0, r0, c7, c10, 4		/* drain write & fill buffers */
	CPWAIT r0

	mrc	p13, 0, r0, c0, c1, 0		/* BCU_WAIT --> wait until the BCU isn't busy */
	submi	pc, pc, #0xc
	
checkme:	/* add in multi-bit error reporting */
	mrc	p13, 0, r0, c0, c1, 0		/* disable ECC */
	and	r0, r0, #(-1-8)
	mcr	p13, 0, r0, c0, c1, 0		
	orr	r0, r0, #6			/* enable single-bit correction, multi-bit detection */
	mcr	p13, 0, r0, c0, c1, 0		
	orr	r0, r0, #8			/* enable ECC */
	mcr	p13, 0, r0, c0, c1, 0		

	mrc	p13, 0, r0, c0, c1, 0		/* BCU_WAIT --> wait until the BCU isn't busy */
	submi	pc, pc, #0xc

	HEX_DISPLAY r0, r1, DISPLAY_1, DISPLAY_1

/*        BOARD_BOOT_LED_DEBUG 3 */

/* moving on */		

    /*
     * Initialize the stack pointer
     */
    ldr     sp, =0xA00FFFF0
 
    /*
     * Tell c_start where the top of ram is 
	 */
    mov     r0, sp

	/*
	 * Zero frame pointer to mark end of chain
	 */
	mov	r11,IMM(0)

	HEX_DISPLAY r2, r3, DISPLAY_2, DISPLAY_0 

	TRIGGER_LA_ON_ADDRESS 0xfedc0000, r2, r3
		
    /*
     * Return from _bsp_init_stack
	 *   At this point, 
	 *   r8 contains the return address,
	 *   r0 contains the address of the top of RAM,
	 *   sp " "
	 *   r11 contains the frame pointer
     */
     mov     pc, lr

FUNC_END _bsp_init_stack

	



/**** Cyclone-added assembly routines for diagnostics ****/
	
	.text
    .align     4

/* Switch the MMU over from using ROM-based PT entries to 
   RAM-based PT entries - this will allow us to erase/reprogram
   the Flash ROM without regard to when the MMU needs to
   perform a PT walk */
FUNC_START _switchMMUpageTables
#if 0
	/* Disable the MMU, change the TTB value, and re-enable the MMU */

	/*
	 * Disable the MMU
	 */
	mrc		p15, 0, r0, c1, c0, 0
	bic		r0, r0, #0x01			/* clear the M bit (bit 0) */
	mcr		p15, 0, r0, c1, c0, 0
	CPWAIT  r0

	/*
	 * Set the TTB register
     */
	ldr	r0, =SYM_NAME(ram_page1)
        mcr     ARM_CACHE_COPROCESSOR_NUM,          \
                ARM_COPROCESSOR_OPCODE_DONT_CARE,   \
                r0,                                 \
                ARM_TRANSLATION_TABLE_BASE_REGISTER,\
                ARM_COPROCESSOR_RM_DONT_CARE,       \
                ARM_COPROCESSOR_OPCODE_DONT_CARE

	/*
	 * Re-enable the MMU
	 */
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, IMM(ARM_MMU_ENABLED)
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0
#endif
	mov     pc, lr;	/* return */

FUNC_END _switchMMUpageTables


	.text
        .align     4

FUNC_START _enableDCache
	mcr p15, 0, r0, c7, c10, 4;
	mrc p15, 0, r0, c1, c0, 0;
	orr r0, r0, #4;
	mcr p15, 0, r0, c1, c0, 0;
	
	mov     pc, lr;	/* return */
FUNC_END _enableDCache
 

	.text
        .align     4

FUNC_START _disableDCache
	mcr p15, 0, r0, c7, c10, 4;
	mrc p15, 0, r0, c1, c0, 0;
	ldr	r1, =0xfffffffb;
	and r0, r1, r0;
	mcr p15, 0, r0, c1, c0, 0;

	mov     pc, lr; /* return */
FUNC_END _disableDCache

	.ltorg

	.sect .ram_func
	.align     4

FUNC_START _disableICache
	mrc		p15, 0, r0, c1, c0, 0;
	bic		r0, r0, #0x1000			/* clear the I bit (bit 12) */
	mcr p15, 0, r0, c1, c0, 0;

	mov     pc, lr; /* return */
FUNC_END _disableICache


	.sect .ram_func
	.align     4

FUNC_START _enableICache
	mrc p15, 0, r0, c1, c0, 0;
	ldr	r1, =0x1000;
	orr r0, r0, r1;
	mcr p15, 0, r0, c1, c0, 0;
	mov     pc, lr;	/* return */
FUNC_END _enableICache

 
	.sect .ram_func
	.align     4

FUNC_START _flushICache
	mcr p15, 0, r0, c7, c5, 0;		/* invalidate the I cache and BTB */
	mcr p15, 0, r0, c8, c5, 0;		/* invalidate the I cache TLB */

	mov     pc, lr;	/* return */
FUNC_END _flushICache


	.text
        .align     4

FUNC_START _enableFiqIrq
	/* make sure we can write to CP13 */
	mrc p15, 0, r0, c13, c0, 1;
	orr r0, r0, #0x2000
	mrc p15, 0, r0, c13, c0, 1;

	mrc p13, 0, r0, c0, c0, 0;
	orr		r0, r0, #3;  /* enable FIQ and IRQ */
	mcr	p13, 0, r0, c0, c0, 0;
	mov     pc, lr;
FUNC_END _enableFiqIrq



FUNC_START _enable_timer
	ldr r1, =0x00000005	/* reset counter, enable timer */
	mcr p14, 0, r1, c0, c0, 0 
	mov     pc, lr
FUNC_END _enable_timer


FUNC_START _disable_timer
	ldr r1, =0x00000000	/* reset counter, enable timer */
	mcr p14, 0, r1, c0, c0, 0 
	mov     pc, lr
FUNC_END _disable_timer



/* Function to delay 1 microsecond */
FUNC_START _usec_delay
	ldr	r2, =0x258	/* 1 microsec = 600 clocks (600 MHz CPU core) */
	
usdelay_loop:
	mrc p14, 0, r0, c1, c0, 0	/*read CCNT into r0 */
	cmp r2, r0	/* compare the current count */
	bpl	usdelay_loop /* stay in loop until count is greater */

	
	mrc p14, 0, r1, c0, c0, 0
	orr	r1, r1, #4	/* clear the timer */
	mcr p14, 0, r1, c0, c0, 0 

	mov     pc, lr
FUNC_END _usec_delay


/* Function to delay 1 millisecond */
FUNC_START _msec_delay
	ldr	r2, =0x927c0	/* 1 millisec = 600,000 clocks (600 MHz CPU core) */
	
msdelay_loop:
	mrc p14, 0, r0, c1, c0, 0	/*read CCNT into r0 */
	cmp r2, r0	/* compare the current count */
	bpl	msdelay_loop /* stay in loop until count is greater */

	
	mrc p14, 0, r1, c0, c0, 0
	orr	r1, r1, #4	/* clear the timer */
	mcr p14, 0, r1, c0, c0, 0 

	mov     pc, lr
FUNC_END _msec_delay


FUNC_START _read_timer
/* R0 will return timer value */
	mrc		p14, 0, r0, c1, c0, 0	/*read CCNT into r0 */

	mov     pc, lr
FUNC_END _read_timer


FUNC_START _read_intctl
	mrc		p13, 0, r0, c0, c0, 0

	mov     pc, lr
FUNC_END _read_intctl


FUNC_START _read_intstr
	mrc		p13, 0, r0, c4, c0, 0

	mov     pc, lr
FUNC_END _read_intstr


FUNC_START _read_cpsr
	mrs		r0, cpsr

	mov     pc, lr
FUNC_END _read_cpsr


FUNC_START _cspr_enable_fiq_int
	mrs		r0, cpsr
	bic		r0, r0, #0x40
	msr		cpsr, r0

	mov     pc, lr
FUNC_END _cspr_enable_fiq_int


FUNC_START _cspr_enable_irq_int
	mrs		r0, cpsr
	bic		r0, r0, #0x80
	msr		cpsr, r0

	mov     pc, lr
FUNC_END _cspr_enable_irq_int



