/* Subroutines for ARM basica block profiling.
   Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.
   
   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.
   
   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* This code is stolen from gcc/libgcc2.c and simplified/adapted to
   work for the ARM port, where a target C library may not be available.
   
   This mandates some changes.  For example there may not be a channel
   for reporting errors, so instead we call an exported function (which
   can be overridden), which sets a global variable.  The idea being that
   a debugger could put a breakpoint on the function or a watch point on
   the variable.

   In addition a file system may not be present, so we may have to
   store the data in memory.  We seperate out the file operations into
   special functions which can be overridden by the user or another
   library, if necessary.  */

/* Simple minded basic block profiling output dumper
   for systems that don't provide gcov support.  */

#define inhibit_libc

#include "tconfig.h"
#include "tsystem.h"

/* Typedefs.  */

typedef int         bool;
typedef long long   gcov_type;

/* Structures emitted by --profile-arcs.  */
typedef struct bb_func_info
{
  long 			checksum;
  int			arc_count;
  const char *		name;
}
bb_func_info;

typedef struct bb
{
  long                  zero_word;
  const char *          filename;
  gcov_type *           counts;
  long                  ncounts;
  struct bb *           next;

  long			sizeof_bb;
  bb_func_info *        function_infos;
}
bb;


/* Prototypes.  */

static unsigned int arm_bb_strlen     PARAMS ((const char *));
static bool         read_gcov_type    PARAMS ((int, gcov_type *));
static bool         write_gcov_type   PARAMS ((int, gcov_type));
static bool         write_gcov_string PARAMS ((int, const char *, unsigned int, signed int));
static bool         load_da_file      PARAMS ((int, bb *, int));
static bool         arm_bb_read_and_verify PARAMS ((int, signed long));
static bool         read_long         PARAMS ((int, long *));
static bool         write_long        PARAMS ((int, long));

/* Make the exported functions weak, so that they
   can be overriden by the user if need be.  */
#ifdef OBJECT_FORMAT_ELF
#define WEAK __attribute__((weak));
#else
#define WEAK
#endif

void __bb_exit_func       PARAMS ((void)) WEAK;
void __bb_init_func       PARAMS ((bb * const)) WEAK;
void __bb_fork_func       PARAMS ((void)) WEAK;

int  arm_bb_open          PARAMS ((const char * const)) WEAK;
bool arm_bb_read          PARAMS ((int, const void *, int)) WEAK;
bool arm_bb_write         PARAMS ((int, const void * const, int)) WEAK;
bool arm_bb_rewind 	  PARAMS ((int)) WEAK;
bool arm_bb_seek 	  PARAMS ((int, int, int)) WEAK;
bool arm_bb_close 	  PARAMS ((int)) WEAK;
void arm_bb_set_error 	  PARAMS ((int)) WEAK;

/* Use the sycall interface to avoid requiring a C library.  */
extern int  _close	  PARAMS ((int));
extern int  _open	  PARAMS ((const char * const, int, ...));
extern int  _write 	  PARAMS ((int, const void * const, int));
extern int  _lseek	  PARAMS ((int, int, int));
extern int  _read	  PARAMS ((int, const void *, int));

/* Variables.  */

static bb *	bb_head;

/* Global so that debuggers can see and watch it.  */
int		arm_bb_error;


/* Functions.  */

static unsigned int
arm_bb_strlen (const char * str)
{
  unsigned int len = 0;

  while (* str ++)
    len ++;

  return len;
}

int
arm_bb_open (const char * const filename)
{
  /* 2 is O_RDWR and 02000 is O_APPEND in target format.  */
  return _open (filename, 2 | 02000);
}

bool
arm_bb_read (int file, const void * buf, int length)
{
  return _read (file, buf, length) == length;
}

bool
arm_bb_write (int file, const void * const buf, int length)
{
  return _write (file, buf, length) == length;
}

bool
arm_bb_rewind (int file)
{
  return _lseek (file, 0, 0) != -1;
}

bool
arm_bb_seek (int file, int offset, int whence)
{
  return _lseek (file, offset, whence) != -1;
}

bool
arm_bb_close (int file)
{
  return _close (file) == 0;
}

void
arm_bb_set_error (int error)
{
  arm_bb_error = error;
  
  switch (error)
    {
    case 0: _write (1, "arm_bb_error 0\n", 15); break;
    case 1: _write (1, "arm_bb_error 1\n", 15); break;
    case 2: _write (1, "arm_bb_error 2\n", 15); break;
    case 3: _write (1, "arm_bb_error 3\n", 15); break;
    case 4: _write (1, "arm_bb_error 4\n", 15); break;
    case 5: _write (1, "arm_bb_error 5\n", 15); break;
    case 6: _write (1, "arm_bb_error 6\n", 15); break;
    case 7: _write (1, "arm_bb_error 7\n", 15); break;
    case 8: _write (1, "arm_bb_error 8\n", 15); break;
    case 9: _write (1, "arm_bb_error 9\n", 15); break;
    default:_write (1, "arm_bb_error ?\n", 15); break;
    }
}

/* The next five routines were stolen and then adapted from gcov-io.h.  */

static bool
read_long (int file, long * dest)
{
  long           value = 0;
  int            i;
  unsigned char  c[10];

  if (! arm_bb_read (file, c, 4))
    return 0;

  value = c [3] & 127;

  for (i = 2; i >= 0; i --)
    value = value * 256 + c [i];

  if ((c [3] & 128) && (value > 0))
    value = - value;

  * dest = value;

  return 1;
}

static bool
read_gcov_type (int file, gcov_type * buffer)
{
  gcov_type      value = 0;
  int            i;
  unsigned char  c[10];

  if (! arm_bb_read (file, c, 8))
    return 0;

  value = c [7] & 127;

  for (i = 6; i >= 0; i --)
    value = value * 256 + c [i];

  if ((c [7] & 128) && (value > 0))
    value = - value;

  * buffer = value;

  return 1;
}

static int
write_long (int file, long value)
{
  char c[10];
  int  upper_bit = (value < 0 ? 128 : 0);
  int  i;

  if (value < 0)
    value = - value;

  for (i = 0; i < 4; i++)
    {
      c [i] = value & (i == 3 ? 127 : 255);
      value = value / 256;
    }

  if (value && value != -1)
    return 0;

  for (; i < 4; i ++)
    c [i] = 0;

  c [3] |= upper_bit;

  return arm_bb_write (file, c, 4);
}

static bool
write_gcov_type (int file, gcov_type value)
{
  char c [10];
  int  upper_bit = (value < 0 ? 128 : 0);
  int  i;

  if (value < 0)
    value = - value;

  for (i = 0 ; i < 8; i++)
    {
      c [i] = value & (i == 7 ? 127 : 255);
      value /= 256;
    }

  if (value && value != -1)
    return 0;

  for (; i < 8; i++) 
    c [i] = 0;

  c [7] |= upper_bit;

  return arm_bb_write (file, c, 8);
}

static int
write_gcov_string (int file, const char * string, unsigned int length, signed int delim)
{
  /* Write first delimiter.  */
  if (! write_long (file, delim))
    return 0;

  /* Write the length.  */
  if (! write_long (file, length))
    return 0;

  /* Write the string (extending the length by 1 to include the nul byte.  */
  if (! arm_bb_write (file, string, ++ length))
    return 0;

  /* Pad the string to a four byte boundary.  */
  length &= 3;

  if (length)
    {
      char c[4];

      c[0] = c[1] = c[2] = c[3] = 0;

      if (! arm_bb_write (file, c, 4 - length))
	return 0;
    }

  /* Write the final delimiter.  */
  return write_long (file, delim);
}


static bool
arm_bb_read_and_verify (int file, signed long value)
{
  signed long datum;

  if (! read_long (file, & datum))
    return 0;

  return datum == value;
}


static int
load_da_file (int da_file, bb * ptr, int object_funcs)
{
  long            n_funcs = 0;
  signed long     datum;
  gcov_type *     count_ptr;
  bb_func_info *  fn_info;
  int             i;

  arm_bb_rewind (da_file);

  /* Check the magic number.  */
  if (! arm_bb_read_and_verify (da_file, -123))
    {
      arm_bb_set_error (1);
      return 0;
    }

  /* Get the number of functions.  */
  if (! read_long (da_file, & n_funcs))
    {
      arm_bb_set_error (2);
      return 0;
    }

  /* If the number of functions in this object file is different from
     last time, do not merge anything, but do not complain either.  */
  if (n_funcs != object_funcs)
    return 1;

  /* Get the length of the extension block.  */
  if (! read_long (da_file, & datum))
    {
      arm_bb_set_error (3);
      return 0;
    }

  /* Skip the extension block.  */
 if (! arm_bb_seek (da_file, datum, 1))
   {
      arm_bb_set_error (4);
      return 0;
   }

  /* Merge execution counts for each function.  */
  count_ptr = ptr->counts;

  for (fn_info = ptr->function_infos;
       fn_info->arc_count != -1;
       fn_info ++)
    {
      /* Check the function name delimiter.  */
      if (! arm_bb_read_and_verify (da_file, -1))
	{
	  arm_bb_set_error (5);
	  return 0;
	}

      /* Read the function name length.  */
      datum = (signed long) arm_bb_strlen (fn_info->name);
      if (! arm_bb_read_and_verify (da_file, datum))
	{
	  arm_bb_set_error (6);
	  return 0;
	}

      /* Skip the function name.  */
      if (! arm_bb_seek (da_file, ((datum + 1) + 3) & ~3, 1))
	{
	  arm_bb_set_error (7);
	  return 0;
	}

      /* Check the other function name delimiter.  */
      if (! arm_bb_read_and_verify (da_file, -1))
	{
	  arm_bb_set_error (8);
	  return 0;
	}

      /* Check the function checksum.  */
      if (! arm_bb_read_and_verify (da_file, fn_info->checksum))
	{
	  arm_bb_set_error (9);
	  return 0;
	}

      /* Check the function's arc count.  */
      if (! arm_bb_read_and_verify (da_file, fn_info->arc_count))
	{
	  arm_bb_set_error (0);
	  return 0;
	}

      /* Read and accumulate the arc counts.  */
      for (i = fn_info->arc_count; i > 0; i--, count_ptr ++)
	{
	  gcov_type v;

	  if (! read_gcov_type (da_file, & v))
	    {
	      arm_bb_set_error (1);
	      return 0;
	    }

	  * count_ptr += v;
	}
    }

  return 1;
}


void
__bb_exit_func (void)
{
  bb *          ptr;
  int           i;
  gcov_type     program_sum = 0;
  gcov_type     program_max = 0;
  long          program_arcs = 0;
  gcov_type     merged_sum = 0;
  gcov_type     merged_max = 0;
  long          merged_arcs = 0;

  if (bb_head == NULL)
    return;

  /* Compute overall sums.  */
  for (ptr = bb_head; ptr != NULL; ptr = ptr->next)
    {
      /* Verify the filename.  */
      i = arm_bb_strlen (ptr->filename) - 3;

      if (   ptr->filename [i + 0] != '.'
	  || ptr->filename [i + 1] != 'd'
	  || ptr->filename [i + 2] != 'a')
	{
	  /* FIXME: Basic block profiling code not yet written.  */
	  arm_bb_set_error (1);
	  continue;
	}

      for (i = 0; i < ptr->ncounts; i++)
	{
	  program_sum += ptr->counts[i];

	  if (ptr->counts[i] > program_max)
	    program_max = ptr->counts[i];
	}

      program_arcs += ptr->ncounts;
    }

  /* Dump the data in a form that gcov expects.  */
  for (ptr = bb_head; ptr != NULL; ptr = ptr->next)
    {
      bb_func_info * fn_info;
      signed long    datum;
      int            da_file;
      gcov_type      object_max = 0;
      gcov_type      object_sum = 0;
      long           object_functions = 0;
      bool           merging = 0;
      gcov_type *    count_ptr;

      da_file = arm_bb_open (ptr->filename);
      if (da_file < 1)
	{
	  arm_bb_set_error (0);
	  continue;
	}

      /* Count the number of functions in this particular object file.  */
      for (fn_info = ptr->function_infos; fn_info->arc_count != -1; fn_info ++)
	object_functions ++;

      /* If the file is not empty try to merge in the counts.  */
      if (read_long (da_file, & datum)
	  && load_da_file (da_file, ptr, object_functions))
	merging = 1;

      arm_bb_rewind (da_file);

      /* Calculate the per-object statistics.  */
      for (i = 0; i < ptr->ncounts; i ++)
	{
	  object_sum += ptr->counts[i];

	  if (ptr->counts[i] > object_max)
	    object_max = ptr->counts[i];
	}

      /* Calculate the global statistics.  */
      merged_sum += object_sum;
      if (merged_max < object_max)
	merged_max = object_max;
      merged_arcs += ptr->ncounts;

      /* Write the .da file, starting with the header.  */
      /* Magic number.  */
      datum = -123L;
      if (! write_long (da_file, datum))
	{
	  arm_bb_set_error (1);
	  arm_bb_close (da_file);
	  continue;
	}

      /* Number of functions.  */
      if (! write_long (da_file, object_functions))
	{
	  arm_bb_set_error (2);
	  arm_bb_close (da_file);
	  continue;
	}

      /* Length of extension block.  */
      datum = (4 + 8 + 8) + (4 + 8 + 8);
      if (! write_long (da_file, datum))
	{
	  arm_bb_set_error (3);
	  arm_bb_close (da_file);
	  continue;
	}

      /* Whole program statistics. If merging write
	 per-object now, rewrite later.  */
      /* Number of instrumented arcs.  */
      datum = merging ? ptr->ncounts : program_arcs;
      if (! write_long (da_file, datum))
	{
	  arm_bb_set_error (4);
	  arm_bb_close (da_file);
	  continue;
	}

      /* Sum of counters.  */
      if (! write_gcov_type (da_file, merging ? object_sum : program_sum))
	{
	  arm_bb_set_error (5);
	  arm_bb_close (da_file);
	  continue;
	}

      /* Maximal counter.  */
      if (! write_gcov_type (da_file, merging ? object_max : program_max))
	{
	  arm_bb_set_error (6);
	  arm_bb_close (da_file);
	  continue;
	}

      /* Per-object statistics.  */
      /* Number of counters.  */
      if (! write_long (da_file, ptr->ncounts))
	{
	  arm_bb_set_error (7);
	  arm_bb_close (da_file);
	  continue;
	}

      /* Sum of counters.  */
      if (! write_gcov_type (da_file, object_sum))
	{
	  arm_bb_set_error (8);
	  arm_bb_close (da_file);
	  continue;
	}

      /* Maximal counter.  */
      if (! write_gcov_type (da_file, object_max))
	{
	  arm_bb_set_error (9);
	  arm_bb_close (da_file);
	  continue;
	}

      /* Write execution counts for each function.  */
      count_ptr = ptr->counts;

      for (fn_info = ptr->function_infos;
	   fn_info->arc_count != -1;
	   fn_info ++)
	{
	  if (! write_gcov_string (da_file, fn_info->name,
				   strlen (fn_info->name), -1))
	    {
	      arm_bb_set_error (10);
	      break;
	    }

	  if (! write_long (da_file, fn_info->checksum))
	    {
	      arm_bb_set_error (11);
	      break;
	    }

	  if (! write_long (da_file, fn_info->arc_count))
	    {
	      arm_bb_set_error (12);
	      break;
	    }

	  for (i = fn_info->arc_count; i > 0; i--, count_ptr ++)
	    if (! write_gcov_type (da_file, * count_ptr))
	      {
		arm_bb_set_error (13);
		break;
	      }
	}

      if (! arm_bb_close (da_file))
	arm_bb_set_error (7);
    }

  /* Upate whole program statistics.  */
  for (ptr = bb_head; ptr; ptr = ptr->next)
    {
      int da_file;

      if (! ptr->filename)
	continue;

      da_file = arm_bb_open (ptr->filename);
      if (da_file < 1)
	{
	  arm_bb_set_error (0);
	  arm_bb_close (da_file);
	  continue;
	}

      if (! arm_bb_seek (da_file, 4 * 3, 0))
	{
	  arm_bb_set_error (1);
	  arm_bb_close (da_file);
	  continue;
	}

      /* Number of instrumented arcs.  */
      if (! write_long (da_file, merged_arcs))
	{
	  arm_bb_set_error (2);
	  arm_bb_close (da_file);
	  continue;
	}

      /* Sum of counters.  */
      if (! write_gcov_type (da_file, merged_sum))
	{
	  arm_bb_set_error (3);
	  arm_bb_close (da_file);
	  continue;
	}

      /* Maximal counter.  */
      if (! write_gcov_type (da_file, merged_max))
	arm_bb_set_error (4);

      if (! arm_bb_close (da_file))
	arm_bb_set_error (5);
    }
}

void
__bb_init_func (bb * const blocks)
{
  /* User is supposed to check whether the first
     word is non-0, but just in case...  */
  if (blocks->zero_word)
    return;

  /* Initialize destructor.  */
  if (! bb_head)
    atexit (__bb_exit_func);

  /* Set up linked list.  */
  blocks->zero_word = 1;
  blocks->next = bb_head;
  bb_head = blocks;
}

/* Called before fork or exec - write out profile information gathered
   so far and reset it to zero.  This avoids duplication or loss of the
   profile information gathered so far.  */

void
__bb_fork_func (void)
{
  long i;
  bb * ptr;

  __bb_exit_func ();

  for (ptr = bb_head; ptr != (bb *) 0; ptr = ptr->next)
    for (i = ptr->ncounts - 1; i >= 0; i --)
      ptr->counts [i] = 0;
}
