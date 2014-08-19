/* main.c --- main function for stand-alone AArch64 simulator.

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
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <stdarg.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "bfd.h"
#include "dis-asm.h"
#include "libiberty.h"

#include "cpustate.h"
#include "err.h"
#include "memory.h"
#include "simulator.h"

static struct option
sim_options[] =
{
  { "end-sim-args", 0, NULL, 'E' },
  { 0, 0, 0, 0 }
};

static struct disassemble_info  info;
static unsigned long            symcount = 0;
static asymbol **               symtab = NULL;

static char opbuf[1000];

static int
op_printf (char *buf, char *fmt, ...)
{
  int ret;
  va_list ap;

  va_start (ap, fmt);
  ret = vsprintf (opbuf + strlen (opbuf), fmt, ap);
  va_end (ap);
  return ret;
}

static int
sim_dis_read (bfd_vma                     memaddr,
	      bfd_byte *                  ptr,
	      unsigned int                length,
	      struct disassemble_info *   info)
{
  aarch64_get_mem_blk (memaddr, (char *) ptr, length);
  return 0;
}

/* Filter out (in place) symbols that are useless for disassembly.
   COUNT is the number of elements in SYMBOLS.
   Return the number of useful symbols. */

static unsigned long
remove_useless_symbols (asymbol ** symbols, unsigned long count)
{
  asymbol ** in_ptr  = symbols;
  asymbol ** out_ptr = symbols;

  while (count-- > 0)
    {
      asymbol *sym = *in_ptr++;

      if (strstr (sym->name, "gcc2_compiled"))
	continue;
      if (sym->name == NULL || sym->name[0] == '\0')
	continue;
      if (sym->flags & (BSF_DEBUGGING))
	continue;
      if (   bfd_is_und_section (sym->section)
	  || bfd_is_com_section (sym->section))
	continue;
      if (sym->name[0] == '$')
	continue;
      
      *out_ptr++ = sym;
    }
  return out_ptr - symbols;
}

static signed int
compare_symbols (const void * ap, const void * bp)
{
  const asymbol *a = * (const asymbol **) ap;
  const asymbol *b = * (const asymbol **) bp;

  if (bfd_asymbol_value (a) > bfd_asymbol_value (b))
    return 1;
  if (bfd_asymbol_value (a) < bfd_asymbol_value (b))
    return -1;
  return 0;
}

// Find the name of the function at ADDR.
const char *
aarch64_print_func (u_int64_t addr)
{
  int  min, max;

  min = -1;
  max = symcount;
  while (min < max - 1)
    {
      int sym;
      bfd_vma sa;

      sym = (min + max) / 2;
      sa = bfd_asymbol_value (symtab[sym]);

      if (sa > addr)
	max = sym;
      else if (sa < addr)
	min = sym;
      else
	{
	  min = sym;
	  break;
	}
    }

  if (min != -1)
    return bfd_asymbol_name (symtab [min]);

  return "";
}

void
aarch64_print_insn (u_int64_t addr)
{
  int size;

  opbuf[0] = 0;
  size = print_insn_aarch64 (addr, & info);
  fprintf (stderr, " %*s ", size, opbuf);
}

u_int64_t
aarch64_get_sym_value (const char * name)
{
  unsigned long i;

  for (i = 0; i < symcount; i++)
    if (strcmp (bfd_asymbol_name (symtab[i]), name) == 0)
      return bfd_asymbol_value (symtab[i]);

  return 0;
}

int
main (int argc, char **argv)
{
  int o;
  bfd *prog;
  int rc;

  if (argc < 1)
    exit (-1);

  while ((o = getopt_long (argc, argv, "Edfhmrtz", sim_options, NULL)) != -1)
    {
      if (o == 'E')
	/* Stop processing the command line. This is so that any remaining
	   words on the command line that look like arguments will be passed
	   on to the program being simulated.  */
	break;

      else switch (o)
	{
	case 't':
	  trace = TRACE_ALL;
	  break;
	case 'f':
	  trace |= TRACE_FUNCTIONS;
	  break;
	case 'm':
	  trace |= TRACE_MEM_WRITES;
	  break;
	case 'r':
	  trace |= TRACE_REG_WRITES;
	  break;
	case 'z':
	  trace |= TRACE_MISC;
	  break;

	case 'd':
	  disas = 1;
	  break;
	case 'h':
	case '?':
	  fprintf (stderr,
		   "usage: run [options] program [arguments]\n"
		   "\t-t\t- trace (all).\n"
		   "\t-f\t- trace (functions).\n"
		   "\t-m\t- trace (memory writes).\n"
		   "\t-r\t- trace (register changes).\n"
		   "\t-z\t- trace (misc).\n"
		   "\t-d\t- disassemble.\n"
		   "\t-E\t- stop processing sim args\n");
	    exit (1);
	}
    }
  
  prog = bfd_openr (argv[optind], 0);
  if (prog == NULL)
    {
      fprintf (stderr, "Can't read %s\n", argv[optind]);
      exit (-1);
    }

  if (!bfd_check_format (prog, bfd_object))
    {
      fprintf (stderr, "%s not an AArch64 program\n", argv[optind]);
      exit (-1);
    }

  if (aarch64_load (prog) == FALSE)
    /* An error message will have already been displayed.  */
    exit (-1);

  long storage;

  memset (& info, 0, sizeof (info));
  INIT_DISASSEMBLE_INFO (info, stdout, op_printf);
  info.read_memory_func = sim_dis_read;
  info.arch = bfd_get_arch (prog);
  info.mach = bfd_get_mach (prog);
  if (info.mach == 0)
    info.arch = bfd_arch_aarch64;
  disassemble_init_for_target (& info);

  storage = bfd_get_symtab_upper_bound (prog);
  if (storage > 0)
    {
      symtab = (asymbol **) xmalloc (storage);
      symcount = bfd_canonicalize_symtab (prog, symtab);
      symcount = remove_useless_symbols (symtab, symcount);
      qsort (symtab, symcount, sizeof (asymbol *), compare_symbols);
    }

  if (! aarch64_init ())
    {
      fprintf (stderr, "%s: failed to initialise sim\n", argv[optind]);
      exit (-1);
    }
    
  rc = aarch64_run ();

  switch (rc)
    {
    case STATUS_RETURN:   exit (0);
    default:              fprintf (stderr, "sim: unknown result: %d\n", rc); exit (1);
    case STATUS_ERROR:    fprintf (stderr, "sim: error: %s\n", aarch64_get_error_text ()); exit (1);
    case STATUS_CALLOUT:  fprintf (stderr, "sim: callout\n"); exit (2);
    case STATUS_READY:	  fprintf (stderr, "sim: unexpecetd ready\n"); exit (3);
    case STATUS_HALT:     fprintf (stderr, "sim: halt\n"); exit (4);
    case STATUS_BREAK:    fprintf (stderr, "sim: break\n"); exit (5);
    }
}
