/*
 * bsp-start.h -- BSP startcode definitions
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
#ifndef __BSP_START_H__
#define __BSP_START_H__ 1

#include <bsp/cpu.h>
#include "bsp-trap.h"
#include "bsp_if.h"
#include <stdlib.h>

#ifdef ADD_DTORS
extern void          (*__do_global_dtors)(void);
#endif /* ADD_DTORS */

extern int           main(int argc, char *argv[], char *envp[]);

extern char _rom_data_start[];
extern char _ram_data_start[];
extern char _ram_data_end[];
extern char _bss_start[];
extern char _bss_end[];

extern void          *_bsp_ram_info_ptr;

#ifndef NO_INIT_FUNCTION
extern void INIT_FUNC(void);
extern void FINI_FUNC(void);
#endif /* NO_INIT_FUNCTION */

/*
 * Shared app and bsp startup code.
 * This gets run before any app or bsp specific code gets run
 */
static inline void c_crt0_begin(void)
{
#if !defined(__SIM_LOADS_DATA__)
    /*
     * Copy initialized data from ROM to RAM if necessary
     */
    if (&_ram_data_start != &_rom_data_start)
    {
        unsigned long *ram_data = (unsigned long *)&_ram_data_start;
        unsigned long *rom_data = (unsigned long *)&_rom_data_start;
        while (ram_data < (unsigned long *)&_ram_data_end)
            *(ram_data++) = *(rom_data++);
    }
#endif

    /*
     * Zero initialize the bss section
     */
    if (&_bss_start != &_bss_end)
    {
        unsigned long *bss_data = (unsigned long *)&_bss_start;
        while (bss_data < (unsigned long *)&_bss_end)
            *(bss_data++) = 0;
    }

#ifdef INIT_FUNC
    INIT_FUNC();
#endif
#ifdef FINI_FUNC
    atexit(FINI_FUNC);
#endif

    /*
     * put __do_global_dtors in the atexit list so the destructors get run
     */
#ifdef ADD_DTORS
    atexit(__do_global_dtors);
#endif /* ADD_DTORS */
}

/*
 * Shared app and bsp startup code.
 * This gets run after all app or bsp specific code gets run
 */
static inline void c_crt0_end(void)
{
    static char *nullstring = "\0";

    /*
     * Enable interrupts.
     * FIXME: Deprecate __cli() macro in favor of BSP_ENABLE_INTERRUPTS.
     */
#if defined(BSP_ENABLE_INTERRUPTS)
   BSP_ENABLE_INTERRUPTS();
#else
   __cli();
#endif

   exit(main(0, &nullstring, &nullstring));

    /*
     * Never reached, but just in case...
     */
    while (1) ;
}

/*
 * Application startup code.  This is not run by the BSP start file
 */
static inline void c_crt0_app_specific()
{
    _bsp_trap(BSP_GET_SHARED, &bsp_shared_data);
}

/*
 * BSP startup code.  This is not run by the Application start file
 */
static inline void c_crt0_bsp_specific(void *config_data)
{
    /*
     * Finish setup of RAM description. Put a
     * physical pointer to the top of RAM in _bsp_ram_info_ptr.
     */
    _bsp_ram_info_ptr = config_data;
    
    /*
     * Do the generic BSP initialization
     */
    _bsp_init();
}
#endif /* __BSP_START_H__ */
