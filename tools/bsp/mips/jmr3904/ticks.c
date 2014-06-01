/*
 * ticks.c -- Support for millisecond timer on JMR3904 board.
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


typedef struct {
    volatile unsigned tcr;
#define TCR_MODE_INTERVAL 0
#define TCR_MODE_PULSE    1
#define TCR_MODE_WATCHDOG 2
#define TCR_MODE_DISABLED 3
#define TCR_CCS_INT       (0 << 2)
#define TCR_CCS_EXT       (1 << 2)
#define TCR_ECES_FALL     (0 << 3)
#define TCR_ECES_RISE     (1 << 3)
#define TCR_CRE           (1 << 5)
#define TCR_CCDE          (1 << 6)
#define TCR_TCE           (1 << 7)

    volatile unsigned tisr;
#define TISR_TIIS         1
#define TISR_TPIAS        (1 << 1)
#define TISR_TPIBS        (1 << 2)
#define TISR_TWIS         (1 << 3)

    /* compare registers */
    volatile unsigned cpra;
    volatile unsigned cprb;

    volatile unsigned itmr;
#define ITMR_TZCE         (1 << 0)
#define ITMR_TIIE         (1 << 15)
    unsigned pad1[3];

    volatile unsigned ccdr;
#define CCDR_DIV2         0
#define CCDR_DIV4         1
#define CCDR_DIV8         2
#define CCDR_DIV16        3
#define CCDR_DIV32        4
#define CCDR_DIV64        5
#define CCDR_DIV128       6
#define CCDR_DIV256       7
    unsigned pad2[3];

    volatile unsigned pgmr;
#define PGMR_FFI          1
#define PGMR_TPIAE        (1 << 14)
#define PGMR_TPIBE        (1 << 15)
    unsigned pad3[3];

    volatile unsigned wtmr;
#define WTMR_TWC          1
#define WTMR_WDIS         (1 << 7)
#define WTMR_TWIE         (1 << 15)
    unsigned pad4[3];

    unsigned pad5[40];

    /* read register */
    volatile unsigned trr;
} tx3904_timer_t;

#define tx3904_tmr0 ((tx3904_timer_t *)0xfffff000)
#define tx3904_tmr1 ((tx3904_timer_t *)0xfffff100)
#define tx3904_tmr2 ((tx3904_timer_t *)0xfffff200)

static tx3904_timer_t *_bsp_timer;

#define CNT_MASK 0xffffff
#define TICKS_PER_MS 312


static unsigned long ticks;
static unsigned int  last_count;

void
init_ms_timer(int i)
{
    tx3904_timer_t *t;

    switch (i) {
      case 2:
	t = tx3904_tmr2;
	break;
      case 1:
	t = tx3904_tmr1;
	break;
      case 0:
      default:
	t = tx3904_tmr0;
	break;
    }

    _bsp_timer = t;

    /* make sure its disabled */
    t->tcr = TCR_MODE_DISABLED;

    t->cpra = CNT_MASK;
    t->itmr = ITMR_TZCE;
    t->ccdr = CCDR_DIV256;
    t->tcr = TCR_MODE_INTERVAL | TCR_CCDE | TCR_TCE;

    ticks = 0;
    last_count = t->trr & CNT_MASK;
}

static unsigned int
__get_count(void)
{
    unsigned u;
    
    u = _bsp_timer->trr & CNT_MASK;

    if (_bsp_timer->tisr & TISR_TIIS) {
	u += (CNT_MASK + 1);
	_bsp_timer->tisr = 0;
    }

    return u;
}

unsigned long
_bsp_ms_ticks(void)
{
    unsigned int        current = __get_count();
    static unsigned int diff = 0;

    /* its an up counter, so use "current - last" */
    diff += ((current - last_count) & CNT_MASK);

    if (diff >= TICKS_PER_MS) {
	ticks += (diff / TICKS_PER_MS);
	diff = (diff % TICKS_PER_MS);
    }

    last_count = current;

    return ticks;
}

