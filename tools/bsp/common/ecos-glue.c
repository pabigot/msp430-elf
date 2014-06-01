/*
 * ecos-glue.c -- Interface glue between bsp and eCos.
 *
 * Copyright (c) 2000 Red Hat Inc.
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
#include <bsp_if.h>
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>
#include <cyg/io/config_keys.h>
#include <cyg/hal/hal_cache.h>

#define __MAX_COMM_CHANNELS__ 4

/* This must be a sprintf format string with exactly one %d format
   which will be replaced with 0...(MAX_COMM_CHANNELS-1) */
#define __COMM_NAME_FMT__   "/dev/ser%d"


static cyg_io_handle_t _handles[__MAX_COMM_CHANNELS__];
static char _names[__MAX_COMM_CHANNELS__][sizeof(__COMM_NAME_FMT__)];

struct bsp_comm_channel _bsp_comm_list[__MAX_COMM_CHANNELS__];
int _bsp_num_comms;


static void 
uart_putchar(void *p, char c)
{
    int len = 1;
    cyg_io_write(*(cyg_io_handle_t *)p, &c, &len);
}

static void
uart_write(void *p, const char *buf, int len)
{
    cyg_io_write(*(cyg_io_handle_t *)p, buf, &len);
}

static int
uart_read(void *p, char *buf, int len)
{
    cyg_io_read(*(cyg_io_handle_t *)p, buf, &len);
    return len;
}

static int
uart_getchar(void *p)
{
    char ch = '\0';
    int len = 1;

    cyg_io_read(*(cyg_io_handle_t *)p, &ch, &len);
    return ch;
}

static int
uart_control(void *p, int func, ...)
{
    int             i, retval = 0;
    cyg_io_handle_t chan = *(cyg_io_handle_t *)p;
    cyg_serial_info_t info;
    va_list         ap;
    cyg_uint32      len = 0;

    va_start (ap, func);

    switch (func) {
      case COMMCTL_SETBAUD:
	i = va_arg(ap, int);
	len = sizeof(info);
	cyg_io_get_config(chan, CYG_IO_GET_CONFIG_SERIAL_INFO, &info, &len);
	info.baud = i;
	if (cyg_io_set_config(chan, CYG_IO_SET_CONFIG_SERIAL_INFO,
			      &info, &len) != ENOERR)
	    retval = -1;
	break;

      case COMMCTL_GETBAUD:
	len = sizeof(info);
	cyg_io_get_config(chan, CYG_IO_GET_CONFIG_SERIAL_INFO, &info, &len);
	retval = info.baud;
	break;

      case COMMCTL_INSTALL_DBG_ISR:
	cyg_io_set_config(chan, CYG_IO_SET_CONFIG_ISR_INSTALL, NULL, NULL);
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	cyg_io_set_config(chan, CYG_IO_SET_CONFIG_ISR_REMOVE, NULL, NULL);
	break;

      case COMMCTL_IRQ_DISABLE:
	cyg_io_set_config(chan, CYG_IO_SET_CONFIG_IRQ_DISABLE, &retval, &len);
	break;

      case COMMCTL_IRQ_ENABLE:
	cyg_io_set_config(chan, CYG_IO_SET_CONFIG_IRQ_ENABLE, NULL, NULL);
	break;

      default:
	retval = -1;
	break;
    }

    va_end(ap);
    return retval;
}

void
_bsp_init_board_comm(void)
{
    int i;
    struct bsp_comm_channel *p;

    for (i = 0; i < __MAX_COMM_CHANNELS__; i++) {
	bsp_sprintf(_names[i], __COMM_NAME_FMT__, i);
        if (cyg_io_lookup(_names[i], &_handles[i]) != ENOERR)
	    break;
	
	p = &_bsp_comm_list[i];
	p->info.name = _names[i];
	p->info.kind = BSP_COMM_SERIAL;
	p->info.protocol = BSP_PROTO_NONE;
	p->procs.ch_data = (void *)&_handles[i];
	p->procs.__write = uart_write;
	p->procs.__read = uart_read;
	p->procs.__putc = uart_putchar;
	p->procs.__getc = uart_getchar;
	p->procs.__control = uart_control;
    }
    _bsp_num_comms = i;
}


/*
 * Flush Icache for given range. (Entire cache for now)
 */
void
__icache_flush(void *addr, int nbytes)
{
    HAL_ICACHE_INVALIDATE_ALL();
}

/*
 * Flush Dcache for given range. (Entire cache for now)
 */
void
__dcache_flush(void *addr, int nbytes)
{
    HAL_DCACHE_SYNC();
    HAL_DCACHE_INVALIDATE_ALL();
}


/*
 * Array of memory region descriptors. We just list RAM and FLASH.
 */
struct bsp_mem_info _bsp_memory_list[] = 
{
    { (void *)0, (void *)0x00000000, 0, 0, BSP_MEM_RAM }
};

/*
 * Number of memory region descriptors.
 */
int _bsp_num_mem_regions = 1;
