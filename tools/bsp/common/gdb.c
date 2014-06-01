/*
 * gdb.c -- Generic support for target gdb stub.
 *
 */

/****************************************************************************

		THIS SOFTWARE IS NOT COPYRIGHTED

   HP offers the following for use in the public domain.  HP makes no
   warranty with regard to the software or it's performance and the
   user accepts the software "AS IS" with all faults.

   HP DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD
   TO THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

****************************************************************************/

/****************************************************************************
 *  Header: remcom.c,v 1.34 91/03/09 12:29:49 glenne Exp $
 *
 *  Module name: remcom.c $
 *  Revision: 1.34 $
 *  Date: 91/03/09 12:29:49 $
 *  Contributor:     Lake Stevens Instrument Division$
 *
 *  Description:     low level support for gdb debugger. $
 *
 *  Considerations:  only works on target hardware $
 *
 *  Written by:      Glenn Engel $
 *  ModuleState:     Experimental $
 *
 *  NOTES:           See Below $
 *
 *************
 *
 *    The following gdb commands are supported:
 *
 * command          function                               Return value
 *
 *    g             return the value of the CPU registers  hex data or ENN
 *    G             set the value of the CPU registers     OK or ENN
 *
 *    mAA..AA,LLLL  Read LLLL bytes at address AA..AA      hex data or ENN
 *    MAA..AA,LLLL: Write LLLL bytes at address AA.AA      OK or ENN
 *
 *    c             Resume at current address              SNN   ( signal NN)
 *    cAA..AA       Continue at address AA..AA             SNN
 *
 *    s             Step one instruction                   SNN
 *    sAA..AA       Step one instruction from AA..AA       SNN
 *
 *    k             kill
 *
 *    ?             What was the last sigval ?             SNN   (signal NN)
 *
 *    bBB..BB	    Set baud rate to BB..BB		   OK or BNN, then sets
 *							   baud rate
 *
 * All commands and responses are sent with a packet which includes a
 * checksum.  A packet consists of
 *
 * $<packet info>#<checksum>.
 *
 * where
 * <packet info> :: <characters representing the command or response>
 * <checksum>    :: < two hex digits computed as modulo 256 sum of <packetinfo>>
 *
 * When a packet is received, it is first acknowledged with either '+' or '-'.
 * '+' indicates a successful transfer.  '-' indicates a failed transfer.
 *
 * Example:
 *
 * Host:                  Reply:
 * $m0,10#2a               +$00010203040506070809101112131415#42
 *
 ****************************************************************************/

#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <stdarg.h>
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include "bsp_if.h"
#include "gdb.h"
#include "gdb-cpu.h"
#include "bsplog.h"
#include <bsp/hex-utils.h>

#ifdef GDB_THREAD_SUPPORT
#include "gdb-threads.h"
#endif

#undef DEBUG_STUB
#define DEBUG_STUB 0

/*
 * Defining DELAYED_ACK causes the stub to delay ack'ing a received packet in
 * the hope of combining the ack with a response packet. This doesn't make any
 * difference for serial channels, but it gives a tremendous performace boost
 * to downloads over a tcp channel. This is due to the simple tcp stack which
 * has a dedicated tx packet which must be ack'ed before the next packet is
 * sent. A 'real' stack on the host side will delay several 10's of millisecs
 * before acking in the hopes of combining that ack with a data packet. Since
 * the gdb protocol guarantees no other data will be forthcoming, we save
 * those milliseconds by combining our remote protocol ack with the remote
 * protocol response.
 */
#define DELAYED_ACK 1

/* BUFMAX defines the maximum number of characters in inbound/outbound buffers*/
/* at least NUMREGBYTES*2 are needed for register packets */
#define BUFMAX 2048

/*
 *  jump buffer used for longjumping out of stub-caused
 *  exceptions.
 */
static jmp_buf gdb_jmpbuf;

void (*_bsp_kill_hook)(void) = NULL;

static int  gdb_write_hook(char *buf, int len);
static int  gdb_read_hook(char *buf, int len);


gdb_data_t _bsp_gdb_data = {
    NULL, /* __mem_read_hook  */
    NULL, /* __mem_write_hook */
    NULL, /* __reg_get_hook   */
    NULL, /* __reg_set_hook   */
    NULL, /* __pkt_query_hook */
    NULL, /* __pkt_set_hook   */
    NULL, /* __pkt_hook       */
    _gdb_pkt_append,
    gdb_read_hook,
    gdb_write_hook
};


/*
 *  Register storage area to save original context so that the proper
 *  context can be restored when a program is killed by the debugger.
 *
 *  NB: We could save space by having architecture specific code do
 *      the save and restore using a minimal set of regs.
 */
static ex_regs_t orig_context;

#ifdef GDB_THREAD_SUPPORT
int _gdb_cont_thread;
int _gdb_general_thread;

ex_regs_t _gdb_general_registers;
#endif

/*
 *  True if in gdb handler.
 */
static volatile short in_gdb_handler;
static volatile short gdb_connect_closed; /* true if connection was closed */

static char output_pkt_buf[BUFMAX];
static char *output_pkt_ptr;

static void process_query(unsigned char *pkt, void *regs);
static void process_set(unsigned char *pkt, void *regs);


/*
 * Underlying comm channel calls this when a comm connection to host is closed.
 */
void
_bsp_dbg_connect_abort(void)
{
    BSPLOG(bsp_log("_bsp_dbg_connect_abort: in_handler[%d]\n",
		   in_gdb_handler));
    gdb_connect_closed = 1;
    /* if called as part of stub polling, then just longjmp out. */
    if (in_gdb_handler)
	longjmp(gdb_jmpbuf, 1);
}

static char   input_buffer[BUFMAX];

/*
 *  Get a packet.
 */
static unsigned char *
pkt_receive(void)
{
    unsigned char checksum;
    unsigned char xmitcsum;
    int  count;
    char ch;
  
    while (1) {
	/* wait around for the start character, ignore all other characters */
	while ((ch = bsp_debug_getc()) != '$')
            ;
	checksum = 0;
	xmitcsum = -1;
	count = 0;
    
	/* now, read until a # or end of buffer is found */
	while (count < (BUFMAX - 1)) {
	    ch = bsp_debug_getc();
	    if (ch == '#')
		break;
	    checksum = checksum + ch;
	    input_buffer[count] = ch;
	    count = count + 1;
	}
	input_buffer[count] = 0;

	if (ch == '#') {
	    ch = bsp_debug_getc();
	    xmitcsum = __hex(ch) << 4;
	    ch = bsp_debug_getc();
	    xmitcsum += __hex(ch);

	    if (checksum != xmitcsum) {
		bsp_debug_write("-", 1);  /* failed checksum */ 
#if DEBUG_STUB
		bsp_printf("\nbadpkt: [%s]\n", input_buffer);
#endif
	    } else {
#if !defined(DELAYED_ACK)
		int  i;
		char reply[4];

		reply[0] = '+';
		/* if a sequence char is present, reply the sequence ID */
		if (input_buffer[2] == ':') {
		    reply[1] = input_buffer[0];
		    reply[2] = input_buffer[1];
		    bsp_debug_write(reply, 3);

		    /* remove sequence chars from buffer */
		    count = strlen(input_buffer);
		    for (i=3; i <= count; i++)
			input_buffer[i-3] = input_buffer[i];
		} else
		    bsp_debug_write(reply, 1);
#endif
#if DEBUG_STUB
		bsp_printf("rpkt: [%s]\n", input_buffer);
#endif
#ifdef BSP_LOG_PACKETS
		if (input_buffer[0] == 'X') {
		    char ctmp;
		    int  itmp;

		    for (itmp = 0; input_buffer[itmp]; ++itmp)
			if (input_buffer[itmp] == ':')
			    break;

		    ctmp = input_buffer[itmp];
		    input_buffer[itmp] = '\0';
		    BSPLOG(bsp_log("rpkt: [%s]\n", input_buffer));
		    input_buffer[itmp] = ctmp;
		} else {
		    BSPLOG(bsp_log("rpkt: [%s]\n", input_buffer));
		}
#endif
		return input_buffer;
	    } 
	} else 
	    bsp_debug_write("-", 1);
    }
}

void
_gdb_pkt_start(int ack)
{
    output_pkt_ptr = &output_pkt_buf[0];
#if defined(DELAYED_ACK)
    if (ack)
	*output_pkt_ptr++ = '+';
#endif
    *output_pkt_ptr++ = '$';
}

/*
 *  Append data to packet using formatted string.
 */
void
_gdb_pkt_append(char *fmt, ...)
{
    va_list ap;
    int     n;

    va_start(ap, fmt);
    n = bsp_vsprintf(output_pkt_ptr, fmt, ap);
    va_end(ap);
    output_pkt_ptr += n;
}

/*
 *  Calculate checksum and append to end of packet.
 */
void
_gdb_pkt_end(void)
{
    unsigned char cksum = 0;
    char          *p;

    p = output_pkt_buf;
    while (*p++ != '$')
	;

    while (p < output_pkt_ptr)
	cksum += *p++;

    _gdb_pkt_append("#%02x", cksum);
}


void
_gdb_pkt_send(void)
{
    int ch, was_enabled = 0, interrupted = 0;

    if (!in_gdb_handler)
	was_enabled = bsp_debug_irq_disable();

    do {
#if DEBUG_STUB
	*output_pkt_ptr = '\0';
	bsp_printf("wpkt[%s]\n", output_pkt_buf+1);
#endif
#ifdef BSP_LOG_PACKETS
	*output_pkt_ptr = '\0';
	BSPLOG(bsp_log("wpkt[%s]\n", output_pkt_buf+1));
#endif

	bsp_debug_write(output_pkt_buf, output_pkt_ptr - output_pkt_buf);

	while ((ch = bsp_debug_getc()) == '\003')
	    interrupted = 1;

#if DEBUG_STUB
	if (ch != '+') {
	    bsp_printf("bad ack: %c [0x%02x]\n", ch, ch);
	}
#endif
    } while (ch != '+' && !interrupted);

    if (interrupted && !in_gdb_handler)
	bsp_shared_data->__console_interrupt_flag = 1;

    if (was_enabled)
	bsp_debug_irq_enable();

#ifdef BSP_LOG_PACKETS
	BSPLOG(bsp_log("wpkt ack'd\n"));
#endif

#if 0
#if DEBUG_STUB
    bsp_printf("wpkt ack'd\n");
#endif
#endif
}


/*
 *  Max number of converted ascii characters. This is 1/2 BUFMAX
 *  less 2 for the leading 'O' and trailing '\0'.
 */
#define MAXWRITE (BUFMAX/2 - 2)


/*
 *  Send program output to gdb console.
 */
static void
gdb_write(void *unused, const char *buf, int len)
{
    int i;

    BSPLOG(bsp_log("gdb_write: len[%d]\n", len));
    while (len > 0) {
	_gdb_pkt_start(0);
	_gdb_pkt_append("O");
	for (i = 0; len > 0 && i < (MAXWRITE-2); i++, len--)
	    _gdb_pkt_append("%02x", *buf++);
	_gdb_pkt_end();
	_gdb_pkt_send();
    }
    BSPLOG(bsp_log("gdb_write: done\n"));
}


static void
gdb_putc(void *unused, const char ch)
{
    gdb_write(NULL, &ch, 1);
}


static int
gdb_read(void *unused, char *buf, int len)
{
    int i = 0;

    for (i = 0; i < len; i++) {
	*(buf + i) = bsp_debug_getc();
	if ((*(buf + i) == '\n') || (*(buf + i) == '\r')) {
	    (*(buf + i + 1)) = 0;
	    break;
	}
    }
    return (i);
}


static int
gdb_getc(void *unused)
{
    return bsp_debug_getc();
}


/*
 * If the BSP client doesn't use a separate comm channel
 * for console output, gdb will install its own i/o routinese
 * to read (not done yet) and write program stdio.
 */
static struct bsp_comm_procs gdb_console_procs = {
    NULL,
    gdb_write,
    gdb_read,
    gdb_putc,
    gdb_getc,
    NULL
};


/*
 * Access to read/write functions through gdb shared data structure.
 */
static int gdb_write_hook(char *buf, int len)
{
    if (in_gdb_handler) 
	bsp_debug_write("+", 1);

    gdb_write(NULL, buf, len);
    _gdb_pkt_start(0);

    return len;
}

static int gdb_read_hook(char *buf, int len)
{
    return gdb_read(NULL, buf, len);
}


/*
 * This routine is called from the debug channel interface when
 * an RX interrupt occurs. It reads a buffer, looks for a 
 * control-C and returns true if one is found.
 */
int
gdb_interrupt_check(void)
{
    int nread, i;
    unsigned char *p;

    BSPLOG(bsp_log("gdb_interrupt_check\n"));
    nread = bsp_debug_read(input_buffer, sizeof(input_buffer));
    
    if (gdb_connect_closed) {
	BSPLOG(bsp_log("gdb_interrupt_check: connect closed\n"));
	gdb_connect_closed = 0;
	return 1;
    }

    for (i = 0, p = input_buffer; i < nread; i++)
	if (*p++ == '\003') {
 	    BSPLOG(bsp_log("gdb_interrupt_check: found ^C\n"));
	    return 1;
	}

    BSPLOG(bsp_log("gdb_interrupt_check: done\n"));
    return 0;
}


#ifdef GDB_THREAD_SUPPORT
/*
 * Kernel Thread Control
 *
 * If the current thread is set to other than zero (or minus one),
 * then ask the kernel to lock it's scheduler so that only that thread
 * can run.
 */
static unsigned char did_lock_scheduler = 0;
static unsigned char did_disable_interrupts = 0;

static void
lock_thread_scheduler (int kind, void *regs)	/* "step" or "continue" */
{
    int ret = 0;

    /* GDB will signal its desire to run a single thread
       by setting _gdb_cont_thread to non-zero / non-negative.  */
    if (_gdb_cont_thread <= 0)
	return;

    ret = _gdb_lock_scheduler(1, kind, _gdb_cont_thread);

    if (ret == 1) {
	did_lock_scheduler = 1;
	return;
    }

    if (ret == -1) {
	/* no kernel or kernel wants stub to handle it */
	if (bsp_set_interrupt_enable(1, regs)) {
	    did_disable_interrupts = 1;
	    return;
	}
    }
}


static void
unlock_thread_scheduler (void *regs)
{
    if (did_lock_scheduler) {
	_gdb_lock_scheduler(0, 0, _gdb_cont_thread);
	/* I could check the return value, but 
	   what would I do if it failed???  */
	did_lock_scheduler = 0;
    }

    if (did_disable_interrupts) {
	bsp_set_interrupt_enable(0, regs);
	/* Again, I could check the return value, but 
	   what would I do if it failed???  */
	did_disable_interrupts = 0;
    }
}
#endif


static int
__memory_read(void *addr,    /* start addr of memory to read */
	      int  asid,     /* address space id */
	      int  rsize,    /* size of individual read ops */
	      int  nreads,   /* number of read operations */
	      void *buf)     /* result buffer */
{
    if (_bsp_gdb_data.__mem_read_hook)
	return _bsp_gdb_data.__mem_read_hook(addr, asid, rsize, nreads, buf);
    return bsp_memory_read(addr, asid, rsize, nreads, buf);
}

static int
__memory_write(void *addr,   /* start addr of memory to write */
	       int  asid,    /* address space id */
	       int  wsize,   /* size of individual write ops */
	       int  nwrites, /* number of write operations */
	       void *buf)    /* source buffer for write data */
{
    if (_bsp_gdb_data.__mem_write_hook)
	return _bsp_gdb_data.__mem_write_hook(addr, asid, wsize, nwrites, buf);
    return bsp_memory_write(addr, asid, wsize, nwrites, buf);
}


static void
__register_get(int regno,    /* id of register value to read */
	       void *regs,   /* pointer to saved registers   */
	       void *val)    /* where to put register value  */
{
    if (_bsp_gdb_data.__reg_get_hook)
	_bsp_gdb_data.__reg_get_hook(regno, regs, val);
    else
	bsp_get_register(regno, regs, val);
}

static void
__register_set(int regno,    /* id of register value to set */
	       void *regs,   /* pointer to saved registers   */
	       void *val)    /* where to get register value  */
{
    if (_bsp_gdb_data.__reg_set_hook)
	_bsp_gdb_data.__reg_set_hook(regno, regs, val);
    else
	bsp_set_register(regno, regs, val);
}


static void
send_stopped_packet (int sigval, void *regs)
{
    _gdb_pkt_start(0);
#ifdef IS_GDB_T_REG
    {
	int i,j;
	void *cur_regs;
	unsigned char buf[16];
	
	_gdb_pkt_append("T%02x", sigval);

#ifdef GDB_THREAD_SUPPORT
	if ((i = _gdb_get_currthread()) != 0)
	    _gdb_pkt_append("thread:%x;", i);

	if (_gdb_general_thread != 0 && _gdb_general_thread != -1)
	    cur_regs = &_gdb_general_registers;
	else
#endif
	    cur_regs = regs;

	for (i = 0; i < NUMREGS; i++) {
	    if (IS_GDB_T_REG(i)) {
		_gdb_pkt_append("%02x:", i);
		__register_get(i, cur_regs, buf);
		for (j = 0; j < bsp_regsize(i); j++)
		    _gdb_pkt_append("%02x", buf[j]);
		_gdb_pkt_append(";");
	    }
	}
    }
#else
    _gdb_pkt_append("S%02x", sigval);
#endif
    _gdb_pkt_end();
    _gdb_pkt_send();
}

/*
 * This function does all command processing for interfacing to gdb.
 */
void
_bsp_gdb_handler(int exc_nr, void *regs)
{
    int           i, j, sigval = 0, is_binary;
    unsigned char *ptr;
    unsigned long addr, length, cur_pc;
    unsigned char ch, buf[512];
    static short  first_time = 1, continuing = 0, stepping = 0;
    void          *cur_regs;
#if BSP_MAX_BP > 0
    unsigned long ztype;
#endif

    if (first_time) {
	memcpy(&orig_context, regs, sizeof(ex_regs_t));
#if BSP_MAX_BP > 0
	bsp_init_breakpoints();
#endif
	first_time = 0;
    }

    /*
     * Check to see if the stub had previously replaced the console funcs
     * with debug protocol specific ones. If so, set them back to null
     * while the user program has stopped.
     */
    if (bsp_shared_data->__console_procs == &gdb_console_procs)
	bsp_shared_data->__console_procs = NULL;

#if DEBUG_STUB
    cur_pc = bsp_get_pc(regs);

    bsp_printf("gdb: exc_nr<%d> pc<0x%lx> stub SP<0x%lx> in handler<%d>\n",
		exc_nr, cur_pc, (unsigned long)&regs, in_gdb_handler);
#endif

    if (in_gdb_handler) {
	longjmp(gdb_jmpbuf, 1);
    }

    in_gdb_handler = 1;

#ifdef GDB_THREAD_SUPPORT
    _gdb_general_thread = -1;

    /* Undo effect of previous single step.  */
    unlock_thread_scheduler(regs);
#endif

    bsp_singlestep_cleanup(regs);

#if BSP_MAX_BP > 0
    bsp_uninstall_all_breakpoints();
#endif

    if (gdb_connect_closed)
	goto do_kill;

    /*
     *  Let host know why we stopped.
     */
    _gdb_pkt_start(0);
    if (setjmp(gdb_jmpbuf) == 0) {
        sigval = bsp_get_signal(exc_nr, regs);
 
	cur_pc = bsp_get_pc(regs);
	if (cur_pc == (unsigned long)bsp_breakinsn) {
	    bsp_skip_instruction(regs);
	    /*
	     * This prevents a hang if we 'continue' before a
	     * 'load' has been done.
	     */
	    if (continuing || stepping)
		send_stopped_packet(sigval, regs);
	} else {
	    /* let host know that a program exception has occurred */
	    send_stopped_packet(sigval, regs);
	}
    } else {
        sigval = bsp_get_signal(exc_nr, regs);

	if (gdb_connect_closed) {
	    /* got here because of comm channel connection close */
	    goto do_kill;
	}
	/* let host know that stub caused an exception. */
	_gdb_pkt_start(1);
	_gdb_pkt_append("E03");
	cur_pc = bsp_get_pc(regs); /* in case longjmp clobbered it */
	_gdb_pkt_end();
	_gdb_pkt_send();
    }

    stepping = continuing = 0;

    /*
     *  Main gdb stub loop. Read a packet and act accordingly.
     */
    while (1) {
	is_binary = 0;  /* assume mem write data is hex ascii */

	/* Get a command packet from host */
	ptr = pkt_receive();

	/* Start forming a response packet. */
	_gdb_pkt_start(1);

	switch (*ptr++) {

	  case '?':
	    /* Return last known signal. */
	    _gdb_pkt_append("S%02x", sigval);
	    break; 

	  case 'g':
	    /* return the value of all CPU registers which GDB knows about */
#ifdef GDB_THREAD_SUPPORT
	    if (_gdb_general_thread != 0 && _gdb_general_thread != -1)
		cur_regs = &_gdb_general_registers;
	    else
#endif
		cur_regs = regs;
	    for (i = 0; i < NUMREGS; i++) {
		__register_get(i, cur_regs, buf);
		for (j = 0; j < bsp_regsize(i); j++)
		    _gdb_pkt_append("%02x", buf[j]);
	    }
	    break;

	  case 'G':
	    /* set the value of all registers */
#ifdef GDB_THREAD_SUPPORT
	    if (_gdb_general_thread != 0 && _gdb_general_thread != -1)
		cur_regs = &_gdb_general_registers;
	    else
#endif
		cur_regs = regs;
	    for (i = 0; i < NUMREGS; i++) {
		__unpack_bytes_to_mem(ptr, buf, bsp_regsize(i));
		ptr += bsp_regsize(i) * 2;
		__register_set(i, cur_regs, buf);
	    }
	    _gdb_pkt_append("OK");
	    break;
      
	  case 'P':
	    /* set the value of a single CPU register */
#ifdef GDB_THREAD_SUPPORT
	    if (_gdb_general_thread != 0 && _gdb_general_thread != -1)
		cur_regs = &_gdb_general_registers;
	    else
#endif
		cur_regs = regs;
	    if (__unpack_ulong((char **)&ptr, &addr)) {
		if (*(ptr++) == '=') {
		    __unpack_bytes_to_mem(ptr, buf, bsp_regsize(addr));
		    __register_set(addr, cur_regs, buf);
		    _gdb_pkt_append("OK");
		    break;
		}
	    }
	    _gdb_pkt_append("E01");
	    break;
      
	  case 'm': 
	    /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
	    if (__unpack_ulong((char **)&ptr, &addr)) {
		if (*(ptr++) == ',') {
		    if (__unpack_ulong((char **)&ptr, &length)) {
			ptr = (unsigned char *)addr;
			for (i = 0; i < length; i++, ptr++) {
			    if (!__memory_read(ptr, -1, 8, 1, &ch))
				break;
			    _gdb_pkt_append("%02x", ch);
			}
			if (i < length) {
			    _gdb_pkt_start(1);
			    _gdb_pkt_append("E03");
			}
			break;
		    }
		}
	    }
	    _gdb_pkt_append("E01");
	    break;
      
	  case 'X':
	    /* XAA..AA,LLLL: Write LLLL escaped binary bytes at address AA.AA */
	    is_binary = 1;
	    /* fall through */
	  case 'M':
	    /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
	    if (__unpack_ulong((char **)&ptr, &addr)) {
		if (*(ptr++) == ',') {
		    if (__unpack_ulong((char **)&ptr, &length)) {
			if (*(ptr++) == ':') {
			    /* now write the data */
			    do {
				if (is_binary) {
				    for (j = 0; j < sizeof(buf) && j < length; j++)
					if ((buf[j] = *ptr++) == 0x7d)
					    buf[j] = 0x20 + *ptr++;
				} else {
				    if (length <= sizeof(buf))
					j = length;
				    else
					j = sizeof(buf);
				    __unpack_bytes_to_mem(ptr, buf, j);
				    ptr += (j<<1);
				}

				i = __memory_write((void*)addr, -1, 8, j, buf);

				/* flush icache and dcache, now */
				bsp_flush_dcache((void *)addr, i);
				bsp_flush_icache((void *)addr, i);

				if (i < j)
				    break;

				addr += i;
				length -= i;
			    } while (length);

			    if (length) {
				_gdb_pkt_start(1);
				_gdb_pkt_append("E03");
			    } else
				_gdb_pkt_append("OK");
			    break;
			}
		    }
		}
	    }
	    _gdb_pkt_append("E02");
	    break;
     
	  case 'S':
	    /* SXX;AA..AA Step from AA..AA(optional) with signal XX */
	  case 's':
	    /* sAA..AA   Step one instruction from AA..AA(optional) */
	    stepping = 1;
	    /* fall through */
	  case 'C':
	    /* CXX;AA..AA Continue at addr AA..AA(optional) with signal XX */
	  case 'c':
	    /* cAA..AA    Continue at addr AA..AA(optional) */

	    /*
	     * read signal if necessary.
	     */
	    if (ptr[-1] == 'C' || ptr[-1] == 'S') {
		/* just ignore signal */
		__unpack_ulong((char **)&ptr, &addr);
		if (*ptr == ';')
		    ++ptr;
	    }

	    /* look for optional address */
	    if (__unpack_ulong((char **)&ptr, &addr))
		bsp_set_pc(cur_pc = addr, regs);

	    if (!stepping)
		continuing = 1;

#if BSP_MAX_BP > 0
	    bsp_install_all_breakpoints();
#endif
	    /*
	     *  Completely flush caches here.
	     */
	    bsp_flush_dcache((void *)cur_pc, 8);
	    bsp_flush_icache((void *)cur_pc, 8);

	    if (stepping)
		bsp_singlestep_setup(regs);

#ifdef GDB_THREAD_SUPPORT
	    lock_thread_scheduler(stepping ? 0 : 1, regs);
#endif
	    /*
	     *  Set console write function to use gdb specific console i/o
	     *  only if the console procs are NULL.
	     */
	    if (bsp_shared_data->__console_procs == NULL)
		bsp_shared_data->__console_procs = &gdb_console_procs;

#if defined(DELAYED_ACK)
	    bsp_debug_write("+", 1);
	    bsp_debug_write("", 0);   /* this causes a flush */
#endif
	    in_gdb_handler = 0;
	    return;
          
	    /* kill the program */
	  case 'k':
	do_kill:

	    BSPLOG(bsp_log("gdb killing inferior.\n"));

	    if (bsp_shared_data->__kill_vector)
		bsp_shared_data->__kill_vector(sigval, regs);

#if defined(DELAYED_ACK)
	    if (!gdb_connect_closed) {
		bsp_debug_write("+", 1);
		bsp_debug_write("", 0);   /* this causes a flush */
	    }
#endif

	    if (_bsp_kill_hook)
		_bsp_kill_hook();
	    
#if BSP_MAX_BP > 0
	    bsp_init_breakpoints();
#endif
	    in_gdb_handler = 0;
	    gdb_connect_closed = 0;
	    /* restore original context */
	    memcpy(regs, &orig_context, sizeof(ex_regs_t));
	    return;

	  case 'q':
	    /* general query packet */
	    process_query(ptr, regs);
	    break;

	  case 'Q':
	    /* general set packet */
	    process_set(ptr, regs);
	    break;

#ifdef GDB_THREAD_SUPPORT
	  case 'H':
	    _gdb_changethread(ptr, regs);
	    break ;

	  case 'T' :
	    _gdb_thread_alive(ptr);
	    break ;
#endif
#if BSP_MAX_BP > 0
	  case 'Z':
	    is_binary = 1; /* overload use for is_binary */
	    /* fall through */
	  case 'z':
	    if (__unpack_ulong((char **)&ptr, &ztype)) {
		if (*(ptr++) == ',') {
		    if (__unpack_ulong((char **)&ptr, &addr)) {
			if (*(ptr++) == ',') {
			    if (__unpack_ulong((char **)&ptr, &length)) {
				switch (ztype) {
				  case 0:
				    /* sw breakpoint */
				    if (is_binary)
					i = bsp_add_breakpoint((void *)addr, length);
				    else
					i = bsp_remove_breakpoint((void *)addr, length);
				    if (i)
					_gdb_pkt_append("E%02x", i);
				    else
					_gdb_pkt_append("OK");
				    break;
				  case 1:
				    /* hw breakpoint */
#ifdef HAVE_HW_BREAKPOINT
				    if ((i = bsp_hw_breakpoint(is_binary, (void *)addr, length)))
					_gdb_pkt_append("E%02x", i);
				    else
					_gdb_pkt_append("OK");
#endif
				    break;
				  case 2:
				  case 3:
				  case 4:
				    /* hw watchpoints */
#ifdef HAVE_HW_WATCHPOINT
				    if ((i = bsp_hw_watchpoint(is_binary, (void *)addr, length, ztype)))
					_gdb_pkt_append("E%02x", i);
				    else
					_gdb_pkt_append("OK");
#endif
				    break;
				}
				break;
			    }
			}
		    }
		}
	    }
	    /* packet error */
	    _gdb_pkt_append("E02");
	    break;
#endif
	  default:
	    if (_bsp_gdb_data.__pkt_hook)
		_bsp_gdb_data.__pkt_hook(ptr - 1);
	}
    
	/* reply to the request */
	_gdb_pkt_end();
	_gdb_pkt_send();
    }
}


/* Table used by the crc32 function to calcuate the checksum. */
static unsigned crc32_table[256];
static int tableInit = 0;

/* 
 * Calculate a CRC-32 using LEN bytes of PTR. CRC is the initial CRC
 * value. PTR is assumed to be a pointer in the user program.
 */

static void
crc32 (unsigned char *ptr, int len)
{
    unsigned char ch;
    unsigned crc = 0xffffffff;

    if (!tableInit) {
	/* Initialize the CRC table and the decoding table. */
	unsigned int i, j, c;

	tableInit = 1;
	for (i = 0; i < 256; i++) {
	    for (c = i << 24, j = 8; j > 0; --j)
		c = c & 0x80000000 ? (c << 1) ^ 0x04c11db7 : (c << 1);
	    crc32_table[i] = c;
	}
    }

    while (len--) {
	if (__memory_read(ptr++, -1, 8, 1, &ch) <= 0) {
	    _gdb_pkt_append("E01");
	    return;
	}
	crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ ch) & 255];
    }
    _gdb_pkt_append("C%x", crc);
}



/* Handle the 'q' request */
static void
process_query (unsigned char *pkt, void *regs)
{
    unsigned long startmem;
    unsigned long length;

    switch (*pkt) {
#ifdef GDB_THREAD_SUPPORT
      case 'L':
	_gdb_getthreadlist(pkt+1);
	break ;
#endif

#ifdef GDB_THREAD_SUPPORT
      case 'P':
	_gdb_getthreadinfo(pkt+1);
	break ;
#endif

      case 'C':
	if (!strncmp(pkt+1, "RC:", 3)) {
	    pkt += 4;
	    if (__unpack_ulong((char **)&pkt, &startmem))
		if (*pkt++ == ',')
		    if (__unpack_ulong((char **)&pkt, &length))
			crc32((unsigned char *)startmem, length);
	    return;
	}
#ifdef GDB_THREAD_SUPPORT
	_gdb_currthread();
	break ;
#endif
	/* fall through if !GDB_THREAD_SUPPORT */
      default:
	if (_bsp_gdb_data.__pkt_query_hook)
	    _bsp_gdb_data.__pkt_query_hook(pkt);
	break;
    }
}


/* Handle the 'Q' request */
static void
process_set (unsigned char *pkt, void *regs)
{
    switch (*pkt) {
      case 'p':
#ifdef GDB_THREAD_SUPPORT
	/* reserve the packet id even if support is not present */
	/* Dont strip the 'p' off the header, there are several variations of
	   this packet */
	_gdb_changethread(pkt, regs);
#endif
	break;

      default:
	if (_bsp_gdb_data.__pkt_set_hook)
	    _bsp_gdb_data.__pkt_set_hook(pkt);
	break;
    }
}
