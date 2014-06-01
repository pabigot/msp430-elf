/* Support for Intel x86-based NetApp box.
   Based on code Copyright 1988, 1991, 1992, 1993, 1994 Free Software
   Foundation, Inc.

This file is part of GDB.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "defs.h"
#include <sys/types.h>
#include <sys/param.h>
#include "symtab.h"
#include "inferior.h"

static void	netapp_fetch_thread_register (CORE_ADDR sp, int regno);

static void	netapp_store_thread_register (CORE_ADDR sp, int regno, char *regs);

/* A PID is just a pointer to a "struct sk_Proc".  Its context is
   in the "context" member of that structure. */

/* A context switch is done by calling "sk_hw_switch()".

   On the 386, "sk_hw_switch()" does a context switch is done by:

	Pushing %ebp
	Pushing %edi
	Pushing %esi
	Pushing %ebx
	Storing %esp into the "sp" member of the "context" member of
	  the "sk_Proc" structure for the process.

   When "sk_hw_switch" is called, its first argument is a pointer
   to the "context" member of the process giving up control, i.e.
   a pointer to the place where the SP of that process will be
   stored.

   Other registers are treated as caller-save registers, so we don't
   care what values that have, which is good because they're not saved
   across a context switch.

   Thus, what we have is:

	SP for process ->   EBX for process
			    ESI for process
			    EDI for process
			    EBP for process
			    EIP of caller of "sk_hw_switch()
			    pointer to ESP of caller of "sk_hw_switch()"

   as the EIP was pushed by the "call" and the pointer to the ESP of
   the caller is the argument to "sk_hw_switch()" pushed before the
   call, so we fetch 6 pointer's worth of data pointed to by the SP,
   and get the five registers in question from that, and the ESP from
   the location pointed to by the 6th value.

   This will return a set of registers that make it look as if
   "sk_hw_switch()" had just returned; I guess if this really
   confuses people we can hack it to back the EIP up so that it looks
   as if we're just about to call "sk_hw_switch()".  It seems kind of
   pointless to leave "sk_hw_switch()" on the stack, though, unless
   not doing so screws up our ability to do certain things to threads
   other than the currently active one.
   
   Note: every thread has the first five words on it (EBX, ESI,
   EDI, EBP, and EIP).  But some threads do not have the sixth word,
   pointer to ESP.  In particular, there are virgin threads that have
   been created and never activated.  These threads have EBP=0 and
   do not have a pointer to ESP in the sixth word.  Instead, a virgin
   thread has its invocation function in EIP, and the arguments to
   the invocation function after that.
   */

/* Mappings from tm-i386v.h */
#define EBX_IN_CONTEXT	0
#define ESI_IN_CONTEXT	1
#define EDI_IN_CONTEXT	2
#define EBP_IN_CONTEXT	3
#define EIP_IN_CONTEXT	4
#define ESP_IN_CONTEXT	5
#define NOT_IN_CONTEXT	-1	/* not available for other threads */

static int regmap[NUM_REGS] = {
  NOT_IN_CONTEXT,		/* EAX */
  NOT_IN_CONTEXT,		/* ECX */
  NOT_IN_CONTEXT,		/* EDX */
  EBX_IN_CONTEXT,
  ESP_IN_CONTEXT,
  EBP_IN_CONTEXT,
  ESI_IN_CONTEXT,
  EDI_IN_CONTEXT,
  EIP_IN_CONTEXT,
  NOT_IN_CONTEXT,
  NOT_IN_CONTEXT,		/* XXX - CS; need to get common value? */
  NOT_IN_CONTEXT,		/* XXX - SS; need to get common value? */
  NOT_IN_CONTEXT,		/* XXX - DS; need to get common value? */
  NOT_IN_CONTEXT,		/* XXX - ES; need to get common value? */
  NOT_IN_CONTEXT,		/* FS */
  NOT_IN_CONTEXT,		/* GS */
};

/* Fetch registers for "sk" process whose PID is "inferior_ptid".
   Must not be called if "inferior_ptid" equals "sk_running_proc",
   as the registers for the running process are not where they'd
   be if the thread weren't running. */
void
netapp_fetch_thread_registers (int regno)
{
  CORE_ADDR sp;

  /* Get the SP for the process. */
  sp = netapp_get_proc_sp (PIDGET (inferior_ptid)); 

  if (regno >= 0)
    {
      /* Get the register in question. */
      netapp_fetch_thread_register (sp, regno);
    }
  else
    {
      /* Get all the registers. */
      for (regno = 0; regno < NUM_REGS; regno++)
	netapp_fetch_thread_register (sp, regno);
    }
}

static void
netapp_fetch_thread_register (CORE_ADDR sp, int regno)
{
  char buf[TARGET_PTR_BIT / TARGET_CHAR_BIT];
  CORE_ADDR regaddr;
  CORE_ADDR spp;
  CORE_ADDR bp;
  int virgin;

  if (regmap[regno] == NOT_IN_CONTEXT)
    return;			/* we can't fetch it */
  regaddr = sp + regmap[regno]*REGISTER_RAW_SIZE(regno);

  /* Figure out if this is a virgin thread. */
  target_read_memory (sp + regmap[EBP_REGNUM] * REGISTER_RAW_SIZE(EBP_REGNUM),
                      buf, REGISTER_RAW_SIZE(regno));
  bp = extract_address (buf, TARGET_PTR_BIT / TARGET_CHAR_BIT);
  virgin = (bp == 0) ? 1 : 0;

  if (virgin)
    {
      if (regno == ESP_REGNUM)
	{
	  /* Use the raw SP. */
	  store_address (buf, TARGET_PTR_BIT / TARGET_CHAR_BIT, sp);
	}
      else if (regno == EBP_REGNUM)
	{
	  /* Fake a BP just below the EIP. */
	  bp = sp + 3 * REGISTER_RAW_SIZE(regno);
	  store_address (buf, TARGET_PTR_BIT / TARGET_CHAR_BIT, bp);
	}
      else
	{
	  /* Get the register at the offset from the context.
	     XXX - if it returns a non-zero value, we should fail! */
	  target_read_memory (regaddr, buf, REGISTER_RAW_SIZE(regno));
	}
    }
  else
    {
      if (regno == ESP_REGNUM)
	{
	  /* Special case - the context contains a pointer to the SP value,
	     not the SP value itself.
	     XXX - if it returns a non-zero value, we should fail! */
	  target_read_memory (regaddr, buf, TARGET_PTR_BIT / TARGET_CHAR_BIT);
	  spp = extract_address (buf, TARGET_PTR_BIT / TARGET_CHAR_BIT);
	  target_read_memory (spp, buf, REGISTER_RAW_SIZE(regno));
	}
      else
	{
	  /* Get the register at the offset from the context.
	     XXX - if it returns a non-zero value, we should fail! */
	  target_read_memory (regaddr, buf, REGISTER_RAW_SIZE(regno));
	}
    }

  supply_register (regno, buf);
}

/* Store registers for "sk" process whose PID is "inferior_ptid".
   Must not be called if "inferior_ptid" equals "sk_running_proc",
   as the registers for the running process are not where they'd
   be if the thread weren't running. */
void
netapp_store_thread_registers (int regno, char *regs)
{
  CORE_ADDR sp;

  /* Get the SP for the process. */
  sp = netapp_get_proc_sp (PIDGET (inferior_ptid)); 

  if (regno >= 0)
    {
      /* Set the register in question. */
      netapp_store_thread_register (sp, regno, regs);
    }
  else
    {
      /* Set all the registers. */
      for (regno = 0; regno < NUM_REGS; regno++)
	netapp_store_thread_register (sp, regno, regs);
    }
}

static void
netapp_store_thread_register (CORE_ADDR sp, int regno, char *regs)
{
  int i;
  CORE_ADDR regaddr;
  char buf[TARGET_PTR_BIT / TARGET_CHAR_BIT];
  CORE_ADDR spp;

  i = regmap[regno];
  if (i == NOT_IN_CONTEXT)
    return;			/* we can't set it */
  regaddr = sp + i*REGISTER_RAW_SIZE(regno);
  if (regno == ESP_REGNUM)
    {
      /* Special case - the context contains a pointer to the SP value,
         not the SP value itself.
         XXX - if it returns a non-zero value, we should fail! */
      target_read_memory (regaddr, buf, TARGET_PTR_BIT / TARGET_CHAR_BIT);
      spp = extract_address (buf, TARGET_PTR_BIT / TARGET_CHAR_BIT);
      target_write_memory (spp, &regs[REGISTER_BYTE(ESP_REGNUM)],
		REGISTER_RAW_SIZE(regno));
    }
  else
    {
      /* Get the register at the offset from the context.
         XXX - if it returns a non-zero value, we should fail! */
      target_write_memory (regaddr,
		&regs[REGISTER_BYTE(regno)], REGISTER_RAW_SIZE(regno));
    }
}

/* Get the value of the "call_firmware_ebp" variable. */
CORE_ADDR
netapp_get_call_firmware_ebp (void)
{
  struct minimal_symbol *call_firmware_ebp_symp;
  static CORE_ADDR call_firmware_ebp_addr;
  char buf[TARGET_PTR_BIT / TARGET_CHAR_BIT];
  CORE_ADDR call_firmware_ebp;

  /* Get the address of "call_firmware_ebp" if we don't already
     have it. */
  if (call_firmware_ebp_addr == 0)
    {
      call_firmware_ebp_symp = lookup_minimal_symbol ("call_firmware_ebp", NULL,
	(struct objfile *) NULL);
      if (call_firmware_ebp_symp == NULL)
	return 0;		/* not defined? */

      call_firmware_ebp_addr = SYMBOL_VALUE_ADDRESS (call_firmware_ebp_symp);
    }

  /* Get the value of "call_firmware_ebp".
     XXX - if it returns a non-zero value, we should fail! */
  target_read_memory (call_firmware_ebp_addr, buf,
			  TARGET_PTR_BIT / TARGET_CHAR_BIT);
  call_firmware_ebp = extract_address (buf, TARGET_PTR_BIT / TARGET_CHAR_BIT);

  return call_firmware_ebp;
}
