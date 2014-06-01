/*
 * mb8683x.h -- Defines specific to Fujitsu MB8683x eval board.
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
 */
#ifndef __MB8683X_H__
#define __MB8683X_H__ 1


/* Address of clock switch */
#define CLKSW_ADDR  0x01000003

/* SW reset address */
#define SWRESET_ADDR 0x01000003

/* Address of SW1 */
#define SW1_ADDR  0x02000003

/* Address of LED bank */
#define LED_ADDR  0x02000003

#define SRAM_BASE 0x30000000
#define SRAM_END  0x30080000


#endif
