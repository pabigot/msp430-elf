/* 
 * Copyright (C) 1995, 2000 Advanced RISC Machines Limited. All rights reserved.
 * 
 * This software may be freely used, copied, modified, and distributed
 * provided that the above copyright notice is preserved in all copies of the
 * software.
 */

/* -*-C-*-
 *
 * $Revision: 1.1 $
 *     $Date: 2000/02/25 13:52:32 $
 *
 *
 * serdrv.c - Synchronous Serial Driver for Angel.
 *            This is nice and simple just to get something going.
 */

#ifdef __hpux
#  define _POSIX_SOURCE 1
#endif
#include <bsp/bsp.h>
#include <stdlib.h>
#include <string.h>

#include "crc.h"
#include "devices.h"
#include "buffers.h"
#include "rxtx.h"
#include "hostchan.h"
#include "params.h"
#include "logging.h"

#if 0
#define DO_TRACE 1
#endif
#define printf bsp_printf

extern int baud_rate;   /* From gdb/top.c */

#ifndef UNUSED
#  define UNUSED(x) (x = x)      /* Silence compiler warnings */
#endif
 
#define MAXREADSIZE 512
#define MAXWRITESIZE 512

#define SERIAL_FC_SET  ((1<<serial_XON)|(1<<serial_XOFF))
#define SERIAL_CTL_SET ((1<<serial_STX)|(1<<serial_ETX)|(1<<serial_ESC))
#define SERIAL_ESC_SET (SERIAL_FC_SET|SERIAL_CTL_SET)

static const struct re_config config = {
    serial_STX, serial_ETX, serial_ESC, /* self-explanatory?               */
    SERIAL_FC_SET,                      /* set of flow-control characters  */
    SERIAL_ESC_SET,                     /* set of characters to be escaped */
    NULL /* serial_flow_control */, NULL  ,    /* what to do with FC chars */
    angel_DD_RxEng_BufferAlloc, NULL                /* how to get a buffer */
};

static struct re_state rxstate;

typedef struct writestate {
  unsigned int wbindex;
  /*  static te_status testatus;*/
  unsigned char writebuf[MAXWRITESIZE];
  struct te_state txstate;
} writestate;

static struct writestate wstate;

/*
 * The set of parameter options supported by the device
 */
static unsigned int baud_options[] = {
    38400, 19200, 9600
};

static ParameterList param_list[] = {
    { AP_BAUD_RATE,
      sizeof(baud_options)/sizeof(unsigned int),
      baud_options }
};

static const ParameterOptions serial_options = {
    sizeof(param_list)/sizeof(ParameterList), param_list };

/* 
 * The default parameter config for the device
 */
static Parameter param_default[] = {
    { AP_BAUD_RATE, 9600 }
};

static ParameterConfig serial_defaults = {
    sizeof(param_default)/sizeof(Parameter), param_default };

/*
 * The user-modified options for the device
 */
static unsigned int user_baud_options[sizeof(baud_options)/sizeof(unsigned)];

static ParameterList param_user_list[] = {
    { AP_BAUD_RATE,
      sizeof(user_baud_options)/sizeof(unsigned),
      user_baud_options }
};

static ParameterOptions user_options = {
    sizeof(param_user_list)/sizeof(ParameterList), param_user_list };

static bool user_options_set;

/* forward declarations */
static int serial_reset( void );
static int serial_set_params( const ParameterConfig *config );
static int SerialMatch(const char *name, const char *arg);

static void process_baud_rate( unsigned int target_baud_rate )
{
    const ParameterList *full_list;
    ParameterList       *user_list;

    /* create subset of full options */
    full_list = Angel_FindParamList( &serial_options, AP_BAUD_RATE );
    user_list = Angel_FindParamList( &user_options,   AP_BAUD_RATE );

    if ( full_list != NULL && user_list != NULL )
    {
        unsigned int i, j;
        unsigned int def_baud = 0;

        /* find lower or equal to */
        for ( i = 0; i < full_list->num_options; ++i )
           if ( target_baud_rate >= full_list->option[i] )
           {
               /* copy remaining */
               for ( j = 0; j < (full_list->num_options - i); ++j )
                  user_list->option[j] = full_list->option[i+j];
               user_list->num_options = j;

               /* check this is not the default */
               Angel_FindParam( AP_BAUD_RATE, &serial_defaults, &def_baud );
               if ( (j == 1) && (user_list->option[0] == def_baud) )
               {
#ifdef DEBUG
                   printf( "user selected default\n" );
#endif
               }
               else
               {
                   user_options_set = TRUE;
#ifdef DEBUG
                   printf( "user options are: " );
                   for ( j = 0; j < user_list->num_options; ++j )
                      printf( "%u ", user_list->option[j] );
                   printf( "\n" );
#endif
               }

               break;   /* out of i loop */
           }
                
#ifdef DEBUG
        if ( i >= full_list->num_options )
           printf( "couldn't match baud rate %u\n", target_baud_rate );
#endif
    }
#ifdef DEBUG
    else
       printf( "failed to find lists\n" );
#endif
}

static int SerialOpen(const char *name, const char *arg)
{
#ifdef DEBUG
    printf("SerialOpen: name %s arg %s\n", name, arg ? arg : "<NULL>");
#endif

    user_options_set = FALSE;

    process_baud_rate(bsp_set_serial_baud(bsp_set_debug_comm(-1), -1));

    serial_reset();

    Angel_RxEngineInit(&config, &rxstate);
    /*
     * DANGER!: passing in NULL as the packet is ok for now as it is just
     * IGNOREd but this may well change
     */
    Angel_TxEngineInit(&config, NULL, &wstate.txstate); 
    return 0;
}

static int SerialMatch(const char *name, const char *arg)
{
    return 0;
}

static void SerialClose(void)
{
#ifdef DO_TRACE
    printf("SerialClose()\n");
#endif
}

static int SerialRead(DriverCall *dc, bool block) {
  static unsigned char readbuf[MAXREADSIZE];
  static int rbindex=0;

  int nread;
  int read_errno;
  int c=0;
  re_status restatus;
  int ret_code = -1;            /* assume bad packet or error */

  /* FIXME!! Handle block */
  nread = bsp_debug_read(readbuf+rbindex, MAXREADSIZE-rbindex);
  read_errno = 0;

  if ((nread > 0) || (rbindex > 0)) {

#if 0 /* FIXME */
#ifdef DO_TRACE
    printf("[%d@%d] ", nread, rbindex);
#endif
#endif

    if (nread>0)
       rbindex = rbindex+nread;

    do {
      restatus = Angel_RxEngine(readbuf[c], &(dc->dc_packet), &rxstate);

#if 0 /* FIXME */
#ifdef DO_TRACE
      printf("<%02X ",readbuf[c]);
      if (!(++c % 16))
          printf("\n");
#else
      c++;
#endif
#else
      c++;
#endif
    } while (c<rbindex &&
             ((restatus == RS_IN_PKT) || (restatus == RS_WAIT_PKT)));

#if 0 /* FIXME */
#ifdef DO_TRACE
   if (c % 16)
        printf("\n");
#endif
#endif

    switch(restatus) {
      
      case RS_GOOD_PKT:
#ifdef DO_TRACE
	{
	    int i;
	    for (i = 0; i < dc->dc_packet.len; i++) {
		if (!(i % 16))
		    printf("\n");
		printf("<%02X ",dc->dc_packet.data[i]);
	    }
	    if (i % 16)
		printf("\n");
	}
#endif
        ret_code = 1;
        /* fall through to: */

      case RS_BAD_PKT:
        /*
         * We now need to shuffle any left over data down to the
         * beginning of our private buffer ready to be used 
         * for the next packet 
         */
#ifdef DO_TRACE
        printf("SerialRead() processed %d, moving down %d\n", c, rbindex-c);
#endif
        if (c != rbindex) memmove((char *) readbuf, (char *) (readbuf+c),
                                  rbindex-c);
        rbindex -= c;
        break;

      case RS_IN_PKT:
      case RS_WAIT_PKT:
#if 0 /* FIXME */
#ifdef DO_TRACE
        printf("in pkt\n");
#endif
#endif
        rbindex = 0;            /* will have processed all we had */
        ret_code = 0;
        break;

      default:
#ifdef DEBUG
        printf("Bad re_status in serialRead()\n");
#endif
        break;
    }
  } else if (nread == 0)
    ret_code = 0;               /* nothing to read */

  return ret_code;
}


static int SerialWrite(DriverCall *dc) {
  int nwritten = 0;
  te_status testatus = TS_IN_PKT;

  if (dc->dc_context == NULL) {
#ifdef DEBUG
    printf("SerialWrite: TXInit\n");
#endif
    Angel_TxEngineInit(&config, &(dc->dc_packet), &(wstate.txstate));
    wstate.wbindex = 0;
    dc->dc_context = &wstate;
  }

  while ((testatus == TS_IN_PKT) && (wstate.wbindex < MAXWRITESIZE))
  {
    /* send the raw data through the tx engine to escape and encapsulate */
    testatus = Angel_TxEngine(&(dc->dc_packet), &(wstate.txstate),
                              &(wstate.writebuf)[wstate.wbindex]);
    if (testatus != TS_IDLE) wstate.wbindex++;
  }

  if (testatus == TS_IDLE) {
#ifdef DEBUG
    printf("SerialWrite: testatus is TS_IDLE during preprocessing\n");
#endif
  }

#ifdef DO_TRACE
  { 
    int i = 0;

    while (i<wstate.wbindex)
    {
        printf(">%02X ",wstate.writebuf[i]);

        if (!(++i % 16))
            printf("\n");
    }
    if (i % 16)
        printf("\n");
  }
#endif

  bsp_debug_write(wstate.writebuf, wstate.wbindex);
  nwritten = wstate.wbindex;

  if (nwritten < 0) {
    nwritten=0;
  }

#ifdef DEBUG
  if (nwritten > 0)
    printf("Wrote %#04x bytes\n", nwritten);
#endif

  if ((unsigned) nwritten == wstate.wbindex && 
      (testatus == TS_DONE_PKT || testatus == TS_IDLE)) {

    /* finished sending the packet */

#ifdef DEBUG
    printf("SerialWrite: calling Angel_TxEngineInit after sending packet (len=%i)\n",wstate.wbindex);
#endif
    testatus = TS_IN_PKT;
    wstate.wbindex = 0;
    return 1;
  }
  else {
#ifdef DEBUG
    printf("SerialWrite: Wrote part of packet wbindex=%i, nwritten=%i\n",
           wstate.wbindex, nwritten);
#endif
   
    /*
     *  still some data left to send shuffle whats left down and reset
     * the ptr
     */
    memmove((char *) wstate.writebuf, (char *) (wstate.writebuf+nwritten),
            wstate.wbindex-nwritten);
    wstate.wbindex -= nwritten;
    return 0;
  }
  return -1;
}


static int serial_reset( void )
{
#ifdef DEBUG
    printf( "serial_reset\n" );
#endif

    return serial_set_params( &serial_defaults );
}


static int serial_set_params( const ParameterConfig *config )
{
    unsigned int speed;
    int termios_value;

#ifdef DEBUG
    printf( "serial_set_params\n" );
#endif

    if ( ! Angel_FindParam( AP_BAUD_RATE, config, &speed ) )
    {
#ifdef DEBUG
        printf( "speed not found in config\n" );
#endif
        return DE_OKAY;
    }

#ifdef DEBUG
    printf( "setting speed to %u\n", speed );
#endif


    if (bsp_set_serial_baud(bsp_set_debug_comm(-1), speed))
	bsp_printf("Invalid baud rate: %d\n", speed);
    
    return DE_OKAY;
}


static int serial_get_user_params( ParameterOptions **p_options )
{
#ifdef DEBUG
    printf( "serial_get_user_params\n" );
#endif

    if ( user_options_set )
    {
        *p_options = &user_options;
    }
    else
    {
        *p_options = NULL;
    }

    return DE_OKAY;
}


static int serial_get_default_params( ParameterConfig **p_config )
{
#ifdef DEBUG
    printf( "serial_get_default_params\n" );
#endif

    *p_config = (ParameterConfig *) &serial_defaults;
    return DE_OKAY;
}


static int SerialIoctl(const int opcode, void *args) {

    int ret_code;

#ifdef DEBUG
    printf( "SerialIoctl: op %d arg %p\n", opcode, args ? args : "<NULL>");
#endif

    switch (opcode)
    {
       case DC_RESET:         
           ret_code = serial_reset();
           break;

       case DC_SET_PARAMS:     
           ret_code = serial_set_params((const ParameterConfig *)args);
           break;

       case DC_GET_USER_PARAMS:     
           ret_code = serial_get_user_params((ParameterOptions **)args);
           break;

       case DC_GET_DEFAULT_PARAMS:
           ret_code = serial_get_default_params((ParameterConfig **)args);
           break;

       default:               
           ret_code = DE_BAD_OP;
           break;
    }

  return ret_code;
}

DeviceDescr angel_SerialDevice = {
    "SERIAL",
    SerialOpen,
    SerialMatch,
    SerialClose,
    SerialRead,
    SerialWrite,
    SerialIoctl
};
