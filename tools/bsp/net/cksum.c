/*
 * cksum.c -- A naive implementation of IP checksum utility.
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
#include <bsp/bsp.h>
#include "bsp_if.h"
#include "net.h"

/*
 * Do a one's complement checksum.
 * The data being checksum'd is in network byte order.
 * The returned checksum is in network byte order.
 */
unsigned short
__sum(word *w, int len, int init_sum)
{
    int sum = init_sum;

    union {
	unsigned char c[2];
	unsigned short s;
    } su;

    union {
	unsigned short s[2];
	int i;
    } iu;

    while ((len -= 2) >= 0)
	sum += *w++;

    if (len == -1) {
	su.c[0] = *(char *)w;
	su.c[1] = 0;
	sum += su.s;
    }

    iu.i = sum;
    sum = iu.s[0] + iu.s[1];
    if (sum > 65535)
	sum -= 65535;

    su.s = ~sum;

    return (su.c[0] << 8) | su.c[1];
}



/*
 * Compute a partial checksum for the UDP/TCP pseudo header.
 */
int
__pseudo_sum(ip_header_t *ip)
{
    int    sum;
    word   *p;
    char   cbuf[2];

    p = (word *)ip->source;
    sum  = *p++;
    sum += *p++;
    sum += *p++;
    sum += *p++;
    
    cbuf[0] = 0;
    cbuf[1] = ip->protocol;
    sum += *(word *)cbuf;

    sum += ip->length;
    
    return sum;
}
