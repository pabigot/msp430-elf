/*
 * breakpoint.c -- Breakpoint generation.
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
#include "bsp_if.h"

#ifndef DEBUG_BREAKPOINT
#define DEBUG_BREAKPOINT 0
#endif

#if BSP_MAX_BP != 0
static bp_t *last_bp_ptr;
static bp_t *first_bp_ptr;
static bp_t *free_bp_list;
static bp_t bp_list[BSP_MAX_BP];
static int  curr_bp_num;
#endif

/*
 *  Trigger a breakpoint exception.
 */
void
bsp_breakpoint(void)
{
#if DEBUG_BREAKPOINT
    bsp_printf("Before BP\n");
#endif

    /*
     * This nop becomes necessary when doing a Cygmon "transfer" command
     * after breaking program execution using Ctrl-C.  When Cygmon does
     * a "transfer", the GDB Debug handler is reinstated, and the return
     * PC is set to this function.  Without this nop, the irq processing
     * will step over this breakpoint, assuming that since this function
     * is called in a while(1) loop, it will immediately be called again.
     *
     * However, since we are processing an interrupt, the return address
     * is setup for the interrupt handler.  ie The instruction that was
     * interrupted will be reexecuted.  Thus when we return from this
     * routine, we are actually returning to the app that was interrupted.
     *
     * By placing a "nop" here, the irq processing logic in irq-cpu.c will
     * not call bsp_skip_instruction(), and we will hit the breakpoint
     * before issuing a function return and everything will work as expected.
     */
#ifdef __NEED_UNDERSCORE__
    asm volatile (".globl _bsp_force_breakinsn\n"
		  "_bsp_force_breakinsn:\n");
#else
    asm volatile (".globl bsp_force_breakinsn\n"
		  "bsp_force_breakinsn:\n");
#endif
    asm volatile (" nop");

#ifdef __NEED_UNDERSCORE__
    asm volatile (".globl _bsp_breakinsn\n"
		  "_bsp_breakinsn:\n");
#else
    asm volatile (".globl bsp_breakinsn\n"
		  "bsp_breakinsn:\n");
#endif
    BREAKPOINT();
#if DEBUG_BREAKPOINT
    bsp_printf("After BP\n");
#endif
}

#if BSP_MAX_BP != 0
static inline int
_bsp_set_breakpoint(bp_t *p)
{
#if DEBUG_BP
    bsp_printf("bsp_set_breakpoint: addr[%x] opcode[%x] flags[%x]\n",
	       (unsigned long)p->address, p->old_inst, p->flags);
#endif

    if (p->flags & BP_IN_MEMORY)
	return 0;
    if (bsp_memory_read(p->address, -1, sizeof (p->old_inst) * 8, 1, &p->old_inst) != 1)
	return 1;
    if (bsp_memory_write(p->address, -1, sizeof (p->old_inst) * 8, 1, bsp_breakinsn) != 1)
	return 1;
    bsp_flush_dcache(p->address, sizeof(p->old_inst));
    bsp_flush_icache(p->address, sizeof(p->old_inst));
    p->flags |= BP_IN_MEMORY;
    return 0;
}

static inline int
_bsp_clear_breakpoint(bp_t *p)
{
#if DEBUG_BP
    bsp_printf("bsp_clear_breakpoint: addr[%x] opcode[%x] flags[%x]\n",
	       (unsigned long)p->address, p->old_inst, p->flags);
#endif

    if (!(p->flags & BP_IN_MEMORY))
	return 0;
    if (bsp_memory_write(p->address, -1, sizeof (p->old_inst) * 8, 1, &p->old_inst) != 1)
	return 1;
    bsp_flush_dcache(p->address, sizeof(p->old_inst));
    bsp_flush_icache(p->address, sizeof(p->old_inst));
    p->flags &= ~BP_IN_MEMORY;
    return 0;
}



void
bsp_install_all_breakpoints (void)
{
    bp_t *ptr;

#if DEBUG_BP
    bsp_printf("bsp_install_all_breakpoints\n");
#endif

    for (ptr = first_bp_ptr; ptr; ptr = ptr->next)
	_bsp_set_breakpoint(ptr);
}


void
bsp_uninstall_all_breakpoints (void)
{
    bp_t *ptr;

#if DEBUG_BP
    bsp_printf("bsp_uninstall_all_breakpoints\n");
#endif

    for (ptr = first_bp_ptr; ptr; ptr = ptr->next)
	_bsp_clear_breakpoint(ptr);
}


int
bsp_add_breakpoint(void *location, unsigned long length)
{
    bp_t *ptr;

#if DEBUG_BP
    bsp_printf("bsp_add_breakpoint: addr[%lx] len[%d]\n",
	       (unsigned long)location, length);
#endif

    /* Its an error to set more than one at a given location */
    for (ptr = first_bp_ptr; ptr != NULL; ptr = ptr->next)
	if (ptr->address == location)
	    return 1;

    /* Its an error to run out of breakpoint space. */
    if (free_bp_list == NULL)
	return 1;

    ptr = free_bp_list;
    free_bp_list = ptr->next;

    if (first_bp_ptr == NULL)
	first_bp_ptr = ptr;
    else
	last_bp_ptr->next = ptr;

    last_bp_ptr = ptr;
    last_bp_ptr->next = NULL;
    last_bp_ptr->address = location;
    last_bp_ptr->flags = 0;

    return 0;
}

int 
bsp_remove_breakpoint(void *location, unsigned long length)
{
    int error = 0;
    bp_t *ptr = first_bp_ptr;
    bp_t *prev_ptr = NULL;

#if DEBUG_BP
    bsp_printf("bsp_remove_breakpoint: addr[%lx] len[%d]\n",
	       (unsigned long)location, length);
#endif

    /* Scan the list looking for the address to clear */
    while (ptr != NULL && ptr->address != location) {
	/* keep a pointer one behind the current position */
	prev_ptr = ptr;
	ptr = ptr->next;
    }
    if (ptr == NULL)
	return 1;

    /* Just in case it's still in memory. */
    error = _bsp_clear_breakpoint(ptr);
    if (error)
	return error;

    /* now we'll point the previous bp->next at the one after the one 
       we're deleting, unless there is no previous bp. */
    if (prev_ptr != NULL)
	prev_ptr->next = ptr->next;

    if (first_bp_ptr == ptr)
	first_bp_ptr = ptr->next;

    if (last_bp_ptr == ptr)
	last_bp_ptr = prev_ptr;

    ptr->flags = 0;
    ptr->next = free_bp_list;
    free_bp_list = ptr;

    return 0;
}	


void
bsp_init_breakpoints(void)
{
    bp_t *ptr, *new_bp_ptr;

    /* one time only initialization */
    free_bp_list = new_bp_ptr = &bp_list[0];
    for (ptr = &bp_list[1]; ptr <= &bp_list[BSP_MAX_BP-1]; ptr++) {
	new_bp_ptr->flags = 0;
	new_bp_ptr->next = ptr;
	new_bp_ptr = ptr;
    }
    ptr->next = NULL;
    ptr->flags = 0;
    new_bp_ptr = free_bp_list;
    free_bp_list = new_bp_ptr->next;
}

#endif /* BSP_MAX_BP != 0 */

