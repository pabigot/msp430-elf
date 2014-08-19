/* gdb-if.c -- Aarch64 sim interface to GDB.

   Copyright (C) 2013 by Red Hat, Inc.

   This file is part of the GNU simulators.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
   
       (1) Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer. 
   
       (2) Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in
       the documentation and/or other materials provided with the
       distribution.  
       
       (3)The name of the author may not be used to
       endorse or promote products derived from this software without
       specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.  */ 

#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "ansidecl.h"
#include "gdb/callback.h"
#include "gdb/remote-sim.h"
#include "gdb/signals.h"
#include "gdb/sim-aarch64.h"

#include "cpustate.h"
#include "memory.h"
#include "simulator.h"

/* Ideally, we'd wrap up all the minisim's data structures in an
   object and pass that around.  However, neither GDB nor run needs
   that ability.

   So we just have one instance, that lives in global variables, and
   each time we open it, we re-initialize it.  */
struct sim_state
{
  const char * message;
};

static struct sim_state the_minisim =
{
  "This is the sole aarch64 minisim instance.  See libsim.a's global variables."
};

static bfd_boolean open = FALSE;


static void
check_desc (SIM_DESC sd)
{
  if (sd != & the_minisim)
    fprintf (stderr, "aarch64 minisim: desc != &the_minisim\n");
}

void
sim_close (SIM_DESC sd, int quitting)
{
  check_desc (sd);

  /* Not much to do.  At least free up our memory.  */
  aarch64_unload ();

  open = FALSE;
}

static bfd *
open_objfile (const char * filename)
{
  bfd * prog = bfd_openr (filename, 0);

  if (!prog)
    {
      fprintf (stderr, "Can't read %s\n", filename);
      return NULL;
    }

  if (!bfd_check_format (prog, bfd_object))
    {
      fprintf (stderr, "%s not a aarch64 program\n", filename);
      return NULL;
    }

  return prog;
}

SIM_RC
sim_load (SIM_DESC sd, char * prog, struct bfd * abfd, int from_tty)
{
  check_desc (sd);

  if (abfd == NULL)
    {
      abfd = open_objfile (prog);

      if (abfd == NULL)
	return SIM_RC_FAIL;
    }

  if (aarch64_load (abfd) && aarch64_init ())
    return SIM_RC_OK;

  return SIM_RC_FAIL;
}

SIM_RC
sim_create_inferior (SIM_DESC sd, struct bfd * abfd, char ** argv, char ** env)
{
  check_desc (sd);

  if (abfd)
    aarch64_load (abfd);

  return SIM_RC_OK;
}

int
sim_read (SIM_DESC sd, SIM_ADDR mem, unsigned char * buf, int length)
{
  int i;

  check_desc (sd);

  if (mem == 0)
    return 0;

  if (aarch64_get_status () == STATUS_ERROR)
    aarch64_set_status (STATUS_HALT, ERROR_NONE);

  for (i = 0; i < length; i++)
    {
      bfd_vma addr = mem + i;

      buf[i] = aarch64_get_mem_u8 (addr);

      if (aarch64_get_status () == STATUS_ERROR)
	return i;
    }

  return length;
}

int
sim_write (SIM_DESC sd, SIM_ADDR mem, const unsigned char * buf, int length)
{
  int i;

  check_desc (sd);

  if (aarch64_get_status () == STATUS_ERROR)
    aarch64_set_status (STATUS_HALT, ERROR_NONE);

  for (i = 0; i < length; i++)
    {
      bfd_vma addr = mem + i;
 
      aarch64_set_mem_u8 (addr, buf[i]);

      if (aarch64_get_status () == STATUS_ERROR)
	return i;
    }

  return length;
}

/* Read the LENGTH bytes at BUF as a little-endian value.  */

static bfd_vma
get_le (unsigned char * buf, unsigned int length)
{
  bfd_vma acc = 0;

  while (length -- > 0)
    acc = (acc << 8) + buf[length];

  return acc;
}

/* Store VAL as a little-endian value in the LENGTH bytes at BUF.  */

static void
put_le (unsigned char * buf, unsigned int length, bfd_vma val)
{
  int i;

  for (i = 0; i < length; i++)
    {
      buf[i] = val & 0xff;
      val >>= 8;
    }
}

static int
check_regno (enum sim_aarch64_regnum regno)
{
  return 0 <= regno && regno < 67;
}

static size_t
reg_size (enum sim_aarch64_regnum regno)
{
  if (regno == 65 || regno == 66)
    return 32;
  return 64;
}

int
sim_fetch_register (SIM_DESC sd, int regno, unsigned char *buf, int length)
{
  size_t size;
  bfd_vma val;

  check_desc (sd);

  if (!check_regno (regno))
    return 0;

  size = reg_size (regno);

  if (length != size)
    return 0;

  switch (regno)
    {
    case  0 ... 31: val = aarch64_get_reg_u64 (regno, 0); break;
    case 32 ... 63: val = aarch64_get_FP_double (regno - 32); break;
    case 64: val = aarch64_get_PC (); break;
    case 65: val = aarch64_get_CPSR (); break;
    case 66: val = aarch64_get_FPSR (); break;
    default:
      fprintf (stderr, "aarch64 minisim: unrecognized register number: %d\n",
	       regno);
      return -1;
    }

  put_le (buf, length, val);

  return size;
}

int
sim_store_register (SIM_DESC sd, int regno, unsigned char *buf, int length)
{
  size_t size;
  bfd_vma val;

  check_desc (sd);

  if (!check_regno (regno))
    return -1;

  size = reg_size (regno);

  if (length != size)
    return -1;

  val = get_le (buf, length);

  switch (regno)
    {
    case  0 ... 31: aarch64_set_reg_u64 (regno, 1, val); break;
    case 32 ... 63: aarch64_set_FP_double (regno - 32, (double) val); break;
    case 64:
      aarch64_set_next_PC (val);
      aarch64_update_PC ();
      break;

    case 65: aarch64_set_CPSR (val); break;
    case 66: aarch64_set_FPSR (val); break;
    default:
      fprintf (stderr, "aarch64 minisim: unrecognized register number: %d\n",
	       regno);
      return 0;
    }

  return size;
}

void
sim_info (SIM_DESC sd, int verbose)
{
  check_desc (sd);

  printf ("The aarch64 minisim doesn't collect any statistics.\n");
}

static enum sim_stop reason;
int siggnal;

/* Take a step return code RC and set up the variables consulted by
   sim_stop_reason appropriately.  */

static void
handle_status (int rc)
{
  switch (rc)
    {
    case STATUS_READY:
    case STATUS_RETURN:
      reason = sim_exited;
      siggnal = 0;
      break;

    case STATUS_CALLOUT:
    case STATUS_HALT:
      reason = sim_stopped;
      siggnal = GDB_SIGNAL_EMT;

    case STATUS_BREAK:
      reason = sim_stopped;
      siggnal = GDB_SIGNAL_TRAP;
      break;

    default:
    case STATUS_ERROR:
      reason = sim_stopped;
      siggnal = GDB_SIGNAL_SEGV;
      break;
    }
}

void
sim_resume (SIM_DESC sd, int step, int sig_to_deliver)
{
  check_desc (sd);

  if (sig_to_deliver != 0)
    fprintf (stderr,
	     "Warning: the aarch64 minisim does not implement "
	     "signal delivery yet.\n" "Resuming with no signal.\n");

  if (step)
    aarch64_step ();
  else
    aarch64_run ();

  handle_status (aarch64_get_status ());
}

int
sim_stop (SIM_DESC sd)
{
  /*FIXME: What should we do here ?  */

  return 1;
}

void
sim_stop_reason (SIM_DESC sd, enum sim_stop *reason_p, int *sigrc_p)
{
  check_desc (sd);

  *reason_p = reason;
  *sigrc_p = siggnal;
}

void
sim_do_command (SIM_DESC sd, char *cmd)
{
  check_desc (sd);

  char *p = cmd;

  /* Skip leading whitespace.  */
  while (isspace (*p))
    p++;

  /* Find the extent of the command word.  */
  for (; *p; p++)
    if (isspace (*p))
      break;

  /* Null-terminate the command word, and record the start of any
     further arguments.  */
  char *args;
  if (*p)
    {
      *p = '\0';
      args = p + 1;
      while (isspace (*args))
	args++;
    }
  else
    args = p;

  /* FIXME: process the command word.  */
}

char **
sim_complete_command (SIM_DESC sd, const char *text, const char *word)
{
  return NULL;
}

SIM_DESC
sim_open (SIM_OPEN_KIND                  kind,
	  struct host_callback_struct *  callback,
	  struct bfd *                   abfd,
	  char **                        argv)
{
  if (open)
    fprintf (stderr, "aarch64 minisim: re-opened sim\n");

  /* The 'run' interface doesn't use this function, so we don't care
     about KIND; it's always SIM_OPEN_DEBUG.  */
  if (kind != SIM_OPEN_DEBUG)
    fprintf (stderr, "aarch64 minisim: sim_open KIND != SIM_OPEN_DEBUG: %d\n",
	     kind);

  /* We don't expect any command-line arguments.  */
  if (aarch64_load (abfd) && aarch64_init ())
    open = TRUE;

  return & the_minisim;
}
