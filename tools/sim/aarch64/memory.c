/* memory.c -- Memory accessor functions for the AArch64 simulator

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
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bfd.h"
#include "libbfd.h"
#include "libiberty.h"
#include "elf/internal.h"
#include "elf/common.h"

#include "memory.h"
#include "simulator.h"

static bfd * saved_prog = NULL;

typedef struct mem_blk
{
  u_int64_t         address;		/* Byte address of start of block.  */
  u_int64_t         end_address;	/* Byte address of first byte beyond the end of the block.  */
  u_int64_t         size;		/* Number of bytes in the block.  */
  char *            buf;                /* The actual block.  */
  bfd_boolean       can_write;		/* True if writes to the block are permitted.  */
  struct mem_blk *  next;
} mem_blk;
  

static mem_blk * first_blk = NULL;
static mem_blk * active_blk = NULL;

void
mem_add_blk (u_int64_t address, char *bufptr, u_int64_t nbytes, bfd_boolean can_write)
{
  mem_blk * blk = xmalloc (sizeof * blk);

  blk->address     = address;
  blk->size        = nbytes;
  blk->end_address = address + nbytes;
  blk->buf         = bufptr;
  blk->can_write   = can_write;
  blk->next        = first_blk;

  if (trace & TRACE_MEM_WRITES)
    fprintf (stderr, " Create a %s block at %" PRINT_64 "x to %" PRINT_64 "x\n",
	     can_write ? "R/W" : "R", address, address + nbytes - 1);
  active_blk = first_blk = blk;
}

static inline bfd_boolean
address_active (u_int64_t start_address, u_int64_t end_address)
{
  return active_blk->address <= start_address
    && active_blk->end_address > end_address;
}

static bfd_boolean
make_active (address)
{
  mem_blk * blk;

  for (blk = first_blk; blk != NULL; blk = blk->next)
    if (blk->address <= address && blk->end_address > address)
      {
	active_blk = blk;
	return TRUE;
      }

  return FALSE;
}

static inline void
sim_error (const char * message, u_int64_t addr)
{
  if (trace || disas)
    fprintf (stderr, "\n");
  fprintf (stderr, "sim error: ");
  fprintf (stderr, message, addr);
  fprintf (stderr, "\n");
}

#define FETCH_FUNC(RETURN_TYPE, ACCESS_TYPE, NAME)		\
  RETURN_TYPE							\
  aarch64_get_mem_##NAME (u_int64_t address)			\
  {									\
    if (!address_active (address, address + sizeof (ACCESS_TYPE)))	\
      {									\
	if (! make_active (address)) /* FIXME: Pass end_address as well ?  */ \
	  {								\
	    sim_error ("read from non-existant mem at %" PRINT_64 "x", address); \
	    aarch64_set_status (STATUS_ERROR, ERROR_EXCEPTION);		\
	    return 0; /* FIXME: Return HALT instruction ?  */		\
	  }								\
      }									\
									\
    /* FIXME: Do we need to the check alignment of address ?  */	\
    return (RETURN_TYPE) * ((ACCESS_TYPE *) (active_blk->buf + (address - active_blk->address))); \
  }


#define STORE_FUNC(TYPE, NAME) \
  StatusCode						 \
  aarch64_set_mem_##NAME (u_int64_t address, TYPE value) \
  {								\
    if (! address_active (address, address + sizeof (TYPE)))		\
      {									\
	if (! make_active (address)) /* FIXME: Pass end_address as well ?  */ \
	  {								\
	    sim_error ("write to non-existant mem at %" PRINT_64 "x", address); \
	    return aarch64_set_status (STATUS_ERROR, ERROR_EXCEPTION);	\
	  }								\
      }									\
									\
    if (! active_blk->can_write)					\
      {									\
	sim_error ("write to read-only mem at %" PRINT_64 "x", address); \
	return aarch64_set_status (STATUS_ERROR, ERROR_EXCEPTION);	\
      }									\
									\
    if (trace & TRACE_MEM_WRITES)					\
      fprintf (stderr, "write of %" PRINT_64 "x to %" PRINT_64 "x ", (u_int64_t) value, address);	\
									\
    /* FIXME: Check address alignment ?  */				\
    * ((TYPE *) (active_blk->buf + address - active_blk->address)) = value; \
    return STATUS_READY;						\
  }

FETCH_FUNC (u_int64_t,   u_int64_t,   u64)
FETCH_FUNC (int64_t,     int64_t,     s64)
FETCH_FUNC (u_int32_t,   u_int32_t,   u32)
FETCH_FUNC (int32_t,     int32_t,     s32)
FETCH_FUNC (u_int32_t,   u_int16_t,   u16)
FETCH_FUNC (int32_t,     int16_t,     s16)
FETCH_FUNC (u_int32_t,   u_int8_t,    u8)
FETCH_FUNC (int32_t,     int8_t,      s8)
FETCH_FUNC (float,       float,       float)
FETCH_FUNC (double,      double,      double)

void
aarch64_get_mem_long_double (u_int64_t address, FRegister * a)
{
  if (!address_active (address, address + sizeof (long double)))
    {
      if (! make_active (address)) /* FIXME: Pass end_address as well ?  */
	{								
	  sim_error ("read from non-existant mem at %" PRINT_64 "x", address); 
	  aarch64_set_status (STATUS_ERROR, ERROR_EXCEPTION);		
	  return;
	}								
    }									

  /* We have to be careful here because x86_64 requires 8 byte alignment
     for long doubles, but aarch64 only requires 4 byte alignment...  */
  a->v[0] = * ((u_int64_t *) (active_blk->buf + (address - active_blk->address)));
  a->v[1] = * ((u_int64_t *) (active_blk->buf + (address + 8 - active_blk->address)));
}

STORE_FUNC (u_int64_t,   u64)
STORE_FUNC (int64_t,     s64)
STORE_FUNC (u_int32_t,   u32)
STORE_FUNC (int32_t,     s32)
STORE_FUNC (u_int16_t,   u16)
STORE_FUNC (int16_t,     s16)
STORE_FUNC (u_int8_t,    u8)
STORE_FUNC (int8_t,      s8)
STORE_FUNC (float,       float)
STORE_FUNC (double,      double)

StatusCode
aarch64_set_mem_long_double (u_int64_t address, FRegister a)
{								
  if (! address_active (address, address + sizeof (long double)))		
    {									
      if (! make_active (address)) /* FIXME: Pass end_address as well ?  */ 
	{								
	  sim_error ("write to non-existant mem at %" PRINT_64 "x", address); 
	  return aarch64_set_status (STATUS_ERROR, ERROR_EXCEPTION);	
	}								
    }									

  if (! active_blk->can_write)					
    {
      sim_error ("write to read-only mem at %" PRINT_64 "x", address);
      return aarch64_set_status (STATUS_ERROR, ERROR_EXCEPTION);
    }

  if (trace & TRACE_MEM_WRITES)
    {
      fprintf (stderr, "write of %" PRINT_64 "x", a.v[0]);
      fprintf (stderr, " %" PRINT_64 "x", a.v[1]);
      fprintf (stderr, " to %" PRINT_64 "x ", address);
    }

  /* Prevent x86_64 alignment issues from mangling our writes.  */
  * ((u_int64_t *) (active_blk->buf + address - active_blk->address)) = a.v[0];
  * ((u_int64_t *) (active_blk->buf + (address + 8 - active_blk->address))) = a.v[1];

  return STATUS_READY;						
}

StatusCode
aarch64_get_mem_blk (u_int64_t address, char * buffer, unsigned length)
{
  if (!address_active (address, address + length))
    {
       /* FIXME: Pass end_address as well ?  */
      if (! make_active (address))
	{
	  sim_error ("read of non-existant mem block at %" PRINT_64 "x", address);
	  memset (buffer, 0, length);
	  return aarch64_set_status (STATUS_ERROR, ERROR_EXCEPTION);
	}
    }

  memcpy (buffer, active_blk->buf + (address - active_blk->address), length);

  return STATUS_READY;
}

const char *
aarch64_get_mem_ptr (u_int64_t address)
{
  if (!address_active (address, address + 1))
    {
      if (! make_active (address))
	{
	  sim_error ("request for non-existant mem addr of %" PRINT_64 "x", address);
	  aarch64_set_status (STATUS_ERROR, ERROR_EXCEPTION);
	  return NULL;
	}
    }

  return active_blk->buf + (address - active_blk->address);
}

void
aarch64_unload (void)
{
  mem_blk * blk;
  mem_blk * next;

  for (blk = first_blk; blk != NULL; blk = next)
    {
      next = blk->next;
      free (blk->buf);
      free (blk);
    }

  first_blk = active_blk = NULL;
  saved_prog = NULL;
}

bfd_boolean
aarch64_load (bfd * prog)
{
  Elf_Internal_Phdr * phdrs;
  unsigned long sizeof_phdrs;
  unsigned int num_headers;
  int i;

  /* Note we load by ELF program header not by BFD sections.
     This is because BFD sections get their information from
     the ELF section structure, which only includes a VMA value
     and not an LMA value.  */
  sizeof_phdrs = bfd_get_elf_phdr_upper_bound (prog);
  if (sizeof_phdrs == 0)
    {
      fprintf (stderr, "sim error: failed to get size of program headers in %s\n",
	       bfd_get_filename (prog));
      return FALSE;
    }

  phdrs = malloc (sizeof_phdrs);
  if (phdrs == NULL)
    {
      sim_error ("failed allocate 0x%lx bytes to hold program headers",
		 sizeof_phdrs);
      return FALSE;
    }

  num_headers = bfd_get_elf_phdrs (prog, phdrs);
  if (num_headers < 1)
    {
      fprintf (stderr, "sim error: failed to read in program headers from %s\n",
	       bfd_get_filename (prog));
      return FALSE;
    }
  
  for (i = 0; i < num_headers; i++)
    {
      Elf_Internal_Phdr * p = phdrs + i;
      char *buf;
      bfd_vma fsize;
      bfd_vma msize;
      bfd_vma base;
      file_ptr offset;

      fsize = p->p_filesz;
      if (fsize == 0)
	continue;

      /* Skip debug sections.  */
      if ((p->p_flags & (PF_X | PF_R | PF_W)) == 0)
	continue;

      base = p->p_paddr;
      msize = p->p_memsz;
      if (msize < fsize)
	{
	  fprintf (stderr, "%s: Segment %d: Corrupt sizes %" PRINT_64 "x %" PRINT_64 "x\n",
		   bfd_get_filename (prog), i, fsize, msize);
	  break;
	}

      buf = xmalloc (msize);
      
      offset = p->p_offset;
      if (prog->iovec->bseek (prog, offset, SEEK_SET) != 0)
	{
	  fprintf (stderr, "Failed to seek to offset %lx in %s for segment %d\n",
		   (long) offset, bfd_get_filename (prog), i);
	  break;
	}
	  
      if (prog->iovec->bread (prog, buf, fsize) != fsize)
	{
	  fprintf (stderr, "Failed to read %" PRINT_64 "x bytes from %s for segment %d\n",
		   fsize, bfd_get_filename (prog), i);
	  break;
	}

      mem_add_blk (base, buf, msize, p->p_flags & PF_W);
    }

  free (phdrs);

  if (i == num_headers)
    {
      saved_prog = prog;
      return TRUE;
    }

  aarch64_unload ();
  return FALSE;
}

/* We implement a combined stack and heap.  That way the sbrk()
   function in libgloss/aarch64/syscalls.c has a chance to detect
   an out-of-memory condition by noticing a stack/heap collision.

   The heap starts at the end of loaded memory and carries on up
   to an arbitary 2Gb limit.  */

u_int64_t
aarch64_get_heap_start (void)
{
  u_int64_t heap = aarch64_get_sym_value ("end");
  if (heap == 0)
    {
      fprintf (stderr, "Unable to find 'end' symbol - using stack addr instead %" PRINT_64 "x\n",
	       aarch64_get_sym_value ("_end"));
      heap = STACK_TOP - 0x10000000;
    }
  return heap;
}
    
u_int64_t
aarch64_get_stack_start (void)
{
  if (aarch64_get_heap_start () >= STACK_TOP)
    sim_error ("executable is too big: %" PRINT_64 "x", aarch64_get_heap_start ());
  return STACK_TOP;
}

u_int64_t
aarch64_get_start_pc (void)
{
  if (saved_prog)
    return bfd_get_start_address (saved_prog);
  return 0;
}
