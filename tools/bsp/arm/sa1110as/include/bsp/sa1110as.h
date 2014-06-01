/*
 * sa1110as.h -- Header for Intel(R) SA-1110 Assabet Board
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
 * Other Brands and Trademarks are the property of their respective owners.
 */
#ifndef __SA1110AS_H__
#define __SA1110AS_H__ 1

#include <bsp/bsp.h>
#include <bsp/sa-1100.h>
#include <bsp/cpu.h>
#include <bsp/defs.h>

/*
 * ROM Definitions
 */
#define ROM_ACTUAL_BASE     SA1100_ROM_BANK0_BASE
#define ROM_TOTAL_SIZE      SZ_128K
#define ROM_VIRTUAL_BASE    0x50000000

/*
 * RAM Definitions
 */
#define RAM_ACTUAL_BASE     SA1100_RAM_BANK0_BASE
#define RAM_TOTAL_SIZE      SZ_32M
#define RAM_VIRTUAL_BASE    0x00000000

/*
 * Flash Definitions
 */
#define FLASH_ACTUAL_BASE   0x08000000
#define FLASH_TOTAL_SIZE    SZ_4M
#define FLASH_VIRTUAL_BASE  0x08000000

/*
 * SA1100 Multimedia Board Register definitions
 */
#define SA1100MM_XBUS_CONTROL_REGISTER          REG32_PTR(0x18400000)
#define SA1100MM_SYSTEM_RESET_REGISTER          REG32_PTR(0x18800000)
#define SA1100MM_VIDEO_INPUT_MODE_REGISTER      REG32_PTR(0x18800002)
#define SA1100MM_KEYPAD_IO_REGISTER             REG32_PTR(0x18800004)
#define SA1100MM_DISCRETE_LED_REGISTER          REG32_PTR(0x18800006)
#define SA1100MM_HEX_LED_REGISTER               REG32_PTR(0x18C00000)
#define SA1100MM_SWITCH_REGISTER                REG32_PTR(0x18C00002)
#define SA1100MM_XBUS_INTERRUPT_REASON_REGISTER REG32_PTR(0x18C00004)
#define SA1100MM_REVISION_REGISTER              REG32_PTR(0x18C00006)

/*
 * Hex LED Register Bit Field Definitions
 */
#define SA1100MM_HEX_DATA_MASK                  0x0F
#define SA1100MM_HEX_LED_0_STROBE               0x10
#define SA1100MM_HEX_LED_1_STROBE               0x20
#define SA1100MM_HEX_LED_BOTH_STROBES           (SA1100MM_HEX_LED_0_STROBE | \
                                                 SA1100MM_HEX_LED_1_STROBE)
#define SA1100MM_HEX_VID_CAP_ENABLE             0x40
#define SA1100MM_HEX_ENABLE_INTERRUPTS          0x80

#define GAFR_DCCLOCK		0x08000000
#define GAFR_MBREQMBGNT		0x00600000


#define PLAT_BCR       0x12000000
#define BCR_SETTINGS   0x00A034E4

#ifdef __ASSEMBLER__
/*
 * Use the following code for debug by flashing some LED's
 */
.macro PORT_TOGGLE_DEBUG
        /*
         * Leave the following values in registers:
         *
         *    r0 = BCR Register
         *    r1 = 
         *    r2 = LED_ON value
         *    r3 = LED_OFF value
         *    r4 = LED_DELAY value
         */
        ldr     r0, =PLAT_BCR
        ldr     r3, =BCR_SETTINGS
        bic     r2, r3, #0x2000     // clear bit to turn on LED
        ldr     r4, =0x7FFFFF
0:
        /*
         * Turn the LED's on
         */
  	str	r2, [r0]

        /*
         * Delay a while
         */
        mov     r5, r4
1:      subs    r5, r5, IMM(1)
        bne     1b

        /*
         * Turn the LED's off
         */
  	str	r3, [r0]

        /*
         * Delay a while
         */
        mov     r5, r4
1:      subs    r5, r5, IMM(1)
        bne     1b

        /*
         * Repeat
         */
        b       0b
.endm
#else
#define PORT_TOGGLE_DEBUG()    asm volatile ("
        /*
         * Leave the following values in registers:
         *
         *    r0 = Discrete LED Register
         *    r1 = Keypad IO Register
         *    r2 = LED_ON value
         *    r3 = LED_OFF value
         *    r4 = LED_DELAY value
         */
        ldr     r0, =0x12000000
        ldr     r3, =0x00A034E4
        bic     r2, r3, #0x2000
        ldr     r4, =0x7FFFFF
0:
        /*
         * Turn the LED's on
         */
  	strh	r2, [r0]

        /*
         * Delay a while
         */
        mov     r5, r4
1:      subs    r5, r5, #1
        bne     1b

        /*
         * Turn the LED's off
         */
  	strh	r3, [r0]

        /*
         * Delay a while
         */
        mov     r5, r4
1:      subs    r5, r5, #1
        bne     1b

        /*
         * Repeat
         */
        b       0b");
#endif /* __ASSEMBLER__ */

#ifdef __ASSEMBLER__

.macro BOARD_BOOT_LED_DEBUG num
//           ldr r0, =SA1100MM_HEX_LED_REGISTER
//
//            ldr r1, =(\num | SA1100MM_HEX_LED_BOTH_STROBES)
//            str r1, [r0]
//            
//            ldr r1, =(\num & ~SA1100MM_HEX_LED_0_STROBE)
//            str r1, [r0]
//
//            ldr r1, =(\num | SA1100MM_HEX_LED_BOTH_STROBES)
//            str r1, [r0]
.endm
#else
extern void flash_led(int, int);
#define BOARD_BOOT_LED_DEBUG(num) flash_led((num), 8)
#endif /* __ASSEMBLER__ */

//----------------------------------------------------------------------------
#ifdef __ASSEMBLER__
/*
 * Use the following code for debug by flashing some LED's
 */

.macro RedLedOn reg1, reg2
          // turn on the red LED

          ldr      \reg1, =BCR_SETTINGS
          bic      \reg1, \reg1, #0x2000     // clear bit to turn on LED
          mov      \reg2, #PLAT_BCR
          str      \reg1, [\reg2]            // write update to BCR reg
.endm

//----------------------------------------------------------------------------

.macro  RedLedOff reg1, reg2
          // turn off the red LED

          ldr      \reg1, =BCR_SETTINGS
          orr      \reg1, \reg1, #0x2000     // set bit to turn off LED
          mov      \reg2, #PLAT_BCR
          str      \reg1, [\reg2]            // write update to BCR reg

.endm

//----------------------------------------------------------------------------

.macro   GreenLedOn reg1, reg2
          // turn on the green LED

          ldr      \reg1, =BCR_SETTINGS
          bic      \reg1, \reg1, #0x4000     // clear bit to turn on LED
          mov      \reg2, #PLAT_BCR
          str      \reg1, [\reg2]            // write update to BCR reg

.endm

//----------------------------------------------------------------------------

.macro  GreenLedOff reg1, reg2
          // turn off the green LED

          ldr      \reg1, =BCR_SETTINGS
          orr      \reg1, \reg1, #0x4000     // set bit to turn off LED
          mov      \reg2, #PLAT_BCR
          str      \reg1, [\reg2]            // write update to BCR reg

.endm

//----------------------------------------------------------------------------

.macro  BothLedsOff reg1, reg2
          // turn off the green LED

          ldr      \reg1, =BCR_SETTINGS
          orr      \reg1, \reg1, #0x6000     // set bit to turn off LED
          mov      \reg2, #PLAT_BCR
          str      \reg1, [\reg2]            // write update to BCR reg

.endm


//----------------------------------------------------------------------------

.macro    hexLedAll reg1, reg2
          // apply the data in reg1 to LED bits 0-31

          mov     \reg2,  #PlatformRegBase     // get Platform reg base addr
          str     \reg1,  [\reg2, #PLAT_LED]    // update register
.endm

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
// .macro         PauseInSeconds  $sec, $w1, $w2, $w3
//          // delay for $sec seconds
//    
//          mov     \$w1,  #SCM_BASE          // base addr of scm module
//          orr     \$w1,  \$w1,  #RTC_OFFSET  // generate addr to RTC base
//
//          ldr     \$w2,  [\$w1, #RCNR]       // get start tick count
//          add     \$w3,  \$w2,  \$sec         // compute target tick count
//
//1:        ldr     \$w2,  [\$w1, #RCNR]       // get current tick count
//          cmp     \$w3,  \$w2                // test target against current
//          bhi     1b                      // reached target?
//          .endm 
                
//----------------------------------------------------------------------------

 .macro         PauseInMsecs $mls, $w1, $w2, $w3
          // delay for $mls milliseconds

          mov     \$w1,  #SCM_BASE        // base addr of scm module

          mov     \$w2,  #0xE00
          orr     \$w2,  \$w2,  #0x66      // generate ticks-per-millisec (0xE66)
          
          mov     \$w3,  \$mls
          mul     \$w2,  \$w3,  \$w2        // number of ticks in the desired interval    

          ldr     \$w3,  [\$w1, #OSCR]     // get start  tick count
          add     \$w3,  \$w3,  \$w2        // compute target tick count

1:        ldr     \$w2,  [\$w1, #OSCR]     // get current tick count
          cmp     \$w3,  \$w2              // test target against current
          bhi     1b                    // reached target?
          .endm 
                
//----------------------------------------------------------------------------

        // ------------------------------------------------------------------
        // macro to set the LEDs to the 32 bit value given in $w1

.macro        NSET_LEDS       $w1, $w2, $w3

        // Write to 32 bit LED display
        LDR     \$w2,  =PlatformRegBase + PLAT_LED     // get platform Base reg addr
	STR     \$w1,  [\$w2]            // display 1st byte
        MOV	\$w3, \$w1, LSR #8
	STR     \$w3,  [\$w2, #+4]       // display 2nd byte
        MOV	\$w3, \$w1, LSR #16
	STR     \$w3,  [\$w2, #+8]       // display 3rd byte        
	MOV	\$w3, \$w1, LSR #24
	STR     \$w3,  [\$w2, #+12]       // display 4th byte
        .endm


        // ------------------------------------------------------------------
        // Angel and uHAL macro to setup GPIOs on powerup.

        // Bit   Function
        // ---   ---------------
        //  0    switch S17
        //  1    switch S16
        // 17    red led 

.macro  INIT_GPIOS      $w1, $w2
        //// set up the direction of the GPIO pins
        ldr     \$w1, =GPIOREGBASE           // base address of GPIO ctl regs
        mvn     \$w2, #0 
        str     \$w2, [\$w1, #GPCR]           // drive outputs low before setting direction
 
        ldr     \$w2, =BIT17
        str     \$w2, [\$w1, #GPSR]           // clear the LED

        ldr     \$w2, =GPDR_SETTINGS
        str     \$w2, [\$w1, #GPDR]           // set the direction

        .endm

#define BOARD_BOOT_STAGE_1()
#define BOARD_BOOT_STAGE_2()
#define BOARD_BOOT_STAGE_3()
#endif /* __ASSEMBLER__ */

#endif /* __SA1110AS_H__ */
