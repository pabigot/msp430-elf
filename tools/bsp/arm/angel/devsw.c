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
 */
#include <stdlib.h>
#include <bsp/bsp.h>

#include "adp.h"
#include "sys.h"
#include "hsys.h"
#include "rxtx.h"
#include "drivers.h"
#include "buffers.h"
#include "devclnt.h"
#include "adperr.h"
#include "devsw.h"
#include "hostchan.h"
#include "logging.h"

#define printf bsp_printf

static char *angelDebugFilename = NULL;
static FILE *angelDebugLogFile = NULL;
static int angelDebugLogEnable = 0;

static void openLogFile ()
{
#if 0
  time_t t;
  struct tm lt;
  
  if (angelDebugFilename == NULL || *angelDebugFilename =='\0')
    return;
  
  angelDebugLogFile = fopen (angelDebugFilename,"a");
  
  if (!angelDebugLogFile)
    {
      fprintf (stderr,"Error opening log file '%s'\n",angelDebugFilename);
      perror ("fopen");
    }
  else
    {
      /* The following line is equivalent to: */
      /* setlinebuf (angelDebugLogFile); */
      setvbuf(angelDebugLogFile, (char *)NULL, _IOLBF, 0);
#if defined(__CYGWIN32__) || defined(__CYGWIN__)
      setmode(fileno(angelDebugLogFile), O_TEXT);
#endif
    }
  
  time (&t);
  fprintf (angelDebugLogFile,"ADP log file opened at %s\n",asctime(localtime(&t)));
#endif
}


static void closeLogFile (void)
{
#if 0
  time_t t;
  struct tm lt;
  
  if (!angelDebugLogFile)
    return;
  
  time (&t);
  fprintf (angelDebugLogFile,"ADP log file closed at %s\n",asctime(localtime(&t)));
  
  fclose (angelDebugLogFile);
  angelDebugLogFile = NULL;
#endif
}

void DevSW_SetLogEnable (int logEnableFlag)
{
  if (logEnableFlag && !angelDebugLogFile)
    openLogFile ();
  else if (!logEnableFlag && angelDebugLogFile)
    closeLogFile ();
  
  angelDebugLogEnable = logEnableFlag;
}


void DevSW_SetLogfile (const char *filename)
{
  closeLogFile ();
  
  if (angelDebugFilename)
    {
	/*      free (angelDebugFilename);*/
      angelDebugFilename = NULL;
    }
  
  if (filename && *filename)
    {
      angelDebugFilename = strdup (filename);
      if (angelDebugLogEnable)
        openLogFile ();
    }
}


#define WordAt(p)  ((unsigned long) ((p)[0] | ((p)[1]<<8) | ((p)[2]<<16) | ((p)[3]<<24)))

static void dumpPacket(FILE *fp, char *label, struct data_packet *p)
{
  unsigned r;
  int i;
  unsigned char channel;
  
  if (!fp)
    return;
  
  bsp_printf("%s [T=%d L=%d] ",label,p->type,p->len);
  for (i=0; i<p->len; ++i)
    bsp_printf("%02x ",p->data[i]);
  bsp_printf("\n");

  channel = p->data[0];

  r = WordAt(p->data+4);
  
  bsp_printf("R=%08x ",r);
  bsp_printf("%s ", r&0x80000000 ? "H<-T" : "H->T");

  switch (channel)
    {
     case CI_PRIVATE: bsp_printf("CI_PRIVATE: "); break;
     case CI_HADP: bsp_printf("CI_HADP: "); break;
     case CI_TADP: bsp_printf("CI_TADP: "); break;
     case CI_HBOOT: bsp_printf("CI_HBOOT: "); break;
     case CI_TBOOT: bsp_printf("CI_TBOOT: "); break;
     case CI_CLIB: bsp_printf("CI_CLIB: "); break;
     case CI_HUDBG: bsp_printf("CI_HUDBG: "); break;
     case CI_TUDBG: bsp_printf("CI_TUDBG: "); break;
     case CI_HTDCC: bsp_printf("CI_HTDCC: "); break;
     case CI_TTDCC: bsp_printf("CI_TTDCC: "); break;
     case CI_TLOG: bsp_printf("CI_TLOG: "); break;
     default:      bsp_printf("BadChan: "); break;
    }

  switch (r & 0xffffff)
    {
     case ADP_Booted: bsp_printf(" ADP_Booted "); break;
#if defined(ADP_TargetResetIndication)
     case ADP_TargetResetIndication: bsp_printf(" ADP_TargetResetIndication "); break;
#endif
     case ADP_Reboot: bsp_printf(" ADP_Reboot "); break;
     case ADP_Reset: bsp_printf(" ADP_Reset "); break;
#if defined(ADP_HostResetIndication)
     case ADP_HostResetIndication: bsp_printf(" ADP_HostResetIndication "); break;
#endif      
     case ADP_ParamNegotiate: bsp_printf(" ADP_ParamNegotiate "); break;
     case ADP_LinkCheck: bsp_printf(" ADP_LinkCheck "); break;
     case ADP_HADPUnrecognised: bsp_printf(" ADP_HADPUnrecognised "); break;
     case ADP_Info: bsp_printf(" ADP_Info "); break;
     case ADP_Control: bsp_printf(" ADP_Control "); break;
     case ADP_Read: bsp_printf(" ADP_Read "); break;
     case ADP_Write: bsp_printf(" ADP_Write "); break;
     case ADP_CPUread: bsp_printf(" ADP_CPUread "); break;
     case ADP_CPUwrite: bsp_printf(" ADP_CPUwrite "); break;
     case ADP_CPread: bsp_printf(" ADP_CPread "); break;
     case ADP_CPwrite: bsp_printf(" ADP_CPwrite "); break;
     case ADP_SetBreak: bsp_printf(" ADP_SetBreak "); break;
     case ADP_ClearBreak: bsp_printf(" ADP_ClearBreak "); break;
     case ADP_SetWatch: bsp_printf(" ADP_SetWatch "); break;
     case ADP_ClearWatch: bsp_printf(" ADP_ClearWatch "); break;
     case ADP_Execute: bsp_printf(" ADP_Execute "); break;
     case ADP_Step: bsp_printf(" ADP_Step "); break;
     case ADP_InterruptRequest: bsp_printf(" ADP_InterruptRequest "); break;
     case ADP_HW_Emulation: bsp_printf(" ADP_HW_Emulation "); break;
     case ADP_ICEbreakerHADP: bsp_printf(" ADP_ICEbreakerHADP "); break;
     case ADP_ICEman: bsp_printf(" ADP_ICEman "); break;
     case ADP_Profile: bsp_printf(" ADP_Profile "); break;
     case ADP_InitialiseApplication: bsp_printf(" ADP_InitialiseApplication "); break;
     case ADP_End: bsp_printf(" ADP_End "); break;
     case ADP_TADPUnrecognised: bsp_printf(" ADP_TADPUnrecognised "); break;
     case ADP_Stopped: bsp_printf(" ADP_Stopped "); break;
     case ADP_TDCC_ToHost: bsp_printf(" ADP_TDCC_ToHost "); break;
     case ADP_TDCC_FromHost: bsp_printf(" ADP_TDCC_FromHost "); break;

     case CL_Unrecognised: bsp_printf(" CL_Unrecognised "); break;
     case CL_WriteC: bsp_printf(" CL_WriteC "); break;
     case CL_Write0: bsp_printf(" CL_Write0 "); break;
     case CL_ReadC: bsp_printf(" CL_ReadC "); break;
     case CL_System: bsp_printf(" CL_System "); break;
     case CL_GetCmdLine: bsp_printf(" CL_GetCmdLine "); break;
     case CL_Clock: bsp_printf(" CL_Clock "); break;
     case CL_Time: bsp_printf(" CL_Time "); break;
     case CL_Remove: bsp_printf(" CL_Remove "); break;
     case CL_Rename: bsp_printf(" CL_Rename "); break;
     case CL_Open: bsp_printf(" CL_Open "); break;
     case CL_Close: bsp_printf(" CL_Close "); break;
     case CL_Write: bsp_printf(" CL_Write "); break;
     case CL_WriteX: bsp_printf(" CL_WriteX "); break;
     case CL_Read: bsp_printf(" CL_Read "); break;
     case CL_ReadX: bsp_printf(" CL_ReadX "); break;
     case CL_Seek: bsp_printf(" CL_Seek "); break;
     case CL_Flen: bsp_printf(" CL_Flen "); break;
     case CL_IsTTY: bsp_printf(" CL_IsTTY "); break;
     case CL_TmpNam: bsp_printf(" CL_TmpNam "); break;

     default: bsp_printf(" BadReason "); break;
    }

  i = 20;
  
  if (((r & 0xffffff) == ADP_CPUread ||
       (r & 0xffffff) == ADP_CPUwrite) && (r&0x80000000)==0)
    {
      bsp_printf("%02x ", p->data[i]);
      ++i;
    }
  
  for (; i<p->len; i+=4)
    bsp_printf("%08x ",WordAt(p->data+i));
  
  bsp_printf("\n");
}


/*
 * TODO: this should be adjustable - it could be done by defining
 *       a reason code for DevSW_Ioctl.  It could even be a
 *       per-devicechannel parameter.
 */
static const unsigned int allocsize = ADP_BUFFER_MIN_SIZE;

#define illegalDevChanID(type)  ((type) >= DC_NUM_CHANNELS)

/**********************************************************************/

/*
 *  Function: initialise_read
 *   Purpose: Set up a read request for another packet
 *
 *    Params:
 *      In/Out: ds      State structure to be initialised
 *
 *   Returns:
 *          OK: 0
 *       Error: -1
 */
static int initialise_read(DevSWState *ds)
{
    struct data_packet *dp;

    /*
     * try to claim the structure that will
     * eventually hold the new packet.
     */
    if ((ds->ds_nextreadpacket = DevSW_AllocatePacket(allocsize)) == NULL)
        return -1;

    /*
     * Calls into the device driver use the DriverCall structure: use
     * the buffer we have just allocated, and declare its size.  We
     * are also obliged to clear the driver's context pointer.
     */
    dp = &ds->ds_activeread.dc_packet;
    dp->buf_len = allocsize;
    dp->data = ds->ds_nextreadpacket->pk_buffer;

    ds->ds_activeread.dc_context = NULL;

    return 0;
}

/*
 *  Function: initialise_write
 *   Purpose: Set up a write request for another packet
 *
 *    Params:
 *       Input: packet  The packet to be written
 *
 *              type    The type of the packet
 *
 *      In/Out: dc      The structure to be intialised
 *
 *   Returns: Nothing
 */
static void initialise_write(DriverCall *dc, Packet *packet, DevChanID type)
{
    struct data_packet *dp = &dc->dc_packet;

    dp->len = packet->pk_length;
    dp->data = packet->pk_buffer;
    dp->type = type;

    /*
     * we are required to clear the state structure for the driver
     */
    dc->dc_context = NULL;
}

/*
 *  Function: enqueue_packet
 *   Purpose: move a newly read packet onto the appropriate queue
 *              of read packets
 *
 *    Params:
 *      In/Out: ds      State structure with new packet
 *
 *   Returns: Nothing
 */
static void enqueue_packet(DevSWState *ds)
{
    struct data_packet *dp = &ds->ds_activeread.dc_packet;
    Packet *packet = ds->ds_nextreadpacket;

    /*
     * transfer the length
     */
    packet->pk_length = dp->len;

    /*
     * take this packet out of the incoming slot
     */
    ds->ds_nextreadpacket = NULL;

    /*
     * try to put it on the correct input queue
     */
    if (illegalDevChanID(dp->type))
    {
        /* this shouldn't happen */
        WARN("Illegal type for Rx packet");
        DevSW_FreePacket(packet);
    }
    else
        Adp_addToQueue(&ds->ds_readqueue[dp->type], packet);
}

/*
 *  Function: flush_packet
 *   Purpose: Send a packet to the device driver
 *
 *    Params:
 *       Input: device  The device to be written to
 *
 *      In/Out: dc      Describes the packet to be sent
 *
 *   Returns: Nothing
 *
 * Post-conditions: If the whole packet was accepted by the device
 *                      driver, then dc->dc_packet.data will be
 *                      set to NULL.
 */
static void flush_packet(const DeviceDescr *device, DriverCall *dc)
{
    if (device->DeviceWrite(dc) > 0) {
        /*
         * the whole packet was swallowed
         */
        dc->dc_packet.data = NULL;
    }
}

/**********************************************************************/

#define NPKTS    8
#define PKTBUFSZ 512

static Packet pkt_array[NPKTS];
static char   buf_array[NPKTS][PKTBUFSZ];
static int    pkt_is_alloc = 0;

/*
 * These are the externally visible functions.  They are documented in
 * devsw.h
 */
Packet *DevSW_AllocatePacket(const unsigned int length)
{
    Packet *pk;
    int    i;
    static unsigned int maxlen = 0;



    if ((length+CHAN_HEADER_SIZE) < maxlen) {
	maxlen = (length+CHAN_HEADER_SIZE);
	bsp_printf("biggest pkt[%d]\n", length+CHAN_HEADER_SIZE);
    }

    if ((length+CHAN_HEADER_SIZE) > PKTBUFSZ) {
	bsp_printf("Packet: too big[%d]\n", length+CHAN_HEADER_SIZE);
	return NULL;
    }

    pk = NULL;
    for (i = 0; i < NPKTS; i++) {
	if (!(pkt_is_alloc & (1<<i))) {
	    pk = &pkt_array[i];
	    pk->pk_buffer = buf_array[i];
	    pkt_is_alloc |= (1<<i);
	    break;
	}
    }

#if 0 /* FIXME */
    bsp_printf("+pk[%x]\n", pk);
#else
    if (pk == NULL)
	bsp_printf("no more packets\n");
#endif

    return pk;
}

void DevSW_FreePacket(Packet *pk)
{
    int    i;

    for (i = 0; i < NPKTS; i++) {
	if (pk == &pkt_array[i]) {
	    pkt_is_alloc &= ~(1<<i);
#if 0 /* FIXME */
	    bsp_printf("-pk[%x]\n", pk);
#endif
	    return;
	}
    }
    bsp_printf("DevSW_FreePacket: pk[%x] not freed. pkt_is_alloc[%x]\n", pk, pkt_is_alloc);
}


static DevSWState dstate;
static int        ds_is_alloc;


AdpErrs DevSW_Open(DeviceDescr *device, const char *name, const char *arg,
                   const DevChanID type)
{
    DevSWState *ds;

    /*
     * is this the very first open call for this driver?
     */
    if ((ds = (DevSWState *)(device->SwitcherState)) == NULL)
    {
        /*
         * yes, it is: initialise state
         */
	if (ds_is_alloc)
            /* give up */
            return adp_malloc_failure;

	ds = &dstate;
	ds_is_alloc = 1;

        (void)memset(ds, 0, sizeof(*ds));
        device->SwitcherState = (void *)ds;
    }

    /*
     * check that we haven't already been opened for this type
     */
    if ((ds->ds_opendevchans & (1 << type)) != 0)
        return adp_device_already_open;

    /*
     * if no opens have been done for this device, then do it now
     */
    if (ds->ds_opendevchans == 0)
        if (device->DeviceOpen(name, arg) < 0)
            return adp_device_open_failed;

    /*
     * open has finished
     */
    ds->ds_opendevchans |= (1 << type);
    return adp_ok;
}

AdpErrs DevSW_Match(const DeviceDescr *device, const char *name,
                    const char *arg)
{
    return (device->DeviceMatch(name, arg) == -1) ? adp_failed : adp_ok;
}

AdpErrs DevSW_Close (DeviceDescr *device, const DevChanID type)
{
    DevSWState *ds = (DevSWState *)(device->SwitcherState);
    Packet *pk;

    if ((ds->ds_opendevchans & (1 << type)) == 0)
        return adp_device_not_open;

    ds->ds_opendevchans &= ~(1 << type);

    /*
     * if this is the last close for this channel, then inform the driver
     */
    if (ds->ds_opendevchans == 0)
        device->DeviceClose();

    /*
     * release all packets of the appropriate type
     */
    for (pk = Adp_removeFromQueue(&(ds->ds_readqueue[type]));
         pk != NULL;
         pk = Adp_removeFromQueue(&(ds->ds_readqueue[type])))
        DevSW_FreePacket(pk);

    /* Free memory */
    ds_is_alloc = 0;
    device->SwitcherState = 0x0;

    /* that's all */
    return adp_ok;
}

AdpErrs DevSW_Read(const DeviceDescr *device, const DevChanID type,
                   Packet **packet, bool block)
{
  int read_err;
  DevSWState *ds = device->SwitcherState;

    /*
     * To try to get information out of the device driver as
     * quickly as possible, we try and read more packets, even
     * if a completed packet is already available.
     */

    /*
     * have we got a packet currently pending?
     */
  if (ds->ds_nextreadpacket == NULL)
      /*
       * no - set things up
       */
      if (initialise_read(ds) < 0) {
	  /*
	   * we failed to initialise the next packet, but can
	   * still return a packet that has already arrived.
	   */
	  *packet = Adp_removeFromQueue(&ds->ds_readqueue[type]); 
	  return adp_ok;
      }
  read_err = device->DeviceRead(&ds->ds_activeread, block);
  switch (read_err) {
  case 1:
    /*
     * driver has pulled in a complete packet, queue it up
     */
#ifdef RET_DEBUG
    printf("got a complete packet\n");
#endif
    
    if (angelDebugLogEnable)
      dumpPacket(angelDebugLogFile,"rx:",&ds->ds_activeread.dc_packet);

    enqueue_packet(ds);
    *packet = Adp_removeFromQueue(&ds->ds_readqueue[type]);
    return adp_ok;
  case 0:
    /*
     * OK, return the head of the read queue for the given type
     */
    /*    enqueue_packet(ds); */
    *packet = Adp_removeFromQueue(&ds->ds_readqueue[type]);
    return adp_ok;
  case -1:
#ifdef RET_DEBUG
    printf("got a bad packet\n");
#endif
    /* bad packet */
    *packet = NULL;
    return adp_bad_packet;
  default:
    bsp_printf("DevSW_Read: bad read status %d", read_err);
  }
  return 0; /* get rid of a potential compiler warning */
}


AdpErrs DevSW_FlushPendingWrite(const DeviceDescr *device)
{
    struct DriverCall *dc;
    struct data_packet *dp;

    dc = &((DevSWState *)(device->SwitcherState))->ds_activewrite;
    dp = &dc->dc_packet;

    /*
     * try to flush any packet that is still being written
     */
    if (dp->data != NULL)
    {
        flush_packet(device, dc);

        /* see if it has gone */
        if (dp->data != NULL)
           return adp_write_busy;
        else
           return adp_ok;
    }
    else
       return adp_ok;
}


AdpErrs DevSW_Write(const DeviceDescr *device, Packet *packet, DevChanID type)
{
    struct DriverCall *dc;
    struct data_packet *dp;

    dc = &((DevSWState *)(device->SwitcherState))->ds_activewrite;
    dp = &dc->dc_packet;

    if (illegalDevChanID(type))
        return adp_illegal_args;

    /*
     * try to flush any packet that is still being written
     */
    if (DevSW_FlushPendingWrite(device) != adp_ok)
       return adp_write_busy;

    /*
     * we can take this packet - set things up, then try to get rid of it
     */
    initialise_write(dc, packet, type);
  
    if (angelDebugLogEnable)
      dumpPacket(angelDebugLogFile,"tx:",&dc->dc_packet);
  
    flush_packet(device, dc);

    return adp_ok;
}

AdpErrs DevSW_Ioctl(const DeviceDescr *device, const int opcode, void *args)
{
    return (device->DeviceIoctl(opcode, args) < 0) ? adp_failed : adp_ok;
}

bool DevSW_WriteFinished(const DeviceDescr *device)
{
    struct DriverCall *dc;
    struct data_packet *dp;

    dc = &((DevSWState *)(device->SwitcherState))->ds_activewrite;
    dp = &dc->dc_packet;

    return (dp == NULL || dp->data == NULL);
}

/* EOF devsw.c */
