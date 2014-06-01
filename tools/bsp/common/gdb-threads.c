/* 
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

/* FIXME: Scan this module for correct sizes of fields in packets */

#include <string.h>
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include <bsp/dbg-threads-api.h>
#include "gdb.h"
#include <bsp/hex-utils.h>

#ifdef GDB_THREAD_SUPPORT

#define UNIT_TEST 0
#define GDB_MOCKUP 0

#define STUB_BUF_MAX 300 /* for range checking of packet lengths */
     
#include "gdb-threads.h"

#if !defined(PKT_DEBUG)
#define PKT_DEBUG 0
#endif

#if PKT_DEBUG
#warning "PKT_DEBUG macros engaged"
#define PKT_TRACE(x) bsp_printf x
#else
#define PKT_TRACE(x)
#endif 

/* This is going to be irregular because the various implementations
 * have adopted different names for registers.
 * It would be nice to fix them to have a common convention
 *   _stub_registers
 *   stub_registers
 *   alt_stub_registers
 */

/* Registers from current general thread */
extern ex_regs_t _gdb_general_registers;

/* Pack an error response into the response packet */
#define PKT_NAK()     _gdb_pkt_append("E02")

/* Pack an OK achnowledgement */
#define PKT_ACK()     _gdb_pkt_append("OK")

/* ------ UNPACK_THREADID ------------------------------- */
/* A threadid is a 64 bit quantity                        */

#define BUFTHREADIDSIZ 16 /* encode 64 bits in 16 chars of hex */

static char *
unpack_threadid(char * inbuf, threadref * id)
{
    int i;
    char * altref = (char *) id;

    for (i = 0; i < (BUFTHREADIDSIZ/2); i++)
	*altref++ = __unpack_nibbles(&inbuf, 2);
    return inbuf ;
}


/* -------- PACK_STRING ---------------------------------------------- */
static void
pack_string(char *string)
{
    char ch ;
    int len ;

    len = strlen(string);
    if (len > 200)
	len = 200 ; /* Bigger than most GDB packets, junk??? */

    _gdb_pkt_append("%02x", len);
    while (len-- > 0) { 
	ch = *string++ ;
	if (ch == '#')
	    ch = '*'; /* Protect encapsulation */
	_gdb_pkt_append("%c", ch);
    }
}


/* ----- PACK_THREADID  --------------------------------------------- */
/* Convert a binary 64 bit threadid  and pack it into a xmit buffer */
/* Return the advanced buffer pointer */
static void
pack_threadid(threadref *id)
{
    int i;
    unsigned char *p;

    p = (unsigned char *)id;

    for (i = 0; i < 8; i++)
	_gdb_pkt_append("%02x", *p++);
}



/* UNFORTUNATLY, not all ow the extended debugging system has yet been
   converted to 64 but thread referenecces and process identifiers.
   These routines do the conversion.
   An array of bytes is the correct treatment of an opaque identifier.
   ints have endian issues.
 */
static void
int_to_threadref(threadref *id, int value)
{
    unsigned char *scan = (unsigned char *)id;
    int i;

    for (i = 0; i < 4; i++)
	*scan++ = 0;

    *scan++ = (value >> 24) & 0xff;
    *scan++ = (value >> 16) & 0xff;
    *scan++ = (value >> 8) & 0xff;
    *scan++ = (value & 0xff);
}

static int
threadref_to_int(threadref *ref)
{
    int value = 0 ;
    unsigned char * scan ;
    int i ;
  
    scan = (char *) ref ;
    scan += 4 ;
    i = 4 ;
    while (i-- > 0)
	value = (value << 8) | ((*scan++) & 0xff);
    return value ;
}


int
_gdb_get_currthread (void)
{
    threadref thread;
  
    if (dbg_currthread(&thread))
	return threadref_to_int(&thread);

    return 0;
}


void
_gdb_currthread(void)
{
    threadref thread;
  
    if (dbg_currthread(&thread)) {
	_gdb_pkt_append("QC%08x", threadref_to_int(&thread));
    } else {
	PKT_NAK();
    }
}


/*
 * Answer the thread alive query
 */
static int
thread_alive(int id)
{
    threadref thread ;
    struct cygmon_thread_debug_info info ;

    int_to_threadref(&thread, id) ;
    if (dbg_threadinfo(&thread, &info) && info.context_exists)
	return 1;

    return 0;
}


void
_gdb_thread_alive(char *inbuf)
{
    unsigned long id;
  
    if (__unpack_ulong(&inbuf, &id)) {
	if (thread_alive((int)id)) {
	    PKT_ACK();
	    return;
	}
    }
    PKT_NAK();
}


/*
 * Writeback changed registers for current general thread.
 */
static void
writeback_regs(void)
{
    threadref  id;
    gdb_regs_t gdb_regs;

    if (_gdb_general_thread != 0 && _gdb_general_thread != -1) {
	int_to_threadref(&id, _gdb_general_thread);
	bsp_copy_exc_to_gdb_regs(&gdb_regs, &_gdb_general_registers);
	dbg_setthreadreg(&id, NUMREGS, &gdb_regs);
    }
}
 

/* ----- _GDB_CHANGETHREAD ------------------------------- */
/* Switch the display of registers to that of a saved context */

/* Changing the context makes NO sense, although the packets define the
   capability. Therefore, the option to change the context back does
   call the function to change registers. Also, there is no
   forced context switch.
     'p' - New format, long long threadid, no special cases
     'c' - Old format, id for continue, 32 bit threadid max, possably less
                       -1 means continue all threads
     'g' - Old Format, id for general use (other than continue)

     replies:
          OK for success
	  ENN for error
   */
void
_gdb_changethread(char *inbuf, ex_regs_t *exc_regs)
{
    threadref id, cur_id;
    int       idefined = -1, is_neg = 0, new_thread = 0;
    unsigned long ul;
    gdb_regs_t gdb_regs;

    PKT_TRACE(("_gdb_changethread: <%s> gen[%d] cont[%d]\n",
	       inbuf, _gdb_general_thread, _gdb_cont_thread));

    /* Parse the incoming packet for a thread identifier */
    switch (*inbuf++) {
      case 'p':
	/* New format: mode:8,threadid:64 */
	idefined = __unpack_nibbles(&inbuf, 1);
	inbuf = unpack_threadid(inbuf, &id); /* even if startflag */
	break ;  

      case 'c':
	/* old format , specify thread for continue */
	if (*inbuf == '-') {
	    is_neg = 1;
	    ++inbuf;
	}
	__unpack_ulong(&inbuf, &ul);
	new_thread = (int)ul;
	if (is_neg) {
	    new_thread = -new_thread;
	    if (new_thread == -1)
		new_thread = 0;
	}

	if (new_thread == 0 || 		/* revert to any old thread */
	    thread_alive(new_thread)) {	/* specified thread is alive */
	    _gdb_cont_thread = new_thread;
	    PKT_ACK();
	} else
	    PKT_NAK();
	break ;

      case 'g':
	/* old format, specify thread for general operations */
	/* OLD format: parse a variable length hex string */
	/* OLD format consider special thread ids */
	if (*inbuf == '-') {
	    is_neg = 1;
	    ++inbuf;
	}
	__unpack_ulong(&inbuf, &ul);
	new_thread = (int)ul;
	if (is_neg)
	    new_thread = -new_thread;

	int_to_threadref(&id, new_thread);
	switch (new_thread) {
	  case  0 : /* pick a thread, any thread */
	  case -1 : /* all threads */
	    idefined = 2;
	    writeback_regs();
	    _gdb_general_thread = new_thread;
	    break ;
	  default :
	    idefined = 1 ; /* select the specified thread */
	    break ;
	}
	break ;

      default:
	PKT_NAK();
	break ;
    }

    PKT_TRACE(("_gdb_changethread: idefined[%d] gen[%d] cont[%d]\n",
	       idefined, _gdb_general_thread, _gdb_cont_thread));

    switch (idefined) {
      case -1 :
	/* Packet not supported, already NAKed, no further action */
	break ;
      case 0 :
	/* Switch back to interrupted context */
	/*_registers = &registers;*/
	break ;
      case 1 :
	writeback_regs();
	/*
	 * dbg_getthreadreg will fail for the original thread.
	 * This is normal.
	 */
	if (dbg_currthread(&cur_id)) {
	    if (threadref_to_int(&cur_id) == new_thread) {
		_gdb_general_thread = 0;
		PKT_ACK();
		return;
	    }
	}
	/*
         * The OS will now update the values it has in a saved
	 * process context. This will probably not be all gdb regs,
         * so we preload the gdb_regs structure with current
         * values.
	 */
	bsp_copy_exc_to_gdb_regs(&gdb_regs, exc_regs);
	if (dbg_getthreadreg(&id, NUMREGS, &gdb_regs)) {
	    /*
	     * Now setup _gdb_general_registers. Initialize _gdb_registers
	     * with the exception frame registers so that the non-gdb regs
	     * have the best chance of being reasonably correct.
	     */
	    memcpy(&_gdb_general_registers, exc_regs, sizeof(ex_regs_t));
	    bsp_copy_gdb_to_exc_regs(&_gdb_general_registers, &gdb_regs);
	    _gdb_general_thread = new_thread;
	    PKT_ACK(); 
	} else {
	    PKT_NAK();
	}
	break ;
      case 2 :
	/* switch to interrupted context */ 
	PKT_ACK();
	break ;
      default:
	PKT_NAK();
	break ;
    }
}


/* ---- _GDB_GETTHREADLIST ------------------------------- */
/* 
 * Get a portion of the threadlist  or process list.
 * This may be part of a multipacket transaction.
 * It would be hard to tell in the response packet the difference
 * between the end of list and an error in threadid encoding.
 */
#define MAX_THREAD_CNT 24
void
_gdb_getthreadlist(char *inbuf)
{
    threadref  th_buf[MAX_THREAD_CNT], req_lastthread;
    int  i, start_flag, batchsize, count, done;
    static threadref lastthread, nextthread;

    PKT_TRACE(("_gdb_getthreadlist: <%s>\n", inbuf));

    start_flag = __unpack_nibbles(&inbuf, 1);
    batchsize  = __unpack_nibbles(&inbuf, 2);
    if (batchsize > MAX_THREAD_CNT)
	batchsize = MAX_THREAD_CNT;

    /* even if startflag */
    inbuf = unpack_threadid(inbuf, &lastthread);

    /* save for later */
    memcpy(&req_lastthread, &lastthread, sizeof(threadref));

    PKT_TRACE(("pkt_getthreadlist: start_flag[%d] batchsz[%d] last[0x%x]\n",
	       start_flag, batchsize, threadref_to_int(lastthread)));

    /* Acquire the thread ids from the kernel */
    for (done = count = 0; count < batchsize; ++count) {
	if (!dbg_threadlist(start_flag, &lastthread, &nextthread)) {
	    done = 1;
	    break;
	}
	start_flag = 0; /* redundent but effective */
#if 0 /* DEBUG */
	if (!memcmp(&lastthread, &nextthread, sizeof(threadref))) {
	    bsp_printf("FAIL: Threadlist, not incrementing\n");
	    done = 1;
	    break;
	}
#endif      
	PKT_TRACE(("pkt_getthreadlist: Adding thread[0x%x]\n",
	       threadref_to_int(nextthread)));

	memcpy(&th_buf[count], &nextthread, sizeof(threadref));
	memcpy(&lastthread, &nextthread, sizeof(threadref));
    }

    /* Build response packet    */
    _gdb_pkt_append("QM%02x%d", count, done);
    pack_threadid(&req_lastthread);
    for (i = 0; i < count; ++i)
	pack_threadid(&th_buf[i]);
}




/* ----- _GDB_GETTHREADINFO ---------------------------------------- */
/*
 * Get the detailed information about a specific thread or process.
 *
 * Encoding:
 *   'Q':8,'P':8,mask:16
 *
 *    Mask Fields
 *	threadid:1,        # always request threadid 
 *	context_exists:2,
 *	display:4,          
 *	unique_name:8,
 *	more_display:16
 */  
void
_gdb_getthreadinfo(char *inbuf)
{
    int mask ;
    int result ;
    threadref thread ;
    struct cygmon_thread_debug_info info ;

    info.context_exists = 0 ;
    info.thread_display = 0 ;
    info.unique_thread_name = 0 ;
    info.more_display = 0 ;

    /* Assume the packet identification chars have already been
       discarded by the packet demultiples routines */
    PKT_TRACE(("_gdb_getthreadinfo: <%s>\n",inbuf));
  
    mask = __unpack_nibbles(&inbuf, 8) ;
    inbuf = unpack_threadid(inbuf,&thread) ;
  
    result = dbg_threadinfo(&thread, &info); /* Make kernel call */

    if (result)	{
	_gdb_pkt_append("qp%08x", mask);
	pack_threadid(&info.thread_id); /* echo threadid */

	if (mask & 2) {
	    /* context-exists */
	    _gdb_pkt_append("%08x%02x%02x", 2, 2, info.context_exists);
	}
	if ((mask & 4) && info.thread_display) {
	    /* display */
	    _gdb_pkt_append("%08x", 4);
	    pack_string(info.thread_display);
	}
	if ((mask & 8) && info.unique_thread_name) {
	    /* unique_name */
	    _gdb_pkt_append("%08x", 8);
	    pack_string(info.unique_thread_name);
	}
	if ((mask & 16) && info.more_display) {
	    /* more display */
	    _gdb_pkt_append("%08x", 16);
	    pack_string(info.more_display);
	}
    } else {
	PKT_TRACE(("FAIL: dbg_threadinfo\n"));
	PKT_NAK();
    }
}


int
_gdb_lock_scheduler(int lock, 	/* 0 to unlock, 1 to lock */
		    int mode, 	/* 0 for step,  1 for continue */
		    long id)	/* current thread */
{
    threadref thread;

    int_to_threadref(&thread, id);
    return dbg_scheduler(&thread, lock, mode);
}


#endif /* GDB_THREAD_SUPPORT */
