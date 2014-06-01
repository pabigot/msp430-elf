/* 
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

#include <bsp/hex-utils.h>

int
__hex(char ch)
{
    if ((ch >= 'a') && (ch <= 'f')) return (ch-'a'+10);
    if ((ch >= '0') && (ch <= '9')) return (ch-'0');
    if ((ch >= 'A') && (ch <= 'F')) return (ch-'A'+10);
    return (-1);
}


/*
 * Convert the hex data in 'buf' into 'count' bytes to be placed in 'mem'.
 * Returns a pointer to the character in mem AFTER the last byte written.
 */
char *
__unpack_bytes_to_mem(char *buf, char *mem, int count)
{
    int  i;
    char ch;

    for (i = 0; i < count; i++) {
	ch = __hex(*buf++) << 4;
	ch = ch + __hex(*buf++);
	*mem++ = ch;
    }
    return(mem);
}

/*
 * While finding valid hex chars, build an unsigned long int.
 * Return number of hex chars processed.
 */
int
__unpack_ulong(char **ptr, unsigned long *val)
{
    int numChars = 0;
    int hexValue;
    
    *val = 0;

    while (**ptr) {
	hexValue = __hex(**ptr);
	if (hexValue >= 0) {
	    *val = (*val << 4) | hexValue;
	    numChars ++;
	} else
	    break;
	(*ptr)++;
    }
    return (numChars);
}


/*
 * Unpack 'count' hex characters, forming them into a binary value.
 * Return that value as an int. Adjust the source pointer accordingly.
 */
int
__unpack_nibbles(char **ptr, int count)
{
    int value = 0;

    while (--count >= 0) {
	value = (value << 4) | __hex(**ptr);
	(*ptr)++;
    }
    return value;
}


