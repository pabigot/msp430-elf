/*
 * sysinfo.c -- Interface for getting system information.
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

#include <stdlib.h>
#include <bsp/bsp.h>
#include "bsp_if.h"


/*
 * In order to construct the _bsp_memory_list, some board specific code
 * may have to size RAM regions. To do this easily and reliably, the code
 * needs to run from ROM before .bss and .data sections are initialized.
 * This leads to the problem of where to store the results of the memory
 * sizing tests. In this case, the _bsp_init_stack routine which sizes
 * memory and sets up the stack will place the board-specific information
 * on the stack and return with the stack pointer pointing to a pointer to
 * the information. That is, addr_of_info = *(void **)sp. The architecture
 * specific code will then copy that pointer to the _bsp_ram_info_ptr variable
 * after initializing the .data and .bss sections.
 */
void *_bsp_ram_info_ptr;

/*
 *  Name of CPU and board. Should be overridden by arch/board specific
 *  code.
 */
struct bsp_platform_info _bsp_platform_info = {
    "Unknown",  /* cpu name */
    "Unknown",  /* board name */
    ""          /* extra info */
};


/*
 *  Information about possible data cache. Should be overridden by
 *  by arch/board specific code.
 */
struct bsp_cachesize_info _bsp_dcache_info = {
    0, 0, 0
};


/*
 *  Information about possible instruction cache. Should be overridden by
 *  by arch/board specific code.
 */
struct bsp_cachesize_info _bsp_icache_info = {
    0, 0, 0
};


/*
 *  Information about possible secondary cache. Should be overridden by
 *  by arch/board specific code.
 */
struct bsp_cachesize_info _bsp_scache_info = {
    0, 0, 0
};



int
_bsp_sysinfo(enum bsp_info_id id, va_list ap)
{
    int  index, rval = 0;
    void *p;

    switch (id) {
      case BSP_INFO_PLATFORM:
	p = va_arg(ap, void *);
	*(struct bsp_platform_info *)p = _bsp_platform_info;
	break;

      case BSP_INFO_DCACHE:
	p = va_arg(ap, void *);
	*(struct bsp_cachesize_info *)p = _bsp_dcache_info;
	break;

      case BSP_INFO_ICACHE:
	p = va_arg(ap, void *);
	*(struct bsp_cachesize_info *)p = _bsp_icache_info;
	break;

      case BSP_INFO_SCACHE:
	p = va_arg(ap, void *);
	*(struct bsp_cachesize_info *)p = _bsp_scache_info;
	break;

      case BSP_INFO_MEMORY:
	index = va_arg(ap, int);
	p = va_arg(ap, void *);

	if (index >= 0 && index < _bsp_num_mem_regions)
	    *(struct bsp_mem_info *)p = _bsp_memory_list[index];
	else
	    rval = -1;
	break;

      case BSP_INFO_COMM:
	index = va_arg(ap, int);
	p = va_arg(ap, void *);

	if (index >= 0 && index < _bsp_num_comms)
	    *(struct bsp_comm_info *)p = _bsp_comm_list[index].info;
	else if (index == _bsp_num_comms && _bsp_net_channel != NULL)
	    *(struct bsp_comm_info *)p = _bsp_net_channel->info;
	else
	    rval = -1;
	break;

      default:
	rval =  -1;
    }

    return rval;
}



