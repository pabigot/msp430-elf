/*
 * hex-utils.h -- Utilities for decoding hexadecimal encoded integers.
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
#ifndef __HEX_UTILS_H__
#define __HEX_UTILS_H__ 1

#ifndef __ASSEMBLER__

/*
 * Convert a single hex character to its binary value.
 * Returns -1 if given character is not a value hex character.
 */
extern int __hex(char ch);

/*
 * Convert the hex data in 'buf' into 'count' bytes to be placed in 'mem'.
 * Returns a pointer to the character in mem AFTER the last byte written.
 */
extern char *__unpack_bytes_to_mem(char *buf, char *mem, int count);

/*
 * While finding valid hex chars, build an unsigned long int.
 * Return number of hex chars processed.
 */
extern int __unpack_ulong(char **ptr, unsigned long *val);

/*
 * Unpack 'count' hex characters, forming them into a binary value.
 * Return that value as an int. Adjust the source pointer accordingly.
 */
extern int __unpack_nibbles(char **ptr, int count);

#endif

#endif
