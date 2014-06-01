/*
 * ppc4xx.h -- Defines for PPC 403 family registers.
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
#ifndef __PPC4XX_H__
#define __PPC4XX_H__ 1


/*
 *  Serial port register definitions.
 */
#define UART_LS_RXRDY		0x80
#define UART_LS_FE		0x40
#define UART_LS_OE		0x20
#define UART_LS_PE		0x10
#define UART_LS_LB		0x08
#define UART_LS_TXRDY		0x04
#define UART_LS_TXEMPTY		0x02

#define UART_HS_DSR		0x80
#define UART_HS_CTS		0x40

#define UART_CTL_NORMAL_MODE    0x00
#define UART_CTL_LOOPBACK_MODE  0x40
#define UART_CTL_ECHO_MODE      0x80
#define UART_CTL_DTR            0x20
#define UART_CTL_RTS            0x10
#define UART_CTL_8BIT           0x08
#define UART_CTL_7BIT           0x00
#define UART_CTL_ODD            0x06
#define UART_CTL_EVEN           0x04
#define UART_CTL_NONE           0x00
#define UART_CTL_1STOP          0x00
#define UART_CTL_2STOP          0x01

#define UART_RC_ENABLE		0x80
#define UART_RC_RXINT		0x20
#define UART_RC_DMA2		0x40
#define UART_RC_DMA3		0x60
#define UART_RC_ERRINT		0x10
#define UART_RC_PAUSE_MODE	0x08

#define UART_TC_ENABLE		0x80
#define UART_TC_TXINT		0x20
#define UART_TC_DMA2		0x40
#define UART_TC_DMA3		0x60
#define UART_TC_EMPTYINT	0x10
#define UART_TC_ERRORINT	0x08
#define UART_TC_CTS_PAUSE	0x04
#define UART_TC_BREAK		0x02
#define UART_TC_PATGEN		0x01



#ifndef __ASSEMBLER__

typedef struct {
    volatile unsigned char spls;	/* line status register */
    volatile unsigned char pad1;
    volatile unsigned char sphs;	/* handshake status register */
    volatile unsigned char pad3;
    volatile unsigned char brdh;	/* baudrate high */
    volatile unsigned char brdl;	/* baudrate low */
    volatile unsigned char spctl;	/* control register */
    volatile unsigned char sprc;	/* receive command register */
    volatile unsigned char sptc;	/* transmit command register */
    volatile unsigned char spb;
} uart_403_t;


static inline void __set_iocr(unsigned val)
{
    asm volatile (
        "mtiocr   %0\n"
	: /* no outputs */
	: "r" (val)  );
}

static inline unsigned __get_iocr(void)
{
    unsigned retval;

    asm volatile (
        "mfiocr   %0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}


static inline void __set_exisr(unsigned val)
{
    asm volatile (
        "mtexisr   %0\n"
	: /* no outputs */
	: "r" (val)  );
}

static inline unsigned __get_exisr(void)
{
    unsigned retval;

    asm volatile (
        "mfexisr   %0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

static inline void __set_exier(unsigned val)
{
    asm volatile (
        "mtexier   %0\n"
	: /* no outputs */
	: "r" (val)  );
}

static inline unsigned __get_exier(void)
{
    unsigned retval;

    asm volatile (
        "mfexier   %0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}


static inline void __set_iccr(unsigned val)
{
    asm volatile (
        "mticcr   %0\n"
        "isync\n"
        "sync\n"
	: /* no outputs */
	: "r" (val)  );
}

static inline void __set_dccr(unsigned val)
{
    asm volatile (
        "mtdccr   %0\n"
        "sync\n"
	: /* no outputs */
	: "r" (val)  );
}

static inline void __set_esr(unsigned val)
{
    asm volatile (
        "mtesr   %0\n"
	: /* no outputs */
	: "r" (val)  );
}

static inline unsigned __get_esr(void)
{
    unsigned retval;

    asm volatile (
        "mfesr   %0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

static inline void __set_besr(unsigned val)
{
    asm volatile (
        "mtbesr   %0\n"
	: /* no outputs */
	: "r" (val)  );
}

static inline unsigned __get_besr(void)
{
    unsigned retval;

    asm volatile (
        "mfbesr   %0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

static inline void __set_dbcr(unsigned val)
{
    asm volatile (
        "mtspr   1010,%0\n"
	: /* no outputs */
	: "r" (val)  );
}

static inline unsigned __get_dbcr(void)
{
    unsigned retval;

    asm volatile (
        "mfspr   %0,1010\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

static inline void __set_dbsr(unsigned val)
{
    asm volatile (
        "mtdbsr   %0\n"
	: /* no outputs */
	: "r" (val)  );
}

static inline unsigned __get_dbsr(void)
{
    unsigned retval;

    asm volatile (
        "mfdbsr   %0\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}


#endif /* __ASSEMBLER__ */
#endif /* __PPC4XX_H__ */

