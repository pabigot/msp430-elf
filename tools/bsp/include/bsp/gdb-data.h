/*
 * gdb-data.h -- Shared data specific to gdb stub.
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
#ifndef __GDB_DATA_H__
#define __GDB_DATA_H__ 1

#ifndef __ASSEMBLER__
typedef int (*gdb_memproc_t)(void *__addr,    /* start addr of memory to read/write */
			     int  __asid,     /* address space id */
			     int  __size,     /* size of individual read/write ops */
			     int  __n,        /* number of read/write operations */
			     void *__buf);    /* result(read)/src(write) buffer */


typedef void (*gdb_regproc_t)(int  __regno,   /* Register number */
			      void *__regs,   /* pointer to saved regs */
			      void *__val);   /* pointer to register value */


typedef struct {
    /*
     * An application may override the standard BSP memory
     * read and/or write routines with these hooks.
     */
    gdb_memproc_t	__mem_read_hook;
    gdb_memproc_t	__mem_write_hook;

    /*
     * An application may override the standard BSP register
     * access routines with these hooks.
     */
    gdb_regproc_t	__reg_get_hook;
    gdb_regproc_t	__reg_set_hook;

    /*
     * An application may extend the gdb remote protocol by
     * installing hooks to handle unknown general query and
     * set packets ("q" pkts and 'Q' pkts) with these two hooks.
     */
    void                (*__pkt_query_hook)(unsigned char *__pkt);
    void                (*__pkt_set_hook)(unsigned char *__pkt);

    /*
     * An application may also extend the gdb remote protocol
     * by installing a hook to handle all unknown packets.
     */
    void                (*__pkt_hook)(unsigned char *__pkt);

    /*
     * The above hooks for receiving packets will probably need
     * a mechanism to respond. This vector is provided to allow
     * an application to append data to the outgoing packet which
     * will be sent after the above hooks are called.
     *
     * This vector uses a printf-like format string followed by
     * some number of arguments.
     */
    void                (*__pkt_append)(char *fmt, ...);

    /*
     * An application can read/write from/to gdb console
     * through these vectors.
     *
     * NB: console read is not supported and will block forever.
     */
    int                 (*__console_read)(char *__buf, int len);
    int                 (*__console_write)(char *__buf, int len);

} gdb_data_t;


extern gdb_data_t *__get_gdb_data(void);

#endif /* __ASSEMBLER__ */


#endif
