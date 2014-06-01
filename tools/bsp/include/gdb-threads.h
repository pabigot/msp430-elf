/* 
 * gdb-threads.h -
 * Optional packet processing for thread aware debugging.
 * Externs as called by central packet switch routine
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
#if !defined(__GDB_THREADS_H__)
#define __GDB_THREADS_H__ 1

#ifdef GDB_THREAD_SUPPORT
/*
 * Export the continue and "general" (context) thread IDs from GDB.
 */
extern int _gdb_cont_thread;
extern int _gdb_general_thread;

/*
 * The registers belonging to _gdb_general_thread.
 */
extern ex_regs_t _gdb_general_registers;

extern void bsp_copy_gdb_to_exc_regs(ex_regs_t *ex_regs, gdb_regs_t *gdb_regs);
extern void bsp_copy_exc_to_gdb_regs(gdb_regs_t *gdb_regs, ex_regs_t *ex_regs);
extern int  bsp_set_interrupt_enable(int disable, void *regs);

extern void _gdb_changethread(char *pkt, ex_regs_t *exc_regs);
extern void _gdb_getthreadlist(char *pkt);
extern void _gdb_getthreadinfo(char *pkt);
extern void _gdb_thread_alive(char *pkt);
extern void _gdb_currthread(void);
extern int  _gdb_lock_scheduler(int lock, int mode, long id);
extern int  _gdb_get_currthread(void);

#endif

#endif
