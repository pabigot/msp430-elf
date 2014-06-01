/* 
 * Pseudo system calls to communicate with a kernel supporting gdb thread-aware
 * debugging.
 *
 * Copyright (c) 1998, 1999, 2000 Cygnus Support
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

#include <stdlib.h>
#include <bsp/cpu.h>
#include <bsp/dbg-threads-api.h>
#include "threads-syscall.h"

static union dbg_thread_syscall_parms tcall ;

#ifdef __ECOS_HAL__
#define bsp_vsr_table hal_vsr_table
#endif

static inline dbg_syscall_func
get_syscall_hook(void)
{
#ifdef BSP_VEC_MT_DEBUG
    extern void *bsp_vsr_table[];
    
    return bsp_vsr_table[BSP_VEC_MT_DEBUG];
#else
    return NULL;
#endif
}

/* All forms of failure return 0 */
/* Whether non support, incomplete initialization, unknown thread */
static inline int
dbg_thread_syscall(enum dbg_syscall_ids id)
{
    dbg_syscall_func f ; /* double indirect via */

    f = get_syscall_hook();
    if (f == NULL)
	return 0 ; /* no pointer to vector */

    return (*f)(id, &tcall);
}


int
dbg_thread_capabilities(struct dbg_capabilities * cbp)
{
    tcall.cap_parms.abilities = cbp ;
    return dbg_thread_syscall(dbg_capabilities_func) ;
}

int
dbg_currthread(threadref * varparm)
{
    tcall.currthread_parms.ref = varparm ;
    return dbg_thread_syscall(dbg_currthread_func) ;
}

int
dbg_threadlist(int startflag, threadref *lastthreadid, threadref *next_thread)
{
    tcall.threadlist_parms.startflag = startflag ;
    tcall.threadlist_parms.lastid = lastthreadid ;
    tcall.threadlist_parms.nextthreadid = next_thread ;
    return dbg_thread_syscall(dbg_threadlist_func) ;
}

int
dbg_threadinfo(threadref *threadid, struct cygmon_thread_debug_info *info)
{
    tcall.info_parms.ref = threadid ;
    tcall.info_parms.info = info ;
    return dbg_thread_syscall(dbg_threadinfo_func) ;
}

int
dbg_getthreadreg(threadref *osthreadid, int regcount, void *regval)
{
    tcall.reg_parms.thread =    osthreadid ;
    tcall.reg_parms.regcount =  regcount ;
    tcall.reg_parms.registers = regval ;
    return dbg_thread_syscall(dbg_getthreadreg_func) ;
}

int
dbg_setthreadreg(threadref *osthreadid, int regcount, void *regval)
{
    tcall.reg_parms.thread =    osthreadid ;
    tcall.reg_parms.regcount =  regcount ;
    tcall.reg_parms.registers =  regval ;
    return dbg_thread_syscall(dbg_setthreadreg_func) ;
}

int
dbg_scheduler(threadref *thread_id, int lock, int mode)
{
    if (get_syscall_hook() == NULL)
	return -1;

    tcall.scheduler_parms.thread    = thread_id;
    tcall.scheduler_parms.lock      = lock ;
    tcall.scheduler_parms.mode      = mode ;
    return dbg_thread_syscall(dbg_scheduler_func) ;
}
