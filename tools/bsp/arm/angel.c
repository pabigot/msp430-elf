/*
 * cpu.c -- ARM(R) Angel glue.
 *
 * Copyright (c) 2000 Red Hat, Inc.
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
 *
 * Intel is a Registered Trademark of Intel Corporation.
 * StrongARM is a Registered Trademark of Advanced RISC Machines Limited.
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */
#include <bsp/cpu.h>
#include <bsp/bsp.h>
#include <stdlib.h>

#include "angel_endian.h"
#include "ardi.h"
#include "buffers.h"
#include "channels.h"
#include "hostchan.h"
#include "host.h"
#include "angel_bytesex.h"
#include "dbg_cp.h"
#include "adp.h"
#include "hsys.h"
#include "logging.h"
#include "msgbuild.h"
#include "rxtx.h"
#include "devsw.h"
#include "params.h"

#define ADP_Ctrl ADP_Control

/* we need a starting point for our first buffers, this is a safe one */
int Armsd_BufferSize = ADP_BUFFER_MIN_SIZE;
int Armsd_LongBufSize = ADP_BUFFER_MIN_SIZE;

static void send_booted(void);
static void boot_listener(Packet *packet, void *stateptr);
static void adp_listener(Packet *packet, void *stateptr);
static void send_stopped(void);

/* Boot states */
#define BOOT_STARTUP   0
#define BOOT_AVAILABLE 1
#define BOOT_RESETTING 2
#define BOOT_CONNECTED 3


typedef struct adp_state {
    int       return_to_target;
    int       boot_state;
    int       stepping;
    ex_regs_t *saved_regs;
    unsigned  catch_mask;
} adp_state_t;

static adp_state_t  adp_state;

static char speed_str[25];
static const char banner_str[] =
    "ADP for CygMon : Serial/IRQ : MMU on, Caches enabled :\n" \
    "1.20 (Cygnus Solutions) built on " __DATE__ " at " __TIME__ "\n";


/* FIXME */
#define BOOT_TIMEOUT_PERIOD 100000


/*
 * This is the ADP debug agent entry point.
 */
void
_bsp_adp_handler(int exc_nr, void *regs)
{
    static short  first_time = 1;
    int           i;

bsp_printf("\n_bsp_adp_handler: pc[0x%x] sp[0x%x]\n",
	   bsp_get_pc(regs), ((ex_regs_t*)regs)->_sp); /* FIXME */

    if (first_time) {
	first_time = 0;
	adp_state.boot_state = BOOT_AVAILABLE;
	Adp_OpenDevice("SERIAL", (void *)0, 0);
	Adp_ChannelRegisterRead(CI_HADP, adp_listener, &adp_state);
	Adp_ChannelRegisterRead(CI_HBOOT, boot_listener, &adp_state);
	Adp_ChannelRegisterRead(CI_TBOOT, boot_listener, &adp_state);
    }

    adp_state.saved_regs = (ex_regs_t *)regs;

    if (adp_state.boot_state == BOOT_AVAILABLE) {
	send_booted();
	Adp_initSeq();
	i = 0;
	do {
	    Adp_AsynchronousProcessing(async_block_on_nothing);
	    if ((i++) > BOOT_TIMEOUT_PERIOD) {
		i = 0;
		/* send_booted(); */
	    }
	} while (adp_state.boot_state != BOOT_CONNECTED);
    } else
	send_stopped();

    while (!adp_state.return_to_target) {
	Adp_AsynchronousProcessing(async_block_on_nothing);
	adp_state.return_to_target = 0;
    }

    /* send any unsent packet(s) */
    Adp_AsynchronousProcessing(async_block_on_write);
}


static void
send_booted(void)
{
    unsigned int count, len, endianess[2];
    Packet   *packet;
    p_Buffer buffer;

    packet = (Packet *)DevSW_AllocatePacket(Armsd_BufferSize);
    buffer = BUFFERDATA(packet->pk_buffer);

#if __ARMEB__
    endianess[0] = ADP_CPU_BE;
    endianess[1] = ADP_CPU_BigEndian;
#else
    endianess[0] = ADP_CPU_LE;
    endianess[1] = 0;
#endif    

    len =  strlen(speed_str);
    count = msgbuild(buffer,
                     "%w%w%w%w%w%w%w%w%w%w%w%w",
                     (ADP_Booted | TtoH),
                     ADP_HandleUnknown,
                     ADP_HandleUnknown,
                     ADP_HandleUnknown,
                     Armsd_BufferSize,
                     Armsd_BufferSize,    /* long buffer size */
                     0x0001,              /* Angel version */
                     ADPVSN,              /* ADP version number */
                     0,                   /* ARM architecture info */
                     endianess[0],        /* CPU feature set */
                     endianess[1],        /* Target hardware status */
                     sizeof(banner_str) + len /* No. of bytes in banner */
		     );

    memcpy(buffer + count, banner_str, sizeof(banner_str) - 1);
    count += sizeof(banner_str) - 1;  /* -1 because sizeof includes trailing null */
    strcpy(buffer + count, speed_str);
    count += len + 1;    /* +1 because we want trailing null from strcpy */

    if (count >= Armsd_BufferSize) {
	/* boot message too long */
        count = Armsd_BufferSize;
        msgbuild(buffer + (11 * 4), "%w", count - (12*4));
    }
    
    packet->pk_length = count;

    /*
     * send the boot out.
     */
    Adp_ChannelWriteAsync(CI_TBOOT, packet);

    return;
}


static void
handle_param_negotiate(p_Buffer buffer, void *stateptr)
{
    unsigned int option[AP_NUM_PARAMS][AP_MAX_OPTIONS];
    ParameterList req_list[AP_NUM_PARAMS];
    ParameterOptions request, *user_options;
    unsigned int i;

    /* set up empty request */
    request.num_param_lists = AP_NUM_PARAMS;
    request.param_list = req_list;
    for (i = 0; i < AP_NUM_PARAMS; ++i) {
        request.param_list[i].num_options = AP_MAX_OPTIONS;
        request.param_list[i].option = option[i];
    }

    if (Angel_ReadParamOptionsMessage(buffer, &request)) {
        const ParameterConfig *config;
        unsigned int count, speed;
	Packet       *packet;
	p_Buffer     buff;

	packet = (Packet *)DevSW_AllocatePacket(Armsd_BufferSize);
        if (packet == NULL) {
	    bsp_printf("No more packets\n");  /*FIXME */
            return;
        }

	buff = BUFFERDATA(packet->pk_buffer);
        count = msgbuild(buff, "%w%w%w%w",
                         (ADP_ParamNegotiate | TtoH), 0,
                         ADP_HandleUnknown, ADP_HandleUnknown);

	Adp_Ioctl(DC_GET_USER_PARAMS, (void *)&user_options);
        config = Angel_MatchParams(&request, user_options);
        if (config != NULL) {
            /* found param match */
            PUT32LE(buff + count, RDIError_NoError);
            count += sizeof(word);
            count += Angel_BuildParamConfigMessage(buff + count, config);

	    /* Send ACK */
	    packet->pk_length = count;
	    Adp_ChannelWrite(CI_HBOOT, packet);

            /* set the new config */
	    Adp_Ioctl(DC_SET_PARAMS, (void *)config );
            if (Angel_FindParam(AP_BAUD_RATE, config, &speed))
		bsp_sprintf(speed_str, " Serial Rate: %6d\n", speed);
            else
                speed_str[0] = '\0';
        } else {
            /* no param match */
            PUT32LE(buff + count, RDIError_Error);
            count += sizeof(word);

	    packet->pk_length = count;
	    Adp_ChannelWrite(CI_HBOOT, packet);
        }
    }
}


/*
 * Rx callback for the HBOOT and TBOOT channels.
 */
static void
boot_listener(Packet *packet, void *stateptr)
{
    adp_state_t *statep = (adp_state_t *)stateptr;
    unsigned int reason, debugID, OSinfo1, OSinfo2;
    bool do_reset = FALSE;
    int reset_ack, err;
    Packet  *wpacket;
    p_Buffer buffer;

    unpack_message(BUFFERDATA(packet->pk_buffer), "%w%w%w%w",
		   &reason, &debugID, &OSinfo1, &OSinfo2);

    if (!(reason & TtoH)) {

        if (reason != ADP_ParamNegotiate)
	    DevSW_FreePacket(packet);
        
        switch (reason) {
	  case ADP_Booted:
	    if (statep->boot_state == BOOT_RESETTING) {
		/* booted ACK */
		Adp_initSeq();
		statep->boot_state = BOOT_CONNECTED;
	    }
	    break;

	  case ADP_Reboot:
	    /* Incoming Reboot request, send ACK */
	    if ((err = msgsend(CI_HBOOT, "%w%w%w%w%w", ADP_Reboot | TtoH,
			       ADP_HandleUnknown, ADP_HandleUnknown,
			       ADP_HandleUnknown, AB_NORMAL_ACK)) == 0) {
		/* FIXME */
		/* need to reboot here */
	    }
	    break;

	  case ADP_Reset:
	    /* Response to incoming Reset request depends on current state */
	    switch (statep->boot_state) {
	      case BOOT_STARTUP:
		reset_ack = AB_LATE_ACK;
		break;

	      case BOOT_RESETTING:
	      case BOOT_AVAILABLE:
	      case BOOT_CONNECTED:
		reset_ack = AB_NORMAL_ACK;
		do_reset = TRUE;
		statep->boot_state = BOOT_RESETTING;
		break;

	      default:
		reset_ack = AB_ERROR;
		break;
	    }

	    wpacket = DevSW_AllocatePacket(Armsd_BufferSize);
	    buffer = BUFFERDATA(wpacket->pk_buffer);
	    wpacket->pk_length = msgbuild(buffer,
					  "%w%w%w%w%w", ADP_Reset | TtoH,
					  ADP_HandleUnknown, ADP_HandleUnknown,
					  ADP_HandleUnknown, reset_ack);

	    Adp_ChannelWrite(CI_HBOOT, wpacket);

	    if (do_reset) {
		send_booted();
	    }

	    break;

	  case ADP_ParamNegotiate:
	    handle_param_negotiate(BUFFERDATA(packet->pk_buffer) + (4*4), stateptr);
	    DevSW_FreePacket(packet);
	    break;

	  case ADP_LinkCheck:
	    wpacket = DevSW_AllocatePacket(Armsd_BufferSize);
	    buffer = BUFFERDATA(wpacket->pk_buffer);
	    wpacket->pk_length = msgbuild(buffer,
					  "%w%w%w%w", (ADP_LinkCheck | TtoH), 0,
					  ADP_HandleUnknown, ADP_HandleUnknown);

	    Adp_ChannelWriteAsync(CI_HBOOT, wpacket);
	    Adp_initSeq();
	    break;

	  default:
	    break;
        }
    }
}


static void
send_stopped(void)
{
    unsigned int count, len;
    Packet   *packet;
    p_Buffer buffer;

    packet = (Packet *)DevSW_AllocatePacket(Armsd_BufferSize);
    buffer = BUFFERDATA(packet->pk_buffer);

    count = msgbuild(buffer,
                     "%w%w%w%w%w",
                     (ADP_Stopped | TtoH),
                     ADP_HandleUnknown,
                     ADP_HandleUnknown,
                     ADP_HandleUnknown,
                     ADP_Stopped_SoftwareInterrupt);

    packet->pk_length = count;

    Adp_ChannelWriteAsync(CI_TADP, packet);
}


static void
send_response(unsigned int reason, int extras, ...)
{
    va_list ap;
    Packet   *packet;
    p_Buffer buffer;
    unsigned int count, word;

    va_start(ap, extras);

    packet = (Packet *)DevSW_AllocatePacket(Armsd_BufferSize);
    if (packet == NULL) {
	va_end(ap);
	return;
    }

    buffer = BUFFERDATA(packet->pk_buffer);
    count = msgbuild(buffer, "%w%w%w%w",
		     (reason | TtoH),
		     ADP_HandleUnknown,
		     ADP_HandleUnknown,
		     ADP_HandleUnknown);

    buffer += count;

    while (extras-- > 0) {
	word = va_arg(ap, unsigned int);
	msgbuild(buffer, "%w", word);
	count += sizeof(word);
	buffer += sizeof(word);
    }

    packet->pk_length = count;
    Adp_ChannelWriteAsync(CI_HADP, packet);
    va_end (ap);
}


/*
 * Reply to Info requests from host.
 */
static void
handle_info(p_Buffer buffer, void *stateptr)
{
    unsigned int subreason;

    unpack_message(buffer, "%w", &subreason);
    buffer += sizeof(subreason);

    switch (subreason) {
      case ADP_Info_NOP:
	send_response(ADP_Info, 2, subreason, adp_ok);
	break;
      case ADP_Info_Target:
	send_response(ADP_Info, 4, subreason, adp_ok, 
		      ADP_Info_Target_CanInquireBufferSize | ADP_Info_Target_HW,
		      0);
	break;
      case ADP_Info_Points:
	send_response(ADP_Info, 3, subreason, adp_ok, 0);
	break;
      case ADP_Info_Step:
	send_response(ADP_Info, 3, subreason, adp_ok, ADP_Info_Step_Single);
	break;
      case ADP_Info_MMU:
	send_response(ADP_Info, 3, subreason, adp_ok, 0);
	break;
      case ADP_Info_SemiHosting:
	send_response(ADP_Info, 3, subreason, adp_failed, 0);
	break;
      case ADP_Info_CoPro:
	send_response(ADP_Info, 3, subreason, adp_ok, 0);
	break;
      case ADP_Info_Cycles:
	send_response(ADP_Info, 8, subreason, adp_failed,
		      0, 0, 0, 0, 0, 0);
	break;
      case ADP_Info_AngelBufferSize:
	send_response(ADP_Info, 4, subreason, adp_ok,
		      Armsd_BufferSize, Armsd_BufferSize);
	break;
      case ADP_Info_ChangeableSHSWI:
	send_response(ADP_Info, 2, subreason, adp_failed);
	break;
      case ADP_Info_CanTargetExecute:
	send_response(ADP_Info, 2, subreason, adp_ok);
	break;
      case ADP_Info_AgentEndianess:
#if __ARMEB__
	send_response(ADP_Info, 2, subreason, RDIError_BigEndian);
#else
	send_response(ADP_Info, 2, subreason, RDIError_LittleEndian);
#endif
	break;
      case ADP_Info_CanAckHeartbeat:
	send_response(ADP_Info, 2, subreason, adp_ok);
	break;
      case ADP_Info_DescribeCoPro:
      case ADP_Info_RequestCoProDesc:
      default:
	send_response(ADP_HADPUnrecognised, 1, ADP_Info);
	break;
    }
}


/*
 * Handle Ctrl requests from host.
 */
static void
handle_ctrl(p_Buffer buffer, void *stateptr)
{
    unsigned int subreason, u;


    unpack_message(buffer, "%w", &subreason);
    buffer += sizeof(subreason);

    switch (subreason) {
      case ADP_Ctrl_NOP:
	send_response(ADP_Ctrl, 2, subreason, adp_ok);
	break;
      case ADP_Ctrl_VectorCatch:
	unpack_message(buffer, "%w", &u);
	((adp_state_t *)stateptr)->catch_mask = u;
	send_response(ADP_Ctrl, 2, subreason, adp_ok);
	break;
      default:
	send_response(ADP_HADPUnrecognised, 1, ADP_Ctrl);
	break;
    }
}


static void
handle_read(unsigned char *address, unsigned int nbytes)
{
    Packet  *packet;
    p_Buffer buffer;
    unsigned int count;
    int   nread, toread;

#define MAX_TO_READ (Armsd_BufferSize - (12*4))

    packet = (Packet *)DevSW_AllocatePacket(Armsd_BufferSize);
    if (packet) {
	buffer = BUFFERDATA(packet->pk_buffer);

	count = msgbuild(buffer, "%w%w%w%w",
			 (ADP_Read | TtoH),
			 ADP_HandleUnknown,
			 ADP_HandleUnknown,
			 ADP_HandleUnknown);
	buffer += count;

	if (nbytes > MAX_TO_READ)
	    toread = MAX_TO_READ;
	else
	    toread = nbytes;

	nread = bsp_memory_read(address, 0, 1, nbytes, buffer + (2*4));
	count += nread;

	msgbuild(buffer, "%w%w", adp_ok, nbytes - nread);
	count += (2*4);

	packet->pk_length = count;

       	Adp_ChannelWriteAsync(CI_HADP, packet);
    }
}

static void
handle_write(unsigned char *data, unsigned char *address, unsigned int nbytes)
{
    Packet  *packet;
    int nwritten;

    nwritten = bsp_memory_write(address, 0, 1, nbytes, data);

    packet = (Packet *)DevSW_AllocatePacket(Armsd_BufferSize);
    if (packet) {
	packet->pk_length = msgbuild(BUFFERDATA(packet->pk_buffer),
				     "%w%w%w%w%w%w",
				     (ADP_Write | TtoH),
				     ADP_HandleUnknown,
				     ADP_HandleUnknown,
				     ADP_HandleUnknown,
				     (nwritten == nbytes) ? adp_ok : adp_failed,
				     nbytes - nwritten);
	Adp_ChannelWriteAsync(CI_HADP, packet);
    }
}


static void
handle_cpu_read(ex_regs_t *regs, unsigned char mode, unsigned int mask)
{
    Packet  *packet;
    p_Buffer buffer;
    unsigned int total, count, val;
    int i, n;

    packet = (Packet *)DevSW_AllocatePacket(Armsd_BufferSize);
    if (packet) {
	buffer = BUFFERDATA(packet->pk_buffer);

	count = msgbuild(buffer, "%w%w%w%w%w",
			 (ADP_CPUread | TtoH),
			 ADP_HandleUnknown,
			 ADP_HandleUnknown,
			 ADP_HandleUnknown,
			 adp_ok);
	buffer += count;
	total = count;

	for (n = REG_R0, i = 0; i < 15; i++, n++) {
	    if (mask & (1<<i)) {
		bsp_get_register(n, regs, &val);
		count = msgbuild(buffer, "%w", val);
		buffer += count;
		total  += count;
	    }
	}

	if (mask & (1<<15)) {
	    /* 26bit PC */
	    /* FIXME */
	    val = 0;
	    count = msgbuild(buffer, "%w", val);
	    buffer += count;
	    total  += count;
	}

	if (mask & (1<<16)) {
	    /* 32bit PC */
	    val = bsp_get_pc(regs);
	    count = msgbuild(buffer, "%w", val);
	    buffer += count;
	    total  += count;
	}

	if (mask & (1<<17)) {
	    /* CPSR */
	    bsp_get_register(REG_CPSR, regs, &val);
	    count = msgbuild(buffer, "%w", val);
	    buffer += count;
	    total  += count;
	}

	if (mask & (1<<17)) {
	    /* SPSR (we don't save this) */
	    /* FIXME */
	    val = 0;
	    count = msgbuild(buffer, "%w", val);
	    buffer += count;
	    total  += count;
	}

	packet->pk_length = total;

       	Adp_ChannelWriteAsync(CI_HADP, packet);
    }
}


/*
 * Rx callback for the ADP channel.
 */
static void
adp_listener(Packet *packet, void *stateptr)
{
    unsigned int reason, debugID, OSinfo1, OSinfo2;
    unsigned char *buffer, *addr, mode;
    unsigned int  nbytes, mask;


    buffer = BUFFERDATA(packet->pk_buffer);

    unpack_message(buffer, "%w%w%w%w",
		   &reason, &debugID, &OSinfo1, &OSinfo2);
    buffer += (4*4);  /* skip past 4 words we just unpacked */

    if (!(reason & TtoH)) {

        switch (reason) {
	  case ADP_Info:
	    handle_info(buffer, stateptr);
	    break;

	  case ADP_Ctrl:
	    handle_ctrl(buffer, stateptr);
	    break;

	  case ADP_Read:
	    unpack_message(buffer, "%w%w", &addr, &nbytes);
	    handle_read(addr, nbytes);
	    break;

	  case ADP_Write:
	    unpack_message(buffer, "%w%w", &addr, &nbytes);
	    buffer += (2*4);
	    handle_write(buffer, addr, nbytes);
	    break;

	  case ADP_CPUread:
	    unpack_message(buffer, "%b%w", &mode, &mask);
	    handle_cpu_read(((adp_state_t *)stateptr)->saved_regs, mode, mask);
	    break;
	       
	  case ADP_InitialiseApplication:
	    send_response(ADP_InitialiseApplication, 1, adp_ok);
	    break;

	  case ADP_End:
	    send_response(ADP_End, 1, adp_ok);
	    ((adp_state_t *)stateptr)->boot_state = BOOT_AVAILABLE;
	    break;

	  case ADP_Execute:
	    send_response(ADP_Execute, 1, adp_ok);
	    ((adp_state_t *)stateptr)->return_to_target = 1;
	    break;

	  default:
	    send_response(ADP_HADPUnrecognised, 1, reason);
	    bsp_printf("Unrecognized ADP message: %08x\n", reason);
	    break;
        }
    }
    DevSW_FreePacket(packet);
}


