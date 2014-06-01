/* Low level interface for debugging NetApp threads for GDB, the GNU debugger.

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

/* This module implements a sort of half target that sits between the
   machine-independent parts of GDB and the remote interface (remote.c) to
   provide access to the NetApp thread implementation.  */

#include "defs.h"

#include "gdbthread.h"
#include "target.h"
#include "inferior.h"

extern struct target_ops netapp_thread_ops; /* Forward declaration */
extern struct target_ops netapp_core_ops; /* Forward declaration */

/* place to store core_ops before we overwrite it */
static struct target_ops orig_core_ops;

struct target_ops netapp_thread_ops;
struct target_ops netapp_core_ops;

extern struct target_ops remote_ops; /* target vector for remote.c */
extern struct target_ops core_ops; /* target vector for corelow.c */

/* TRUE if we've already scanned for threads. */
static int scanned_for_threads;

/* Address of the "sk" process table. */
static CORE_ADDR sk_proc_table;

/* Size of an entry in the "sk" process table. */
static int sk_proc_size;

/* PID of the currently-running process, which is the index of the
   currently running process in the process table - we can't use
   the address as a PID, as PIDs are "int"s in GDB and a pointer
   won't fit in an "int" on Alpha.

   The indices are 1-origin, as a PID of 0 seems to mean something
   special to GDB. */
int sk_running_proc_pid;

/* Macros to convert PIDs to pointers to the process, and *vice versa*. */
#define	PID_TO_PROC_PTR(pid)	(sk_proc_table + ((pid) - 1)*sk_proc_size)
#define	PROC_PTR_TO_PID(proc_ptr) \
	(((proc_ptr) - sk_proc_table)/sk_proc_size + 1)

static int
netapp_get_sk_nprocs PARAMS ((void));

static void
netapp_get_sk_proc_size PARAMS ((void));

static int
netapp_get_sk_proc_table PARAMS ((void));

static int
netapp_is_sk_proc PARAMS ((int pid));

/* Convert an "sk" process ID to a string.  Returns the string in a static
   buffer.  */

static char *
netapp_pid_to_str (int pid)
{
  static char buf[40];

  sprintf (buf, "process 0x%lx", (unsigned long)PID_TO_PROC_PTR(pid));

  return buf;
}

static void
netapp_thread_detach (char *args, int from_tty)
{
  unpush_target (&netapp_thread_ops);
  remote_ops.to_detach (args, from_tty);
}

/* XXX - pick better names, so we don't confuse "netapp_thread_fetch_registers
   and "netapp_fetch_thread_registers".  */
static void
netapp_thread_fetch_registers (int regno)
{
  struct cleanup *old_chain;
  int i;

  if (!ptid_equal (inferior_ptid, null_ptid) 
      && PIDGET (inferior_ptid) != sk_running_proc_pid)
    {
      /* We're not in the currently-running thread, so fetch the
         registers using that thread's context and stack. */
      netapp_fetch_thread_registers (regno);
    }
  else
    {
      /* Save the current inferior PID, and set it to 0 so that we don't
	 try to fetch the registers from some per-thread register section,
	 because we don't have per-thread register sections.  */
      old_chain = save_inferior_ptid ();
      inferior_ptid = null_ptid;
      if (target_has_execution)
	remote_ops.to_fetch_registers (regno);
      else
	orig_core_ops.to_fetch_registers (regno);
      do_cleanups (old_chain);
    }
}

static void
netapp_thread_store_registers (int regno)
{
  struct cleanup *old_chain;
  char regs[REGISTER_BYTES];

  if (! ptid_equal (inferior_ptid, null_ptid)
      && PIDGET (inferior_ptid) != sk_running_proc_pid)
    {
      /* We're not in the currently-running thread, so fetch the
         registers using that thread's context and stack. */
#if 0
      netapp_fetch_thread_registers (regno, regs);
#else
      internal_error (__FILE__, __LINE__, "failed internal consistency check");	/* XXX - implement me */
#endif
    }
  else
    {
      /* Save the current inferior PID, and set it to 0 so that we don't
	 try to fetch the registers from some per-thread register section,
	 because we don't have per-thread register sections.  */
      old_chain = save_inferior_ptid ();
      inferior_ptid = null_ptid;
      if (target_has_execution)
	remote_ops.to_fetch_registers (regno);
      else
	orig_core_ops.to_fetch_registers (regno);
      do_cleanups (old_chain);
    }
}

/* Build a thread list from the "sk" process table. */

static void
netapp_find_new_threads (void)
{
  int i;
  int sk_nprocs;
  int pid;

  /* XXX - for now, don't do this on live appliances. */
  if (target_has_execution)
    return;

  /* If we already got the PID of the currently running process, we've
     already fetched the threads, and, as we're looking at a core dump,
     once we've fetched them once, there's no need to fetch them again. */
  if (scanned_for_threads)
    return;
  scanned_for_threads = 1;

  /* Get the size of the process table. */
  sk_nprocs = netapp_get_sk_nprocs ();

  /* Get the address of the process table. */
  if (!netapp_get_sk_proc_table() || sk_proc_table == 0)
    {
      /* Failed to fetch the address, or it's NULL because it
         hasn't been allocated yet.
         Can't fetch the list of threads. */
      return;
    }

  /* Get the size of an entry in the process table. */
  netapp_get_sk_proc_size ();
  if (sk_proc_size == 0)
    {
      /* Failed to find the size of a process table entry.
         Can't fetch the list of threads. */
      return;
    }

  /* Loop through the process table looking for live processes, and make
     a thread for each one. */
  for (i = 0; i < sk_nprocs; i++)
    {
      pid = i + 1;	/* PIDs start at 1 */
      if (netapp_is_sk_proc (pid))
	{
	  /* Make a thread. */
	  add_thread (pid);
	}
    }

  /* Set "inferior_ptid" to the PID of the currently running process. */
  inferior_ptid = pid_to_ptid (netapp_get_sk_running_proc_pid ());
}

int
netapp_get_sk_running_proc_pid (void)
{
  struct minimal_symbol *sk_running_proc_symp;
  static CORE_ADDR sk_running_proc_addr;
  char buf[TARGET_PTR_BIT / TARGET_CHAR_BIT];
  CORE_ADDR sk_running_proc;

  /* Get the address of "sk_running_proc", if we don't already have it. */
  if (sk_running_proc_addr == 0)
    {
      sk_running_proc_symp = lookup_minimal_symbol ("sk_running_proc",
	NULL, (struct objfile *) NULL);
      if (sk_running_proc_symp == NULL)
        return 0;	/* "should not happen" - return error here? */
      sk_running_proc_addr = SYMBOL_VALUE_ADDRESS (sk_running_proc_symp);
    }

  /* Get the value of "sk_running_proc".
     XXX - if it returns a non-zero value, we should fail! */
  target_read_memory (sk_running_proc_addr, buf,
			  TARGET_PTR_BIT / TARGET_CHAR_BIT);

  sk_running_proc = extract_address (buf, TARGET_PTR_BIT / TARGET_CHAR_BIT);

  /* Now convert it to a PID. */
  sk_running_proc_pid = PROC_PTR_TO_PID(sk_running_proc);
  return sk_running_proc_pid;
}

/* Get the size of the process table.
   XXX - assumes an "int" on the netapp fits in an "int" on the
   machine debugging the netapp. */
static int
netapp_get_sk_nprocs (void)
{
  struct minimal_symbol *sk_nprocs_symp;
  CORE_ADDR sk_nprocs_addr;
  char buf[TARGET_INT_BIT / TARGET_CHAR_BIT];
  int sk_nprocs;

  /* Get the address of "sk_nprocs". */
  sk_nprocs_symp = lookup_minimal_symbol ("sk_nprocs", NULL,
	(struct objfile *) NULL);
  if (sk_nprocs_symp == NULL)
    {
      /* Symbol isn't defined; we have an older netapp software
         release that doesn't save the size of the process table
	 in the core dump.
	
	 Use the value from the old #define.
	
	 XXX - it went from 600 in 1.0 to 700 in 2.0, so we may
	 think it's bigger than it is on old core dumps. */
      sk_nprocs = /*1000*/700;	/* XXX - make this 700 again eventually,
      			   once we no longer have 3.0 cores that don't
			   have "sk_nprocs" */
    }
  else
    {
      sk_nprocs_addr = SYMBOL_VALUE_ADDRESS (sk_nprocs_symp);

      /* Get the value of "sk_nprocs".
	 XXX - if it returns a non-zero value, we should fail! */
      target_read_memory (sk_nprocs_addr, buf,
			  TARGET_INT_BIT / TARGET_CHAR_BIT);
      sk_nprocs = extract_signed_integer (buf, TARGET_INT_BIT / TARGET_CHAR_BIT);
    }
  return sk_nprocs;
}

/* Offset, in bytes, of the "sp" member of the "context" member of an
   "sk_Proc", relative to the beginning of an "sk_Proc". */
static int sp_offset;

/* Offset, in bytes, of the "state" member of an "sk_Proc", relative to
   the beginning of an "sk_Proc". */
static int state_offset;

/* Values of "SK_PROC_UNUSED" and "SK_PROC_IS_MSG" members of an
   "sk_Ptype" - and flag indicating whether there *is* an SK_PROC_IS_MSG
   member. */
static int sk_proc_unused;
static int sk_proc_is_msg;
static int sk_proc_is_msg_found;

/* Get the size of an entry in the process table, and also get various
   information about "sk_Proc" and "sk_Ptype" from the symbol table. */
static void
netapp_get_sk_proc_size (void)
{
  struct symbol *sk_Proc_sym, *sk_Ptype_sym;
  register struct type *sk_Proc_type, *offset_type, *sk_Ptype_type;
  int i;
  int context_found = 0;
  int state_found = 0;
  int sk_proc_unused_found = 0;

  sk_Proc_sym = lookup_symbol ("sk_Proc", (struct block *) NULL,
		       VAR_NAMESPACE, 0, (struct symtab **) NULL);
  if (sk_Proc_sym == NULL)
    {
      warning ("sk_Proc type not found; can't find sk processes.");
      return;
    }
  sk_Proc_type = check_typedef (SYMBOL_TYPE (sk_Proc_sym));
  if (TYPE_CODE (sk_Proc_type) != TYPE_CODE_STRUCT)
    {
      warning ("sk_Proc is not a struct; can't find sk processes.");
      return;
    }

  /* Look for the offsets of "context" and "state" in an "sk_Proc". */
  for (i = TYPE_NFIELDS (sk_Proc_type) - 1;
		i >= TYPE_N_BASECLASSES (sk_Proc_type); i--)
    {
      if (strcmp(TYPE_FIELD_NAME (sk_Proc_type, i), "context") == 0)
	{
	  /* Found the "context" member.  Is it some C++ "static member"? */
	  if (TYPE_FIELD_STATIC (sk_Proc_type, i))
	    {
	      warning ("\"context\" member of sk_Proc is weird; can't find sk processes.");
	      return;
	    }

	  /* Its offset, in bits, from the beginning of an "sk_Proc"
	     is TYPE_FIELD_BITPOS (sk_Proc_type, i).
	     Compute its offset in bytes. */
	  sp_offset = TYPE_FIELD_BITPOS (sk_Proc_type, i)/8;

	  /* Also, remember its type, so we can find the offset of the
	     "sp" member of that structure. */
	  offset_type = check_typedef (TYPE_FIELD_TYPE (sk_Proc_type, i));

	  /* Make sure it *is* a structure. */
	  if (TYPE_CODE (offset_type) != TYPE_CODE_STRUCT)
	    {
	      warning ("\"context\" member of sk_Proc is not a struct; can't find sk processes.");
	      return;
	    }
	  context_found = 1;
	}
      else if (strcmp(TYPE_FIELD_NAME (sk_Proc_type, i), "state") == 0)
	{
	  /* Found the "state" member.  Is it some C++ "static member"? */
	  if (TYPE_FIELD_STATIC (sk_Proc_type, i))
	    {
	      warning ("\"state\" member of sk_Proc is weird; can't find sk processes.");
	      return;
	    }

	  /* Its offset, in bits, from the beginning of an "sk_Proc"
	     is TYPE_FIELD_BITPOS (sk_Proc_type, i).
	     Compute its offset in bytes. */
	  state_offset = TYPE_FIELD_BITPOS (sk_Proc_type, i)/8;

	  /* XXX - remember the type and size of "state"? */
	  state_found = 1;
	}
    }

  if (!context_found)
    {
      /* We didn't find a "context" member. */
      warning ("sk_Proc has no \"context\" member; can't find sk processes.");
      return;
    }

  if (!state_found)
    {
      /* We didn't find a "state" member. */
      warning ("sk_Proc has no \"state\" member; can't find sk processes.");
      return;
    }

  /* Find the offset of the "sp" member of "context". */
  for (i = TYPE_NFIELDS (offset_type) - 1;
		i >= TYPE_N_BASECLASSES (offset_type); i--)
    {
      if (strcmp(TYPE_FIELD_NAME (offset_type, i), "sp") == 0)
	{
	  /* Found the "sp" member.  Is it some C++ "static member"? */
	  if (TYPE_FIELD_STATIC (offset_type, i))
	    {
	      warning ("\"sp\" member of sk_Proc.context is weird; can't find sk processes.");
	      return;
	    }

	  /* Its offset, in bits, from the beginning of "context"
	     is TYPE_FIELD_BITPOS (offset_type, i).
	     Compute its offset in bytes, and add it to "sp_offset". */
	  sp_offset += TYPE_FIELD_BITPOS (offset_type, i)/8;

	  /* XXX - remember the size of "sp"? */
	  goto found;
	}
    }

    /* We didn't find an "sp" member. */
    warning ("sk_Proc.context has no \"sp\" member; can't find sk processes.");
    return;

found:
  /* Now find the values of the "SK_PROC_UNUSED" and "SK_PROC_IS_MSG"
     members of "sk_Pstate", which is an "enum". */
  sk_Ptype_sym = lookup_symbol ("sk_Pstate", (struct block *) NULL,
		       VAR_NAMESPACE, 0, (struct symtab **) NULL);
  if (sk_Ptype_sym == NULL)
    {
      warning ("sk_Ptype type not found; can't find sk processes.");
      return;
    }
  sk_Ptype_type = check_typedef (SYMBOL_TYPE (sk_Ptype_sym));
  if (TYPE_CODE (sk_Ptype_type) != TYPE_CODE_ENUM)
    {
      warning ("sk_Ptype is not an enum; can't find sk processes.");
      return;
    }

  /* Look for the values of "SK_PROC_UNUSED" and "SK_PROC_IS_MSG" in
     "sk_Ptype". */
  for (i = TYPE_NFIELDS (sk_Ptype_type) - 1; i >= 0; i--)
    {
      if (strcmp(TYPE_FIELD_NAME (sk_Ptype_type, i), "SK_PROC_UNUSED") == 0)
	{
	  /* Its value, as an integral value, is
	     TYPE_FIELD_BITPOS (sk_Ptype_type, i). */
	  sk_proc_unused = TYPE_FIELD_BITPOS (sk_Ptype_type, i);
	  sk_proc_unused_found = 1;
	}
      else if (strcmp(TYPE_FIELD_NAME (sk_Ptype_type, i), "SK_PROC_IS_MSG") == 0)
	{
	  /* Its value, as an integral value, is
	     TYPE_FIELD_BITPOS (sk_Ptype_type, i). */
	  sk_proc_is_msg = TYPE_FIELD_BITPOS (sk_Ptype_type, i);
	  sk_proc_is_msg_found = 1;
	}
    }

  if (!sk_proc_unused_found)
    {
      /* We didn't find an "SK_PROC_UNUSED" member. */
      warning ("sk_Ptype has no \"SK_PROC_UNUSED\" member; can't find sk processes.");
      return;
    }

  /* Set "sk_proc_size" to the length of an "sk_Proc". */
  sk_proc_size = (SYMBOL_TYPE (sk_Proc_sym))->length;
}

/* Get the address of "sk_proc_table".  On success, returns 1 and sets
   "sk_proc_table"; on failure, returns 0. */
static int
netapp_get_sk_proc_table (void)
{
  struct minimal_symbol *sk_proc_table_symp;
  CORE_ADDR sk_proc_table_addr;
  char buf[TARGET_PTR_BIT / TARGET_CHAR_BIT];

  /* Get the address of "sk_proc_table". */
  sk_proc_table_symp = lookup_minimal_symbol ("sk_proc_table", NULL,
	(struct objfile *) NULL);
  if (sk_proc_table_symp == NULL)
    return 0;		/* not defined? */

  sk_proc_table_addr = SYMBOL_VALUE_ADDRESS (sk_proc_table_symp);

  /* Get the value of "sk_proc_table".
     XXX - if it returns a non-zero value, we should fail! */
  target_read_memory (sk_proc_table_addr, buf,
			  TARGET_PTR_BIT / TARGET_CHAR_BIT);
  sk_proc_table = extract_address (buf, TARGET_PTR_BIT / TARGET_CHAR_BIT);

  if (sk_proc_table == 0)
    return 0;		/* not yet allocated */

  return 1;
}

/* Return TRUE if there's an "sk" process with the PID "pid". */
static int
netapp_is_sk_proc (int pid)
{
  int state;
  char buf[TARGET_INT_BIT / TARGET_CHAR_BIT];
  CORE_ADDR proc_ptr;
 
  /* Find the address of the process. */
  proc_ptr = PID_TO_PROC_PTR(pid);

  /* Fetch the "state" member of the "sk_Proc" for the process in
     question. */
  if (target_read_memory (proc_ptr + state_offset, buf,
			  TARGET_INT_BIT / TARGET_CHAR_BIT))
    return 0;

  state = extract_signed_integer (buf,
			TARGET_INT_BIT / TARGET_CHAR_BIT);

  /* There's a process there if the state is neither SK_PROC_UNUSED
     or SK_PROC_IS_MSG (assuming there *is* an SK_PROC_IS_MSG state). */
  return (state == sk_proc_unused
    || (sk_proc_is_msg_found && state == sk_proc_is_msg)) ? 0: 1;
}

/* Get the SP for a given "sk" process with the PID "pid". */
CORE_ADDR
netapp_get_proc_sp (int pid)
{
  char buf[TARGET_PTR_BIT / TARGET_CHAR_BIT];
  CORE_ADDR proc_ptr;
 
  /* Find the address of the process. */
  proc_ptr = PID_TO_PROC_PTR(pid);
 
  /* Fetch the "sp" member "context" member of the "sk_Proc" for the
     process in question.
     XXX - if it returns a non-zero value, we should fail! */
  target_read_memory (proc_ptr + sp_offset, buf,
			  TARGET_PTR_BIT / TARGET_CHAR_BIT);

  return extract_address (buf, TARGET_PTR_BIT / TARGET_CHAR_BIT);
}

static void
netapp_core_detach (char *args, int from_tty)
{
  unpush_target (&core_ops);
  orig_core_ops.to_detach (args, from_tty);
}

/* Fork an inferior process, and start debugging it with /proc.  */

static void
netapp_thread_create_inferior (char *exec_file, char *allargs, char **env)
{
  remote_ops.to_create_inferior (exec_file, allargs, env);

  push_target (&netapp_thread_ops);
}

/* Clean up after the inferior dies.  */

static void
netapp_thread_mourn_inferior (void)
{
  unpush_target (&netapp_thread_ops);
  remote_ops.to_mourn_inferior ();
}

static void
netapp_thread_core_detach (char *args, int from_tty)
{
  unpush_target (&core_ops);
  orig_core_ops.to_detach (args, from_tty);
}

static void
init_netapp_thread_ops (void)
{
  netapp_thread_ops = remote_ops;
  netapp_thread_ops.to_shortname = "netapp-threads";
  netapp_thread_ops.to_longname = "Netapp threads.";
  netapp_thread_ops.to_doc = "NetApp thread support.";
  netapp_thread_ops.to_detach = netapp_thread_detach;
  netapp_thread_ops.to_create_inferior = netapp_thread_create_inferior;
  netapp_thread_ops.to_mourn_inferior = netapp_thread_mourn_inferior;
  netapp_thread_ops.to_fetch_registers = netapp_thread_fetch_registers;
  netapp_thread_ops.to_store_registers = netapp_thread_store_registers;
  netapp_thread_ops.to_pid_to_str = netapp_pid_to_str;
  netapp_thread_ops.to_find_new_threads = netapp_find_new_threads;
}

static void
init_netapp_core_ops (void)
{
  netapp_core_ops = core_ops;
  netapp_core_ops.to_shortname = "netapp-core";
  netapp_core_ops.to_longname = "NetApp core threads.";
  netapp_core_ops.to_doc = "NetApp thread support for core files.";
  netapp_core_ops.to_detach = netapp_thread_core_detach;
  netapp_core_ops.to_fetch_registers = netapp_thread_fetch_registers;
  netapp_core_ops.to_pid_to_str = netapp_pid_to_str;
  netapp_core_ops.to_find_new_threads = netapp_find_new_threads;
}

/* we suppress the call to add_target of core_ops in corelow because
   if there are two targets in the stratum core_stratum, find_core_target
   won't know which one to return.  see corelow.c for an additonal
   comment on coreops_suppress_target. */
int coreops_suppress_target = 1;

void
_initialize_ntap_thread (void)
{
  init_netapp_thread_ops();
  init_netapp_core_ops();

  add_target (&netapp_thread_ops);

  memcpy(&orig_core_ops, &core_ops, sizeof (struct target_ops));
  memcpy(&core_ops, &netapp_core_ops, sizeof (struct target_ops));
  add_target (&core_ops);
}
