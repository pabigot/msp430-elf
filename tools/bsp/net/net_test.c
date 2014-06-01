/*
 * net_test.c -- A simple app to allow dowloading and debugging of
 *               BSP network code. The board-specific code should
 *               not include the ethernet support. Use a serial
 *               to run this test.
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
#include <stdio.h>
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include "bsp_if.h"
#include "net.h"

struct bsp_comm_channel *_bsp_net_channel = NULL;

struct bsp_comm_procs *com;


int
test_read(char *buf, int len)
{
    return com->__read(com->ch_data, buf, len);
}


void
test_write(const char *buf, int len)
{
    com->__write(com->ch_data, buf, len);
}


int
test_getc(void)
{
    return com->__getc(com->ch_data);
}

void
test_putc(char ch)
{
    com->__putc(com->ch_data, ch);
}


#define CSR13	0xbfbf0034
#define CSR23	0xbfbf005c

static inline unsigned long
get_count(void)
{
    unsigned long u;
    volatile unsigned *cnt = (volatile unsigned *)CSR13;
    volatile unsigned *iclr = (volatile unsigned *)CSR23;
    
    u = *cnt & 0xfffffff;
    *(volatile unsigned char *)cnt = 0x80;
    
    *iclr = 0x00100000;

    return u;
}


static unsigned long ticks;
static unsigned long last_count;

unsigned long
_bsp_ms_ticks(void)
{
    unsigned long current = get_count();
    unsigned long diff;

    /* its a down counter, so use "last - current" */
    diff = last_count - current;

    ticks += (diff / 1000);

    last_count = current;

    return ticks;
}

int gdb_connect_closed = 0;

void
_bsp_dbg_connect_abort(void)
{
    gdb_connect_closed = 1;
#if 0
    /* if called as part of stub polling, then just longjmp out. */
    if (in_gdb_handler)
	longjmp(gdb_jmpbuf, 1);
#endif
}


/*
 * This is called by the ethernet interrupt handler.
 */
int
gdb_interrupt_check(void)
{
    int nread, i;
    unsigned char *p, buf[1024];

    nread = test_read(buf, sizeof(buf));
    
    if (gdb_connect_closed) {
	gdb_connect_closed = 0;
	return 1;
    }

    for (i = 0, p = buf; i < nread; i++)
	if (*p++ == '\003') {
	    return 1;
	}

    return 0;
}


char buffer[1024];


int
main(int argc, char *argv[])
{
    enet_addr_t addr;
    struct bsp_comm_procs *eprocs;
    int ch;
    volatile int echo_test;

    /* Use ethernet if we can find an IP address using BOOTP. */
	   
    addr[0] = 0x00;
    addr[1] = 0x00;
    addr[2] = 0x0E;
    addr[3] = 0x31;
    addr[4] = 0x00;
    addr[5] = 0x01;

    eprocs = __enet_link_init(addr);
    if (eprocs) {
	if (!_bsp_net_init(eprocs)) {
	    bsp_printf("Network initialization failed.\n");
	    return 0;
	}
    }
    else {
	bsp_printf("Ethernet initialization failed.\n");
	return 0;
    }

    com = &_bsp_net_channel->procs;

    echo_test = 0;

    while (echo_test) {
	ch = test_getc();
	test_putc(ch);
    }

    com->__control(com->ch_data, COMMCTL_INSTALL_DBG_ISR);

    while (1)
	;

    com->__control(com->ch_data, COMMCTL_REMOVE_DBG_ISR);

    return 0;
}


