/* Reader for Cisco IOS format core files, for GDB.
   Copyright 1999 Free Software Foundation, Inc.
   Derived in part from core-cisco.c, contributed by Cisco Systems Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include "defs.h"
#include "gdbcore.h"
#include "inferior.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "command.h"
#include "gdbcmd.h"
#include "symfile.h"
#include "objfiles.h"
#include "remote.h"

#if 0
#define MASK_ADDR(x)	((x) & 0x0fffffff)	/* mips, powerpc, x86, i960 */
#else
#define MASK_ADDR(x)	(x)			/* m68k, sparc */
#endif

#define CRASH_MAGIC 0xdead1234

/*
 * V4 for IOS under 10.3
 * V5 for IOS 10.3 and up.
 * V5 differs from V4 only by adding cpu context at end of crashinfo.
 */
#define CRASH_VERSION_OLD 1
#define CRASH_VERSION_RELOCATABLE 4

typedef enum
  {
    CRASH_REASON_NOTCRASHED,
    CRASH_REASON_EXCEPTION,
    CRASH_REASON_CORRUPT,
  }
crashreason;

struct crashinfo_external
{
  unsigned char magic[4];		/* Magic number */
  unsigned char version[4];		/* Version number */
  unsigned char reason[4];		/* Crash reason */
  unsigned char cpu_vector[4];		/* CPU vector for exceptions */
  unsigned char registers[4];		/* Pointer to saved registers */
  unsigned char rambase[4];		/* Base of RAM (not in V1 crash info) */
  unsigned char textbase[4];		/* Base of .text section (not in V3 crash info */
  unsigned char database[4];		/* Base of .data section (not in V3 crash info */
  unsigned char bssbase[4];		/* Base of .bss section (not in V3 crash info */
};

struct crashinfo_internal
{
  int magic;				/* Magic number */
  int version;				/* Version number */
  crashreason reason;			/* Crash reason */
  int cpu_vector;			/* CPU vector for exceptions */
  unsigned long registers;		/* Pointer to saved registers */
  unsigned long rambase;		/* Base of RAM (not in V1 crash info) */
  unsigned long textbase;		/* Base of .text section (not in V3 crash info */
  unsigned long database;		/* Base of .data section (not in V3 crash info */
  unsigned long bssbase;		/* Base of .bss section (not in V3 crash info */
};

/* Forward declarations of local functions. */

static int cisco_reference_addresses (bfd_vma *, bfd_vma *, bfd_vma *);
static int cisco_section_offsets (bfd_vma, bfd_vma, bfd_vma, bfd_signed_vma *,
				  bfd_signed_vma *, bfd_signed_vma *);
static int validate_cisco_classic_core (bfd *);
static int cisco_ios_core_sniffer (struct core_fns *, bfd *);

static int
cisco_reference_addresses (bfd_vma *text_base, bfd_vma *data_base,
			   bfd_vma *bss_base)
{
  struct minimal_symbol *start;
  asection *sect;
  bfd *abfd;
  int len;
  char *p;

  if (symfile_objfile == NULL)
    return -1;			/* no can do nothin' */

  start = lookup_minimal_symbol ("_start", NULL, NULL);
  if (start == NULL)
    return -1;			/* Can't find "_start" symbol */

  *data_base = *bss_base = 0;
  *text_base = SYMBOL_VALUE_ADDRESS (start);

  abfd = symfile_objfile->obfd;
  for (sect = abfd->sections;
       sect != 0;
       sect = sect->next)
    {
      p = (unsigned char *) bfd_get_section_name (abfd, sect);
      len = strlen (p);
      if (strcmp (p + len - 4, "data") == 0)	/* ends in "data" */
	if (*data_base == 0 ||
	    *data_base > bfd_get_section_vma (abfd, sect))
	  *data_base = bfd_get_section_vma (abfd, sect);
      if (strcmp (p + len - 3, "bss") == 0)	/* ends in "bss" */
	if (*bss_base == 0 ||
	    *bss_base > bfd_get_section_vma (abfd, sect))
	  *bss_base = bfd_get_section_vma (abfd, sect);
    }
  return 0;
}

static int
cisco_section_offsets (bfd_vma text_addr, bfd_vma data_addr, bfd_vma bss_addr,
		       bfd_signed_vma *text_offs, bfd_signed_vma *data_offs,
		       bfd_signed_vma *bss_offs)
{
  bfd_vma text_base, data_base, bss_base;

  if (cisco_reference_addresses (&text_base, &data_base, &bss_base) != 0)
    {
      warning ("Unable to get section addresses.");
      return -1;
    }

  *text_offs = text_addr - text_base;
  *data_offs = data_addr - data_base;
  *bss_offs = bss_addr - bss_base;
  return 0;
}

/*
 * Function: remote_cisco_objfile_relocate
 *
 * Relocate the symbol file for a remote target. 
 */

void
remote_cisco_objfile_relocate (bfd_signed_vma text_off, bfd_signed_vma data_off,
			       bfd_signed_vma bss_off)
{
  struct section_offsets *offs;

  if (text_off != 0 || data_off != 0 || bss_off != 0)
    {
      /* FIXME: This code assumes gdb-stabs.h is being used; it's
         broken for xcoff, dwarf, sdb-coff, etc.  But there is no
         simple canonical representation for this stuff.  */

      offs = (struct section_offsets *) 
	alloca (SIZEOF_N_SECTION_OFFSETS (symfile_objfile->num_sections));
      memcpy (offs, symfile_objfile->section_offsets, 
	      SIZEOF_N_SECTION_OFFSETS (symfile_objfile->num_sections));

      offs->offsets[SECT_OFF_TEXT (symfile_objfile)] = text_off;
      offs->offsets[SECT_OFF_DATA (symfile_objfile)] = data_off;
      offs->offsets[SECT_OFF_BSS (symfile_objfile)] = bss_off;

      /* First call the standard objfile_relocate.  */
      objfile_relocate (symfile_objfile, offs);

      /* Now we need to fix up the section entries already attached to
         the exec target.  These entries will control memory transfers
         from the exec file.  */

      exec_set_section_offsets (text_off, data_off, bss_off);
    }
}

/* Check a cisco IOS-Classic core file for validity */

static int
validate_cisco_classic_core (bfd *abfd)
{
  struct crashinfo_external xcrash;
  struct crashinfo_internal icrash;
  bfd_signed_vma text_off;
  bfd_signed_vma data_off;
  bfd_signed_vma bss_off;
  bfd_size_type size;
  sec_ptr crash_sec;

  /* Find the BFD section containing the crash information. */

  crash_sec = bfd_get_section_by_name (abfd, ".crash");
  if (crash_sec == NULL)
    return (0);

  /* Check that the size of the .crash section matches what we
     expect.  If not, there is probably a mismatch between BFD
     and us, with regards to the contents of the crash struct,
     so we play it safe and reject it. */

  size = bfd_section_size (abfd, crash_sec);
  if (size != sizeof (xcrash))
    return (0);

  if (!bfd_get_section_contents (abfd, crash_sec, (PTR) &xcrash,
				 (file_ptr) 0, size))
    return (0);

  /* Validate the magic number in the crash info */

  icrash.magic = bfd_get_32 (abfd, xcrash.magic);
  if (icrash.magic != CRASH_MAGIC)
    {
      return (0);
    }

  icrash.version = bfd_get_32 (abfd, xcrash.version);
  if (icrash.version == CRASH_VERSION_OLD)
    {
      /* Handle V1 core dumps which don't specify the dump base.
         Assume that the dump base is zero.  */
      icrash.rambase = 0;
    }
  else
    {
      icrash.rambase = bfd_get_32 (abfd, xcrash.rambase);
    }

  if (icrash.version < CRASH_VERSION_RELOCATABLE)
    {
      /* Handle core files that don't include section base addresses. */
      icrash.textbase = ~0;
      icrash.database = ~0;
      icrash.bssbase = ~0;
    }
  else
    {
      icrash.textbase = bfd_get_32 (abfd, xcrash.textbase);
      icrash.database = bfd_get_32 (abfd, xcrash.database);
      icrash.bssbase = bfd_get_32 (abfd, xcrash.bssbase);
    }

  icrash.reason = bfd_get_32 (abfd, xcrash.reason);
  icrash.cpu_vector = bfd_get_32 (abfd, xcrash.cpu_vector);
  icrash.registers = bfd_get_32 (abfd, xcrash.registers);

  icrash.rambase = MASK_ADDR (icrash.rambase);
  icrash.registers = MASK_ADDR (icrash.registers);
  
  /* Check for a relocatable core file and relocate it if necessary.  */

  if (icrash.version >= CRASH_VERSION_RELOCATABLE)
    {
      if (cisco_section_offsets ((bfd_vma) icrash.textbase,
				 (bfd_vma) icrash.database,
				 (bfd_vma) icrash.bssbase,
				 &text_off,
				 &data_off,
				 &bss_off) != 0)
	{
	  return (0);
	}
    }
  if (text_off != 0 || data_off != 0 || bss_off != 0)
    remote_cisco_objfile_relocate (text_off, data_off, bss_off);

  /* Print the reason for the crash.  We probably should not be doing
     any output like this in the core file sniffer until after gdb has
     called all of them and accepted one of them as the handler.  This
     is fairly safe at this point as we pretty much have committed to
     the fact that this handler will work.  Longterm it might be better
     to have a function in the handler that gets called once the handler
     is accepted by gdb, to do this type of post-sniffing work. */

  switch (icrash.reason)
    {
    case CRASH_REASON_NOTCRASHED:
      printf_filtered ("This crash file probably came from write core\n");
      break;

    case CRASH_REASON_CORRUPT:
      printf_filtered ("The crash context area was corrupt - proceed with caution\n");
      /* fall  through to CRASH_REASON_EXCEPTION */

    case CRASH_REASON_EXCEPTION:
      printf_filtered ("Crash occured due to CPU exception %d\n", icrash.cpu_vector);
      break;

    default:
      printf_filtered ("Unknown crash reason %d\n", icrash.reason);
      break;
    }

  return (1);
}

static int
cisco_ios_core_sniffer (struct core_fns *cf, bfd *abfd)
{
  int result;

  result = ((bfd_get_format (abfd) == bfd_core) &&
	    (bfd_get_flavour (abfd) == cf -> core_flavour) &&
	    (strncmp (bfd_get_target (abfd), "cisco-ios-core", 14) == 0) &&
	    (validate_cisco_classic_core (abfd)));
  return (result);
}

/* Register that we are able to handle Cisco IOS core files */

static struct core_fns cisco_ios_core_fns =
{
  bfd_target_unknown_flavour,		/* core_flavour */
  default_check_format,			/* check_format */
  cisco_ios_core_sniffer,		/* core_sniffer */
  fetch_cisco_core_registers,		/* core_read_registers */
  NULL					/* next */
};

void
_initialize_cisco_ios_core (void)
{
  add_core_fns (&cisco_ios_core_fns);
}
