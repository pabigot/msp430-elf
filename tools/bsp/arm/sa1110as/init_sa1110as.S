/*
 * init_sa1110as.S -- Initialization for 
 *                    Intel(R) SA-1100 Multimedia Development Board
 *
 * Copyright (c) 1998, 1999, 2000 Cygnus Support
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
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */
	.file "init_sa1110as.S"
        .title "Init Code for Intel(R) SA-1100 Multimedia Development Board"

#include <bsp/cpu.h>
#include <bsp/sa-1100.h>
#include <bsp/sa1100.inc>
#include <bsp/sa1110as.h>
#include <bsp/mmumacro.s>

	.text

#define ALL_LEDS_OFF           0xF

#define MMU_ENABLE_VALUE       (ARM_MMU_ENABLED                                | \
                                ARM_ADDRESS_FAULT_DISABLED                     | \
                                ARM_DATA_CACHE_ENABLED                         | \
                                ARM_WRITE_BUFFER_ENABLED                       | \
                                ARM_LITTLE_ENDIAN                              | \
                                ARM_ACCESS_CHECKS_NONE                         | \
                                ARM_INSTRUCTION_CACHE_ENABLED                  | \
                                ARM_VIRTUAL_IVR_BASE_00000000)


//
// Assembler logical name header file for SA-1110 Handheld Development Board
// 

//  IF ALLOCATE_TABLE = "FALSE"
#define MDCAS00_VAL				0xaaaaaa7f		//bring up 0xaaaaaa9f	// perf 0xaaaaaa7f //Gds changed back 9->7
#define MDCAS01_VAL				0xaaaaaaaa
#define MDCAS02_VAL				0xaaaaaaaa

//These are Zilker setting
//MDCAS20_VAL				0x83c1e01f		
//MDCAS21_VAL				0x3c1e0f07
//MDCAS22_VAL				0xfffff078

// //note: We need a clock source for the audio clock generater.  Since there is no 
// //note: memory in bank 2, we'll use the bank2 sdram clock as input to the audio clock 
// //note: generator for UDA1341.  So the UDA1341 audio driver is modifying the bank2 settings.
// //note: we are doing this to save the cost and power of a seperate occilator.
// //note: WARNING: if you are using bank 2 memory, this will turn on/off that bank of memory


//SEP make the Assabet bank settings for 2 & 3 same as bank 0 & 1
#define MDCAS20_VAL		0xaaaaaa7f		//bring up 0xaaaaaa9f	// perf 0xaaaaaa7f //Gds changed back 9->7
#define MDCAS21_VAL		0xaaaaaaaa
#define MDCAS22_VAL		0xaaaaaaaa

// MSC0_VAL  			0x6d696d69	
#define MSC0_VAL		0x4b384b38		// gds replaced from above value
// MSC1_VAL			0x43296729
#define MSC1_VAL		0x22212419		// gds replaced from above value
//MSC2_VAL			0x4329FFF9 		// dead slow
#define MSC2_VAL		0x42194449 		// gds replaced from above value

//MECR_VAL			0x7FFF7FFF		// dead slow //0x10841084 sidearm/sidekick value
//MECR_VAL			0x10841084
#define MECR_VAL		0x994a994a		// gds from 7f value
#define MDREFR_VAL		0x49fc0327		// Mike says this is the correct 1/2 speed number
//MDREFR_VAL			0x49bc0327		// Gregg Mack's updated number gds from above value
#define SMCNFG_VAL		0xafccafcc

#define UNENABLED_MDCNFG_VAL	0x72547254		//new value as per Mike	// 0xf3507254	// bringup // perf 0xf4507254
//ENABLED_MDCNFG_VAL		0xf4537257	// 0xf3533157 bring up // perf 0xf4537257
//ENABLED_MDCNFG_VAL		0xf3537255	// 0xf4537257 GDS new bringup  val from R. Dorris
#define ENABLED_MDCNFG_VAL	0x72547255	//SEP: setup bits "according to Mike S"

#define GPIO_2_T0_9		0x3FC

// IF _FLASH = "1"
//#define DRAM_MAPPED		32		
//#define FLASHRAM_MAPPED	32
// ELSE
#define DRAM_MAPPED		16		
#define FLASHRAM_MAPPED		16
// ENDIF
#define IMG_COUNTER		0x01000000

//
// Assabet Register
//
#define BCR_PHYSICAL		0x12000000
#define CODEC_RST		0x4
#define BCR_SETUPVAL1		0x00a014e4            //GDS
#define BCR_SETUPVAL2		0x00a044e4

//
// Zilker register
//
#define MISCWR_PHYSICAL		0x10000030
#define WHOAMI_PHYSICAL		0x10000000
#define WHOAMI_VAL_SA1111	0x00000001

#define LED_BLANK_PHYSICAL	(SA1110MM_REG_BASE_PHYSICAL+0x50)
#define LEDBANK_PHYSICAL	(SA1110MM_REG_BASE_PHYSICAL+0x10)
#define LEDBANK_VIRTUAL		(SA1110MM_REG_BASE_U_VIRTUAL+0x10)

//bits
#define CDC_RESET		0x00000001
#define LCD_DISP		0x00000100

//MECR values 7FFF7FFF is slowest, 10841084 medium, 194a194a Gregg Mack, 11FF7FE8 in ODO

#define TUCR_VAL 		0x20000400

// PPCR values 0=59mhz 3=103.2mhz,6=147Mhz, 7=162.2mhz, 10=206.4mhz 
//PPCR_CLOCK				0x0000000a
// set clock to 147Mhz for Slinky/Neponset      
#define PPCR_CLOCK		0x00000006
   	
#define GAFR_SKAF		0x08600000
#define GPDR_SKAF		0x0827A7FC 		//0x08200000 SideARM

#define EbootOffset		0x00078000
#define StackOffset		0x00068000

#define DRAM_BANK1_PHYSICAL	0xc0000000

//
// SA-1100 MM Board Registers 
//

#define CACHED_2_UNCACHED_OFFSET	0x20000000

#define SA1110MM_REG_BASE_PHYSICAL	0x12000000	// Assabet BCR
#define SA1110MM_REG_BASE_C_VIRTUAL	0x8B900000	// 
#define SA1110MM_REG_BASE_U_VIRTUAL	(SA1110MM_REG_BASE_C_VIRTUAL+CACHED_2_UNCACHED_OFFSET)

//-----------------------------------------------------------------
        .align     4
FUNC_START _bsp_init_stack
        /*
         * Initialize the Discrete LED's to off
         */
        BothLedsOff r0,r1

	ldr		r0, =GPCR_PHYSICAL
	ldr		r1, =0xFFFFFFFF				//  clear all output (esp MBREQ) before the magic incantation
	str		r1, [r0]

	ldr	r0, =GPDR_PHYSICAL
//	ldr	r1, =0x0C2277FC			//  Pat Carr comment: ZILKER requires this value in GPDR
	ldr	r1, =0x0807A7FC			//  for now get rid of MBGRNT, STEREO RXD, GDS changed for assabet
	str	r1, [r0]
	ldr	r0, =GPSR_PHYSICAL		//  to find out SA-1110 HH board config must write FF to GPIO 2:9
	ldr	r1, =GPIO_2_T0_9		//  writing 0 has no effect so can blast the 1s in
	str	r1, [r0]				 
	ldr	r0, =GPDR_PHYSICAL		//  now config pin 2 through 9 as input
	ldr	r1, =0x0807A400	
	str	r1, [r0]	

		
	ldr	r0, =GPLR_PHYSICAL		//  now read pins to determine the configuration
	mov 	r2,#0x64
20:	 	
	ldr	r1, [r0]
        subs  	r2,r2,#1			// loop 100x through to make sure the pin bleed down takes
	bne 	20b
							//
	and	r1, r1, #GPIO_2_T0_9			//  mask off non-config pins
//	ldr	r0, =DRIVER_GLOBALS_CONFIG_PHYSICAL	
//	str	r1, [r0] 				//  store configuration in driver globals
        ldr	r0, =GPDR_PHYSICAL			//  restore correct pin direction
	ldr	r1, =0x0807A7FC				//  GDS changed for assabet
	str	r1, [r0]	


        mcr     p15, 0, r0, c15, c2, 2          // disable clock switching

 	ldr 	r0, =PPCR_PHYSICAL 
	ldr 	r1, =PPCR_CLOCK			// Defined in SA-1100MM.inc (default is 0xA = 220Mhz)  
	str 	r1, [r0]	
    	mcr     p15, 0, r0, c15, c1, 2          // turn on clock switching
    	nop
    	nop
InitMemoryController:

 	
	ldr 	r0,=MDCAS00_PHYSICAL
	ldr 	r1,=MDCAS00_VAL			// values determined at build time by IFDEF in SA1100MM.inc (supports 16 and 32 meg)
	STR 	r1,[r0]

	ldr 	r0,=MDCAS01_PHYSICAL
	ldr 	r1,=MDCAS01_VAL			// values determined at build time by IFDEF in SA1100MM.inc (supports 16 and 32 meg)
	STR 	r1,[r0]

	ldr 	r0,=MDCAS02_PHYSICAL
	ldr 	r1,=MDCAS02_VAL			// values determined at build time by IFDEF in SA1100MM.inc (supports 16 and 32 meg)
	STR 	r1,[r0]

	ldr 	r0,=MSC0_PHYSICAL
	ldr 	r1,=MSC0_VAL			// defined in SA1100MM.inc
	STR 	r1,[r0]

	ldr 	r0,=MSC1_PHYSICAL
	ldr 	r1,=MSC1_VAL			// defined in SA1100MM.inc
	STR 	r1,[r0]
 
	ldr 	r0,=MECR_PHYSICAL
	ldr 	r1,=MECR_VAL			// defined in SA1100MM.h
	STR 	r1,[r0]

 	ldr 	r0,=MDRERF_PHYSICAL
 	ldr 	r1,=MDREFR_VAL		// orr in bit22 to slow down SDRAM for Video/LCD defined in SA1100MM.inc
 	STR 	r1,[r0]

	ldr 	r0,=MDCAS20_PHYSICAL
	ldr 	r1,=MDCAS20_VAL			// values determined at build time by IFDEF in SA1100MM.inc (supports 16 and 32 meg)
	STR 	r1,[r0]

	ldr 	r0,=MDCAS21_PHYSICAL
	ldr 	r1,=MDCAS21_VAL			// values determined at build time by IFDEF in SA1100MM.inc (supports 16 and 32 meg)
	STR 	r1,[r0]

	ldr 	r0,=MDCAS22_PHYSICAL
	ldr 	r1,=MDCAS22_VAL			// values determined at build time by IFDEF in SA1100MM.inc (supports 16 and 32 meg)
	STR 	r1,[r0]

	ldr 	r0,=MSC2_PHYSICAL
	ldr 	r1,=MSC2_VAL			// defined in SA1100MM.inc
	STR 	r1,[r0]
 
	ldr 	r0,=SMCNFG_PHYSICAL
	ldr 	r1,=SMCNFG_VAL			// defined in SA1100MM.inc
	STR 	r1,[r0]

	ldr 	r0,=MDCNFG_PHYSICAL
	ldr 	r1,=UNENABLED_MDCNFG_VAL	// turn on the DRAM
	STR 	r1,[r0]

	ldr 	r0,=PSSR_PHYSICAL
	ldr 	r1,=PSSR_DRH			// clear the DRAM Hold bit, do peripherals later
	STR 	r1,[r0] 

	//  Perform 8 reads from unenabled DRAM
	ldr     r1, =DRAM_BANK1_PHYSICAL
	ldr     r3, [r1]
	ldr     r3, [r1]
	ldr     r3, [r1]
	ldr     r3, [r1]
	ldr     r3, [r1]
	ldr     r3, [r1]
	ldr     r3, [r1]
	ldr     r3, [r1]

	ldr 	r0,=MDCNFG_PHYSICAL
	ldr 	r1,=ENABLED_MDCNFG_VAL		// turn on the DRAM
	STR 	r1,[r0]

Handle_MMU:

	//ldr	r0, =MDRERF_PHYSICAL		// start SDRAM autoclocking
	//ldr	r1, [r0]
	//orr	r1, r1, #0x30000000		// set KAPD, EAPD
	//str	r1, [r0]

	ldr 	r0, =0x00000000			// 
	mcr     p15, 0, r0, c1, c0              // MMU off 
	mcr     p15, 0, r0, c8, c7, 0           // flush tb's
	mcr     p15, 0, r0, c7, c7, 0           // flush caches

	mcr     p15, 0, r0, c15, c1, 2      // enable clock switching
	nop
	nop

	ldr 	r0,=PSSR_PHYSICAL
	ldr 	r1,=PSSR_PRH			// clear the peripheral hold bits
	STR 	r1,[r0]

	ldr	r0, =DRAM_BANK1_PHYSICAL	
        add	sp, r0, #0x61000		// temp stack for initialization 

	//bl	InitHexLeds

	MOV	r0, #0xd0     
	//BL	NSET_LEDS

 	ldr 	r0, =ICCR_PHYSICAL 
	ldr 	r1, =0x00000000			// All interrupts bring the SA-1100 out of idle
	str 	r1, [r0]	

 	ldr 	r0, =ICMR_PHYSICAL
	ldr 	r1, =0x00000000			// Disable all SA-1100 ints  
	str 	r1, [r0]		

 	ldr 	r0, =LCCR0_PHYSICAL
	ldr 	r1, =0x00000000			// Turn off the LCD 
	str 	r1, [r0]		
           
	MOV	r0, #0xd1     
	//BL	NSET_LEDS
       
// 	ldr     r0,=MISCWR_PHYSICAL		// Zilk misc write register
//	ldr     r1,=CDC_RESET			// 
//	orr     r1,r1,#9			// unreset the SA-1101 and ucb1200 chips
//	str     r1,[r0] 			// unreset the SA-1111 

	ldr     r0,=BCR_PHYSICAL			// Assabet Board Control Register
	ldr	r1,=BCR_SETUPVAL1		// init val bot Leds on force RS232 tranceiver
	str	r1,[r0]

	// Need to figure out how to configure the board	
//        ldr	r0, =DRIVER_GLOBALS_CONFIG_PHYSICAL	//  get the config value we stored earlier
//	ldr	r1, [r0]
//	ands	r2, r1, #BIT9				// Bit 9 indicates there is no Daughter card
//	bne	NoGfxOrDC
	b	NoGfxOrDC

	ldr     r0,=BCR_PHYSICAL		// Assabet Board Control Register
	ldr	r1,=BCR_SETUPVAL2		// turn green led we've got a SA-1111 DC, turn off 
	str	r1,[r0]					// SA-1110 bd RS232 tranceiver it will fight with the SA-1111BD's 

	ldr     r0,=GAFR_PHYSICAL			// GPIO Alternate Function register
	mov	r1,#GAFR_MBREQMBGNT|GAFR_DCCLOCK	// 
	str     r1,[r0]

	ldr     r0,=GPDR_PHYSICAL
	ldr     r1,=GPDR_SKAF			// defined in SA1100MM.h :OR:GAFR_LDD:OR:GAFR_UART_TXD
	str     r1,[r0]

	ldr     r0,=PPDR_PHYSICAL
	ldr     r1,=GPDR_SKAF			// defined in SA1100MM.h :OR:GAFR_LDD:OR:GAFR_UART_TXD
	str     r1,[r0]

	ldr     r0,=TUCR_PHYSICAL		// set the oscillator out to the SA-1101
	ldr     r1,=TUCR_VAL			// defined in SA1100MM.inc
	str     r1,[r0]

//	ldr	r0,=SKCR_PHYSICAL		// Turn on the PLL
//        ldr     r1,=SKCR_PLL			// defined in SA1101.inc
	str     r1,[r0]

	mov 	r0,#0x80000
70:	subs  	r0,r0,#1			// wait for PLL to settle
	bne 	70b

//	ldr	r0,=SKCR_PHYSICAL		// turn on the RCLOCK
//        ldr     r1,=SKCR_RCLK			// defined in SA1101.inc
	str     r1,[r0]
        
//	ldr	r0,=SKSMCR_PHYSICAL		// initialize the SMC (debug SA-1111 reset)
//        ldr     r1,=0x35			// defined in SA1101.inc
	str     r1,[r0]       
            
//	ldr	r0,=SKPCMCIA_PCCR_PHYSICAL	// initialize the SMC (debug SA-1111 reset)
        ldr     r1,=0x0				// defined in SA1101.inc
	str     r1,[r0]

NoGfxOrDC:

        // initialize the tick counters and 1hz clock
        mov     r0, #SCM_BASE                // base addr of scm module
        mov     r1, #0                       // get initial value for counters
        str     r1, [r0, #OSCR]             // initialize os timer counter
        orr     r0, r0,  #RTC_OFFSET        // generate addr to RTC base
        str     r1, [r0, #RCNR]             // initialize rtc counter

        mov     r1, #0x7F00
        orr     r1, r1,  #0xFF              // generate 0x7FFF bit pattern
        str     r1, [r0, #RTTR]             // divide RTC to get 1hz output

        PauseInMsecs    #700, r1,  r5,  r6

        //BOARD_BOOT_LED_DEBUG 2
        RedLedOn r0,r1
	  GreenLedOff r0,r1

        PauseInMsecs    #700, r1,  r5,  r6

        /*
         * Save the Return Address
         */
        mov     r8, lr

        /*
         * Setup a temporary stack pointer pointing somewhere
         * in the unmapped RAM address space.
         */
        ldr     sp, =(RAM_ACTUAL_BASE + SZ_512K)

        /*
         * Setup the Page Tables
         */
        bl      SYM_NAME(_bsp_mmu_init)

        /*
         * Setup the real stack pointer pointing at the 
         * top of the remapped RAM address -- returned
         * in r0 by _bsp_mmu_init.
         */
        mov     sp, r0

        //BOARD_BOOT_LED_DEBUG 3
        RedLedOn r0,r1
	  GreenLedOn r0,r1
        PauseInMsecs    #700, r1,  r5,  r6

        /*
         * Absolute address of 0 in r1.
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
        /*
         * Tell c_start where the top of ram is
         */
        //BOARD_BOOT_LED_DEBUG 4
        RedLedOff r0,r1
	  GreenLedOff r0,r1

        mov     r0, sp

        /*
         * Return
         */
        mov     pc, r8
FUNC_END _bsp_init_stack
