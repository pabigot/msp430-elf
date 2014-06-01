/*
 * bsp_if.h -- BSP interface definitions.
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
#ifndef __BSP_IF_H__
#define __BSP_IF_H__ 1

#include <bsp/bsp.h>

/*
 *  Maximum number of interrupt controllers supported by
 *  this bsp.
 */
#define BSP_MAX_IRQ_CONTROLLERS 8

#ifndef __ASSEMBLER__


/*
 *  Interrupt controller abstraction.
 *  Each interrupt controller on a given board should be described using
 *  this data structure.
 */
struct bsp_irq_controller {
    /*
     * First and last irqs handled by this controller.
     */
    short	first;
    short	last;

    /* 
     * pointer to array of bsp_vec struct pointers. These are
     * the heads of the linked list of ISRs for each irq handled
     * by this controller.
     */
    bsp_vec_t	**vec_list;

    /*
     * Pointer to initialization routine which is run once at boot time.
     */
    void	(*init)(const struct bsp_irq_controller *__ic);

    /*
     * Pointer to routines used to disable and enable interrupts handled
     * by this controller.
     */
    int		(*disable)(const struct bsp_irq_controller *__ic,
			   int __irq_nr);
    void	(*enable)(const struct bsp_irq_controller *__ic,
			  int __irq_nr);
};


/*
 * Board specific code needs to provide at least one communication channel
 * for use as the debug and console (stdio) channel. For each channel,
 * there must be a set of function vectors for the common BSP code to
 * control the channel.
 */
struct bsp_comm_procs {
    /*
     * Implementation dependent data pointer passed to the following procs.
     */
    void *ch_data;

    /*
     * Write a buffer of the given length. All of buffer is sent before
     * the write call returns.
     */
    void (*__write)(void *ch_data, const char *buf, int len);

    /*
     * Fill a buffer with up to the given length. Returns the actual number
     * of characters read.
     */
    int  (*__read)(void *ch_data, char *buf, int len);

    /*
     * Send a single character.
     */
    void (*__putc)(void *ch_data, char ch);

    /*
     * Read a single character. If no character is immediately available, will
     * block until one becomes available.
     */
    int  (*__getc)(void *ch_data);

    /*
     * Catchall comm port control.
     */
    int  (*__control)(void *ch_data, int func, ...);

    /*
     * For serial ports, the control function may be used to set and get the
     * current baud rate. Usage:
     * 
     *   err = (*__control)(COMMCTL_SETBAUD, int bits_per_second);
     *     err => Zero if successful, -1 if error.
     *
     *   baud = (*__control)(COMMCTL_GETBAUD);
     *     baud => -1 if error, current baud otherwise.
     */
#define COMMCTL_SETBAUD 0
#define COMMCTL_GETBAUD 1

    /*
     * Install and remove debugger interrupt handlers. These are the receiver
     * interrupt routines which are used to change control from a running
     * program to the debugger stub.
     */
#define COMMCTL_INSTALL_DBG_ISR 2
#define COMMCTL_REMOVE_DBG_ISR  3

    /*
     * Disable comm port interrupt. Returns TRUE if interrupt was enabled,
     * FALSE otherwise.
     */
#define COMMCTL_IRQ_DISABLE 4
    /*
     * Enable comm port interrupt.
     */
#define COMMCTL_IRQ_ENABLE 5
};


/*
 * The board specific code uses this data structure to provide information
 * about and procedure vectors for each supported communication channel.
 * See _bsp_comm_list below.
 */
struct bsp_comm_channel {
    struct bsp_comm_info   info;
    struct bsp_comm_procs  procs;
};


/*
 * Number to place in the version field. If structure is changed
 * in a way which is not backwards compatible, this number should
 * be incremented.
 */
#define BSP_SHARED_DATA_VERSION 2

/*
 * Clients of this BSP will need to have access to BSP functions and
 * data structures. Because, the client and the BSP may not be linked
 * together, a structure of vectors is used to gain this access. A
 * pointer to this structure can be gotten via a syscall. This syscall
 * is made automatically from within the crt0.o file.
 */
typedef struct {
    int		version;	/* version number for future expansion */

    /*
     *  Pointer to the array of pointers to interrupt controller descriptors.
     */
    const struct bsp_irq_controller **__ictrl_table;

    /*
     *  Pointer to the array of exception vectors.
     */
    bsp_vec_t **__exc_table;

    /*
     * Pointer to debug handler vector.
     */
    bsp_handler_t *__dbg_vector;

    /*
     * User hook to catch debugger 'kill' command.
     */
    bsp_handler_t __kill_vector;

    /*
     * Vectored functions for console and debug i/o.
     */
    struct bsp_comm_procs *__console_procs;
    struct bsp_comm_procs *__debug_procs;

    /*
     * Vectored cache control functions.
     */
    void (*__flush_dcache)(void *__p, int __nbytes);
    void (*__flush_icache)(void *__p, int __nbytes);

    /*
     * Generic data pointers
     */
    void *__cpu_data;
    void *__board_data;

    /*
     * General BSP information access.
     * See bsp.h for details.
     */
    int  (*__sysinfo)(enum bsp_info_id __id, va_list __ap);

    /*
     * Set or get active debug and console channels.
     * Returns -1 if unsucessful.
     * If the passed in __comm_id is -1, then the id of the current channel
     * is returned.
     */
    int	 (*__set_debug_comm)(int __comm_id);
    int	 (*__set_console_comm)(int __comm_id);

    /*
     * Set or get the current baud rate of a serial comm channel.
     * Returns -1 on if unsuccessful.
     * If the given baud is -1, then the current baudrate is returned.
     */
    int  (*__set_serial_baud)(int __comm_id, int baud);

    /*
     * Debug agent data.
     */
    void *__dbg_data;

    /*
     * Reset function
     * We want to avoid calling this with a trap since
     * we may be calling it from SWI mode (in cygmon).
     * That is problematic, as nested SWI's are not
     * very good.
     */
    void (*__reset)(void);

    /*
     * TRUE if console interrupt detected during program output.
     */
    int  __console_interrupt_flag;

} bsp_shared_t;


#ifdef __arm__
/* Version 1 shared data structure is laid out differently on arm. */
typedef struct {
    int		version;
    const struct bsp_irq_controller **__ictrl_table;
    bsp_vec_t **__exc_table;
    bsp_handler_t *__dbg_vector;
    bsp_handler_t __kill_vector;
    struct bsp_comm_procs *__console_procs;
    struct bsp_comm_procs *__debug_procs;
    void (*__flush_dcache)(void *__p, int __nbytes);
    void (*__flush_icache)(void *__p, int __nbytes);

    /* Reset is inserted here on version 1 arm boards */
    void (*__reset)(void);

    void *__cpu_data;
    void *__board_data;
    int  (*__sysinfo)(enum bsp_info_id __id, va_list __ap);
    int	 (*__set_debug_comm)(int __comm_id);
    int	 (*__set_console_comm)(int __comm_id);
    int  (*__set_serial_baud)(int __comm_id, int baud);
    void *__dbg_data;
} bsp_v1_shared_t;
#else
/* ! __arm__ */
typedef bsp_shared_t bsp_v1_shared_t;
#endif

extern bsp_shared_t *bsp_shared_data;

/*
 * Platform info which may be overriden/modified by arch/board specific code.
 */
extern struct bsp_platform_info _bsp_platform_info;

/*
 * Cache info which may be overriden/modified by arch/board specific code.
 */
extern struct bsp_cachesize_info _bsp_dcache_info;
extern struct bsp_cachesize_info _bsp_icache_info;
extern struct bsp_cachesize_info _bsp_scache_info;

/*
 * Array of comm channel descriptors which must be provided by board specific
 * code.
 */
extern struct bsp_comm_channel _bsp_comm_list[];

/*
 * Number of comm channel descriptors which must be provided by board specific
 * code.
 */
extern int _bsp_num_comms;


/*
 * Array of memory region descriptors which must be provided by board specific
 * code.
 */
extern struct bsp_mem_info _bsp_memory_list[];

/*
 * Number of memory region descriptors which must be provided by board specific
 * code.
 */
extern int _bsp_num_mem_regions;

/*
 * In order to construct the above _bsp_memory_list, some board specific
 * code may have to size RAM regions. To do this easily and reliably,
 * the code needs to run from ROM before .bss and .data sections are
 * initialized. This leads to the problem of where to store the results
 * of the memory sizing tests. In this case, the _bsp_init_stack routine
 * which sizes memory and sets up the stack will place the board-specific
 * information on the stack and return with the stack pointer pointing to
 * a pointer to the information. That is, addr_of_info = *(void **)sp.
 * The architecture specific code will then copy that pointer to the
 * _bsp_ram_info_ptr variable after initializing the .data and .bss sections.
 */
extern void *_bsp_ram_info_ptr;

/*
 * Generic bsp initialization. Called by low level startup code
 */
extern void _bsp_init(void);

/*
 *  Initialize board communication in polling mode. This enables
 *  debugging printf for later initializations. Interrupts for
 *  comm channels may be set up later in _bsp_board_init().
 */
extern void _bsp_init_board_comm(void);

/*
 * Make generic BSP aware of CPU/MCU specific interrupt controllers.
 */
extern void _bsp_install_cpu_irq_controllers(void);

/*
 * Make generic BSP aware of board specific interrupt controllers.
 */
extern void _bsp_install_board_irq_controllers(void);

/*
 * Callback used by above two routines to install a single
 * interrupt controller.
 */
extern void _bsp_install_irq_controller(const struct bsp_irq_controller *__ic);

/*
 *  Generic exception dispatch routine. Usually called from asm-level
 *  exception handler to call vectors in vector chain for the given
 *  exception number. Stops traversing vector chain when a called
 *  vector returns a non-zero value. If no vector returns non-zero,
 *  a default error message and register dump is printed.
 */
extern int _bsp_exc_dispatch(int __exc_number, void *__regs);


/*
 * Architecture specific routine to dump register values.
 */
extern void _bsp_dump_regs(void *__regs);


/*
 * Generic syscall handler called by architecture specific handler.
 * Returns non-zero if given 'func' number was handled by the generic
 * code, zero otherwise. If handled, the syscall error is returned
 * via the err_ptr.
 */
extern int  _bsp_do_syscall(int __func,
			    long __arg1, long __arg2, long __arg3, long __arg4,
			    int *__err_ptr);


extern void _bsp_cpu_init(void);
extern void _bsp_board_init(void);


/*
 * General interface for getting certain BSP parameters.
 * See bsp.h for details.
 */
extern int  _bsp_sysinfo(enum bsp_info_id __id, va_list __ap);

/*
 * Called from comm channel when a connection to host is closed.
 */
extern void _bsp_dbg_connect_abort(void);
  

/*
 * Pointer to a network channel. NULL if no network channel
 * exists.
 */
extern struct bsp_comm_channel *_bsp_net_channel;


/*
 * Formatted output primitive.
 */
extern void __vprintf(void (*putc_func)(char c), const char *fmt0, va_list ap);

typedef struct bp {
    struct bp     *next;
    void          *address;
    unsigned long old_inst;
    unsigned int  flags;
#define BP_IN_MEMORY 1
} bp_t;

/* Generic breakpoint support */

extern void bsp_init_breakpoints(void);
extern void bsp_install_all_breakpoints(void);
extern void bsp_uninstall_all_breakpoints(void);
/* Return zero if sucessful, non-zero error code. */
extern int  bsp_add_breakpoint (void *location, unsigned long length);
extern int  bsp_remove_breakpoint (void *location, unsigned long length);

/* Architecture specific routines for setting/clearing breakpoints */
/* Return zero if sucessful, non-zero error code. */
extern int _bsp_set_breakpoint(bp_t *bp);
extern int _bsp_clear_breakpoint(bp_t *bp);


#endif /* !__ASSEMBLER__ */

/*
 * SYSCALL number to use to get pointer to above bsp_shared_t structure.
 */
#define BSP_GET_SHARED  0xbaad

#endif  /* __BSP_IF_H__ */


