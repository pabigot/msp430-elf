/*
 * main.c -- Default BSP ROM program. Just causes breakpoint exception so
 *           debug stub can take control.
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

#include <bsp/bsp.h>
static void
print_banner(void)
{
    struct bsp_platform_info platform;
    struct bsp_mem_info      mem;
    int                      i;
    unsigned long            u, totmem, topmem;

    bsp_sysinfo(BSP_INFO_PLATFORM, &platform);
    bsp_printf("Cygnus BSP\n");
    bsp_printf("CPU: %s\n", platform.cpu);
    bsp_printf("Board: %s\n", platform.board);
    if (*(platform.extra))
        bsp_printf("%s\n", platform.extra);
    totmem = topmem = 0;
    i = 0;
    while (bsp_sysinfo(BSP_INFO_MEMORY, i++, &mem) == 0) {
	if (mem.kind == BSP_MEM_RAM) {
	    totmem += mem.nbytes;
	    u = (unsigned long)mem.virt_start + mem.nbytes;
	    if (u > topmem)
		topmem = u;
	}
    }
    
    bsp_printf("Total RAM: %d bytes\n", totmem);
    bsp_printf("Top of RAM: 0x%x\n", topmem);
}

#ifdef __ECOS_HAL__
void
cyg_start(void)
#else
int
main(int argc, char *argv[], char *envp[])
#endif
{
    int  cur_port;
    struct bsp_comm_info comm_info;

#ifdef __ECOS_HAL__
    _bsp_init();
#endif

    /*bsp_set_console_comm(1);*/
    print_banner();

    /* get info on debug channel */
    cur_port = bsp_set_debug_comm(-1);
    bsp_sysinfo(BSP_INFO_COMM, cur_port, &comm_info);

    /*
     * If we're using a network interface, lower level
     * code setup a serial console driver. Here, we
     * set the console to the network interface so that
     * program output will go to the debugger console on
     * the host.
     */
    if (comm_info.kind == BSP_COMM_ENET)
	bsp_set_console_comm(cur_port);

    while (1) {
	bsp_breakpoint();
    }

    return 0;
}
