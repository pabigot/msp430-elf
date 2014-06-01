/*
 * init_328ads.S -- Initialization for Motorola Dragonball(68328) ADS board.
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
	.file "init_328ads.S"
        .title "Boot Code for Dragonball ADS board"

#include __BOARD_HEADER__
        
	.text

        .align     2
FUNC_START _bsp_init_stack
    /*
     * Dragonball Register base will remain in a5
     */
    moveal  IMM(MC68328_REG_BASE), a5

    /*
     * Setup the chip selects
     */

    /*
     * Group A: ROM = CSA0 -> CSA3
     *
     * GRPBASE     = $004XXXXX
     * GRPMASK     = $003XXXXX
     * Group Range =[$00400000,$007FFFFF], Size = 4M
     *
     * CSA0:  Base       = $XX40XXXX
     *        Mask       = $XX0FXXXX
     *        CSA0 Range =[$00400000,$004FFFFF], Size = 1M
     * CSA1:  Base       = $XX50XXXX
     *        Mask       = $XX0FXXXX
     *        CSA1 Range =[$00500000,$005FFFFF], Size = 1M
     * CSA2:  Base       = $Xx60XXXX
     *        Mask       = $XX0FXXXX
     *        CSA2 Range =[$00600000,$006FFFFF], Size = 1M
     * CSA3:  Base       = $Xx70XXXX
     *        Mask       = $XX0FXXXX
     *        CSA1 Range =[$00700000,$007FFFFF], Size = 1M
     */
    movel   IMM((0x40<<24)+(0x1<<16)+(0x0F<<8)+(0x1<<3)+1), a5@(MC68328_CSA0_o)
#if 0
    movel   IMM((0x50<<24)+(0x1<<16)+(0x0F<<8)+(0x1<<3)+1), a5@(MC68328_CSA1_o)
    movel   IMM((0x60<<24)+(0x1<<16)+(0x0F<<8)+(0x1<<3)+1), a5@(MC68328_CSA2_o)
    movel   IMM((0x70<<24)+(0x1<<16)+(0x0F<<8)+(0x1<<3)+1), a5@(MC68328_CSA3_o)
#endif
    movew   IMM(0x003<<4), a5@(MC68328_GRPMASKA_o)
    movew   IMM((0x004<<4)+1), a5@(MC68328_GRPBASEA_o)

    /* 
     * Group B: SRAM = CSB0 -> CSB3
     *
     * GRPBASE     = $000XXXXX
     * GRPMASK     = $003XXXXX
     * Group Range =[$00000000,$003FFFFF], Size = 4M
     *
     * CSB0:  Base       = $XX00XXXX
     *        Mask       = $XX0FXXXX
     *        CSB0 Range =[$00000000,$000FFFFF], Size = 1M
     * CSB1:  Base       = $XX10XXXX
     *        Mask       = $XX0FXXXX
     *        CSB1 Range =[$00100000,$001FFFFF], Size = 1M
     * CSB2:  Base       = $XX20XXXX
     *        Mask       = $XX0FXXXX
     *        CSB2 Range =[$00200000,$002FFFFF], Size = 1M
     * CSB3:  Base       = $XX30XXXX
     *        Mask       = $XX0FXXXX
     *        CSB3 Range =[$00300000,$003FFFFF], Size = 1M
     */
    movel   IMM((0x00<<24)+(0x1<<16)+(0x0F<<8)+(0x0<<3)), a5@(MC68328_CSB0_o)
    movel   IMM((0x10<<24)+(0x1<<16)+(0x0F<<8)+(0x0<<3)), a5@(MC68328_CSB1_o)
    movel   IMM((0x20<<24)+(0x1<<16)+(0x0F<<8)+(0x0<<3)), a5@(MC68328_CSB2_o)
#if 0
    movel   IMM((0x30<<24)+(0x1<<16)+(0x0F<<8)+(0x0<<3)), a5@(MC68328_CSB3_o)
#endif
    movew   IMM(0x003<<4), a5@(MC68328_GRPMASKB_o)
    movew   IMM((0x000<<4)+1), a5@(MC68328_GRPBASEB_o)

    /* 
     * Group D: DUART=CSD1, ADI=CSD2
     *
     * GRPBASE     = $00FXXXXX
     * GRPMASK     = $000XXXXX
     * Group Range =[$00F00000,$00FFFFFF], Size = 1M
     *
     * CSD1:  Base       = $XXF00XXX
     *        Mask       = $XX000XXX
     *        CSD1 Range =[$00F00000,$00F00FFF], Size = 4K
     */
    movel   IMM((0xF00<<20)+(0x1<<16)+(0x000<<4)+(0x0<<3)+6), a5@(MC68328_CSD1_o)
    movew   IMM(0x000<<4), a5@(MC68328_GRPMASKD_o)
    movew   IMM((0x00F<<4)+1), a5@(MC68328_GRPBASED_o)

    movel   IMM(USERSTACK), a5
    movel   a5, usp

    /*
     * Tell c_start where the top of ram is
     */
    movel  IMM(SUPERSTACK),sp@-

    /*
     * Return (w/o using stack)
     */
    jmp     (a0)
FUNC_END _bsp_init_stack
