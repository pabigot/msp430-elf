/* Reader for Cisco CIP format core files, for GDB.
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
#include "libbfd.h"
#include "inferior.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "command.h"
#include "gdbcmd.h"
#include "value.h"
#include "gdbthread.h"
#include "symfile.h"
#include "objfiles.h"

#define MAX_SEG_NAMELEN		64	   /* maximum lengths .. Pranav*/
#define MAX_SECT_NAMELEN	32
#define MAX_BIN_SUF_NAMELEN	64
#define MAX_PATH_LEN		256
#define MAX_SEG_FILELEN		MAX_PATH_LEN + MAX_SEG_NAMELEN + 64 + MAX_BIN_SUF_NAMELEN
#define MAX_COMMAND_LEN		1024 + MAX_SEG_FILELEN
#define MAX_TCBS		531
#define CIP_HWVER		0xB2
#define CIP1			4
#define CIP2			5
#define COREFILE_BASE		0x80000000
#define CIP1_MAX_SRAM		0x80080000
#define CIP1_BASE_DRAM		0x84000000

/* Local storage */

static unsigned int tcb_table_p;
static unsigned int tcb_regs_offset;
static unsigned int tcb_pid_offset;
static unsigned int current_tcb_offset;
static unsigned int kernel_base_offset;
static unsigned int kernel_stack_offset;
static unsigned int kstack_regs_offset;
static unsigned int sizeof_kstack;
static unsigned int cmcc_hw_version;

static char *section_names[] =
{
  ".text", ".data", ".bss", ".rodata",
  ".sdata", ".sbss", ".scommon", "COMMON"
};

#define NUM_SECTIONS (sizeof (section_names) / sizeof (section_names[0]))

/* Forward declarations of local functions. */

static int create_data_sections (bfd *);
static asection *make_bfd_asection (bfd *, CONST char *, flagword,
				    bfd_size_type, bfd_vma, file_ptr);
static int create_reg_section (bfd *, int, file_ptr);
static int add_cip_tasks (bfd *);
static int cmcc_init_offsets (bfd *);
static file_ptr addr_to_offset (CORE_ADDR);
static file_ptr find_registers (bfd *);
static int cisco_cip_core_sniffer (struct core_fns *, bfd *);
static int get_num_segments (void);
static void cmcc_get_string (char *, CORE_ADDR, unsigned int);
static void get_seg_info (int, char *, CORE_ADDR *);
static void printLoadMap (char *, int);
static char *cisco_core_file_failing_command (bfd *);
static int cisco_core_file_failing_signal (bfd *);

static asection *
make_bfd_asection (bfd *abfd, CONST char *name, flagword flags,
		   bfd_size_type _raw_size, bfd_vma vma, file_ptr filepos)
{
  asection *asect;
  char *newname;

  newname = bfd_alloc (abfd, strlen (name) + 1);
  if (!newname)
    {
      return (NULL);
    }
  strcpy (newname, name);
  asect = bfd_make_section (abfd, newname);
  if (asect == NULL
      || ! bfd_set_section_size (abfd, asect, _raw_size)
      || ! bfd_set_section_alignment (abfd, asect, 2)
      || ! bfd_set_section_flags (abfd, asect, flags))
    {
      return (NULL);
    }
  /* WARNING - accessing BFD private data - FIXME */
  asect -> vma = vma;
  asect -> filepos = filepos;
  return (asect);
}

static int
create_reg_section (bfd *abfd, int tid, file_ptr regoffset)
{
  int result = 0;
  asection *newsect;
  char sectname[100];

  if (tid > 0)
    {
      sprintf (sectname, ".reg/%d", tid);
    }
  else
    {
      strcpy (sectname, ".reg");
    }
  newsect = make_bfd_asection (abfd, sectname, SEC_HAS_CONTENTS,
			       (bfd_size_type) DEPRECATED_REGISTER_BYTES,
			       (bfd_vma) 0, regoffset);
  if (newsect != NULL)
    {
      result = 1;
    }
  return (result);
}

static int
create_data_sections (bfd *abfd)
{
  int result = 0;
  asection *newsect1;
  asection *newsect2;
  struct stat st_core;
  int status;

  /* First we need to stat the core file so we can get it's length. */
  status = fstat (fileno ((FILE *) (abfd -> iostream)), &st_core);
  if (status < 0)
    {
      return (0);
    }
  
  switch (cmcc_hw_version)
    {
    case CIP1:
      /* The corefile consists of two segments.  The first is 0x80000
	 bytes long, with vma address of 0x80000000 to 0x8007FFF.  The
	 second segment is the memory starting at vma 0x84000000. */
      newsect1 = make_bfd_asection (abfd, ".data0", SEC_HAS_CONTENTS | SEC_ALLOC,
				    0x80000, 0x80000000, 0);
      
      newsect2 = make_bfd_asection (abfd, ".data1", SEC_HAS_CONTENTS | SEC_ALLOC,
				    st_core.st_size - 0x80000, 0x84000000, 0x80000);
      if (newsect1 != NULL && newsect2 != NULL)
	{
	  result = 1;
	}
      break;
    case CIP2:
      newsect1 = make_bfd_asection (abfd, ".data", SEC_HAS_CONTENTS | SEC_ALLOC,
				    st_core.st_size - 1, (bfd_vma) COREFILE_BASE, 0);
      if (newsect1 != NULL)
	{
	  result = 1;
	}
      break;
    default:
      break;
    }
  return (result);
}

static int
add_cip_tasks (bfd *abfd)
{
  int i;
  int atid;
  file_ptr filepos;
  CORE_ADDR current_tcb_addr;
  CORE_ADDR tcb_table_addr;
  CORE_ADDR atcb_addr;
  file_ptr tcbtbloff;
  char tmp[4];

  bfd_seek (abfd, current_tcb_offset, SEEK_SET);
  bfd_read (&tmp, sizeof (tmp), 1, abfd);
  current_tcb_addr = bfd_get_32 (abfd, (bfd_byte *) tmp);

  bfd_seek (abfd, tcb_table_p, SEEK_SET);
  bfd_read (&tmp, sizeof (tmp), 1, abfd);
  tcb_table_addr = bfd_get_32 (abfd, (bfd_byte *) tmp);
  tcbtbloff = addr_to_offset (tcb_table_addr);
  
  for (i = 0; i < MAX_TCBS; i++, tcbtbloff += 4)
    {
      bfd_seek (abfd, tcbtbloff, SEEK_SET);
      bfd_read (tmp, sizeof (tmp), 1, abfd);
      atcb_addr = bfd_get_32 (abfd, (bfd_byte *) tmp);
      if (atcb_addr <= MAX_TCBS)
	{
	  continue;
	}

      filepos = addr_to_offset (atcb_addr) + tcb_pid_offset;
      bfd_seek (abfd, filepos, SEEK_SET);
      bfd_read (tmp, sizeof (tmp), 1, abfd);
      atid = bfd_get_32 (abfd, (bfd_byte *) tmp);

      if (i != atid)
	{
	  /* Get rid of bad tasks */
	  continue;
	}
      if (atcb_addr == current_tcb_addr)
	{
	  inferior_ptid = pid_to_ptid (atid);
	  filepos = find_registers (abfd);
	  create_reg_section (abfd, 0, filepos);
	}
      else
	{
	  filepos = addr_to_offset (atcb_addr) + tcb_regs_offset;
	}
      create_reg_section (abfd, atid, filepos);
      add_thread (pid_to_ptid (atid));
    }
  return (1);
}

static int
cmcc_init_offsets (bfd *abfd)
{
  char temp;

  /* Yup, that's right, we have to have symbols or we can't read
     a Cisco cip corefile, or even be sure that is what we are
     dealing with. */
  if (!have_full_symbols () && !have_partial_symbols ())
    {
      return (0);
    }

  /* Find all the offsets */

  tcb_table_p = parse_and_eval_address ("&((lowcore *)0x0)->tcb_table_p");
  if (tcb_table_p == 0)
    {
      return (0);
    }

  tcb_regs_offset = parse_and_eval_address ("&((tcb*)0x0)->save_registers");
  if (tcb_regs_offset == 0)
    {
      return (0);
    }

  tcb_pid_offset = parse_and_eval_address ("&((tcb*)0x0)->process_id");
  if (tcb_pid_offset == 0)
    {
      return (0);
    }

  current_tcb_offset = parse_and_eval_address ("&((lowcore *)0x0)->current_tcb");  
  if (current_tcb_offset == 0)
    {
      return (0);
    }

  kernel_base_offset = parse_and_eval_address ("&((lowcore *)0x0)->kernel_base");
  if (kernel_base_offset == 0)
    {
      return (0);
    }

  kernel_stack_offset = parse_and_eval_address ("&((lowcore *)0x0)->kernel_stack");
  if (kernel_stack_offset == 0)
    {
      return (0);
    }

  kstack_regs_offset = parse_and_eval_address ("&((kstack *)0x0)->save_registers");
  if (kstack_regs_offset == 0)
    {
      return (0);
    }

  sizeof_kstack = parse_and_eval_address ("sizeof(kstack)"); 
  if (sizeof_kstack == 0)
    {
      return (0);
    }

  bfd_seek (abfd, CIP_HWVER, SEEK_SET);
  bfd_read (&temp, sizeof (temp), 1, abfd);
  cmcc_hw_version = temp;

  if (cmcc_hw_version != CIP1 && cmcc_hw_version != CIP2)
    {
      return (0);
    }
  return (1);
}
    
/* Given address ADDR in the defunct process, figure out the file
   offset in the core file where the data resides that was at this
   address at runtime.  The caller will most likely then do a bfd_seek
   to that offset and then a bfd_read to fetch that data.

   FIXME: Should we do more checking to make sure that the address is
   not out of bounds?  */

static file_ptr
addr_to_offset (CORE_ADDR addr)
{
  file_ptr offset;

  switch (cmcc_hw_version)
    {
    case (CIP1):
      if (addr <= CIP1_MAX_SRAM)
	{
	  offset = addr - COREFILE_BASE;
	}
      else if (addr >= CIP1_BASE_DRAM)
	{
	  offset = addr - CIP1_BASE_DRAM - COREFILE_BASE + CIP1_MAX_SRAM;
	}
      else
	{
	  /* Address in hole */
	  offset = 0;
	}
      break;

    case (CIP2):
      offset = addr - COREFILE_BASE;
      break;

    default:
      error ("Invalid CIP hardware version");
    }
  return (offset);
}

/* Determine where the relevant stored register values are.
   FIXME: Should do error checking on bfd calls. */

static file_ptr
find_registers (bfd *abfd)
{
  CORE_ADDR kernel_stack_addr;
  CORE_ADDR kernel_base_addr;
  CORE_ADDR current_tcb_addr;
  file_ptr regoffset;
  char temp[4];

  bfd_seek (abfd, kernel_stack_offset, SEEK_SET);
  bfd_read (temp, sizeof (temp), 1, abfd);
  kernel_stack_addr = bfd_get_32 (abfd, (bfd_byte *) temp);

  bfd_seek (abfd, kernel_base_offset, SEEK_SET);
  bfd_read (temp, sizeof (temp), 1, abfd);
  kernel_base_addr = bfd_get_32 (abfd, (bfd_byte *) temp);

  if (kernel_stack_addr == kernel_base_addr)
    {
      /* CIP was running a user task, registers got by following
	 current_tcb pointer. */
      bfd_seek (abfd, current_tcb_offset, SEEK_SET);
      bfd_read (temp, sizeof (temp), 1, abfd);
      current_tcb_addr = bfd_get_32 (abfd, (bfd_byte *) temp);
      regoffset = addr_to_offset (current_tcb_addr) + tcb_regs_offset;
    }
  else
    {
      /* CIP was running on interrupt stack, registers got from kstack
         structure. */
      regoffset = addr_to_offset (kernel_stack_addr - sizeof_kstack) + kstack_regs_offset;
    }
  return (regoffset);
}

static char *
cisco_core_file_failing_command (bfd *abfd)
{
  return (NULL);
}

static int
cisco_core_file_failing_signal (bfd *abfd)
{
  return (0);
}

static int
cisco_core_file_matches_executable_p (bfd *core_bfd, bfd *exec_bfd)
{
  /* FIXME, We have no way of telling at this point */
  return (1);
}

/* Perform the equivalent of bfd_check_format for a Cisco CIP
   core file. */

int
cisco_cip_check_format (bfd *abfd)
{
  int result = 0;

  if (cmcc_init_offsets (abfd) &&
      add_cip_tasks (abfd) &&
      create_data_sections (abfd))
    {
      /* WARNING!! Mucking with private BFD internal structures */
      bfd_target *newtarg;
      newtarg = (bfd_target *) bfd_alloc (abfd, sizeof (bfd_target));
      memcpy ((void *) newtarg, (const void *) abfd->xvec, sizeof (bfd_target));
      newtarg -> name = "cisco-cip-core";
      newtarg -> flavour = bfd_target_unknown_flavour;
      newtarg -> _core_file_failing_command = cisco_core_file_failing_command;
      newtarg -> _core_file_failing_signal = cisco_core_file_failing_signal;
      newtarg -> _core_file_matches_executable_p = cisco_core_file_matches_executable_p;
      abfd -> xvec = newtarg;
      abfd -> format = bfd_core;
      result = 1;
    }
  return (result);
}


static int
cisco_cip_core_sniffer (struct core_fns *cf, bfd *abfd)
{
  int result;

  result = ((bfd_get_format (abfd) == bfd_core) &&
	    (bfd_get_flavour (abfd) == cf -> core_flavour) &&
	    (strncmp (bfd_get_target (abfd), "cisco-cip-core", 14) == 0));
  return (result);
}

/* Returns the number of loaded segments (not including the kernel),
   or returns -1 for error.  */

static int
get_num_segments (void)
{
  int count = 0;
  int i = 0;
    
  /* Loop until name_ptr == NULL, counting segments where version_ptr != NULL. */
  for (i = 0; ; i++)
    {
      CORE_ADDR name_ptr;
      CORE_ADDR version_ptr;
      char expr[256];

      sprintf (expr, "load_table[%d]->name", i);
      name_ptr = parse_and_eval_address (expr);

      sprintf (expr, "load_table[%d]->version_ptr", i);
      version_ptr = parse_and_eval_address (expr);

      if (name_ptr == 0)
	{
	  /* Counted all the segments */
	  break;
	}

      if (version_ptr == 0)
	{
	  /* This segment is not loaded */
	  continue;
	}

      /* Count this segment, and go on .. */
      ++count;
    }
  return (count);
}

/* Pranav: 
 * cmcc_get_string - extract a string from cip corefiles or live images
 * Inputs: dest      - destination of the string
 *         cmcc_addr - source addr - could be corefile addr or live image
 *         max       - max size to extract
 * Output: void
 */

static void
cmcc_get_string (char *dest, CORE_ADDR cmcc_addr, unsigned int imax)
{
  char buf[4];
  int i;

  while (imax != 0)
    {
      read_memory (cmcc_addr, buf, sizeof (buf));
      for (i = 0; i < sizeof (buf); i++)
	{
	  dest[i] = buf[i];
	  if (buf[i] == '\0')
	    {
	      return;
	    }
	}
      imax -= sizeof (buf);
      dest += sizeof (buf);
      cmcc_addr += sizeof (buf);
    }
}

/* Returns in segname and offsets the name and section offsets for a
   segment identified by segment_index. The number of segments loaded
   may be determined with function get_num_segments, the segment are
   then identified with indexes ranging from 0 to the number of
   segments minus one.  Function return 0 for success, otherwise an
   error code.  */

static void
get_seg_info (int segment_index, char *segname, CORE_ADDR *offsets)
{
  int count = 0;
  int i;

  for (i = 0; ; i++)
    {
      CORE_ADDR name_ptr;
      CORE_ADDR version_ptr;
      CORE_ADDR section_ptr;
      char expr[256];
        
      sprintf (expr, "load_table[%d]->name", i); /* load_table not init? */
      name_ptr = parse_and_eval_address (expr);

      sprintf (expr, "load_table[%d]->version_ptr", i);
      version_ptr = parse_and_eval_address (expr);

      sprintf (expr, "load_table[%d]->section_ptr", i);
      section_ptr = parse_and_eval_address (expr);

      if (name_ptr == 0)
	{
	  /* Counted all the segments */
	  break;
	}

      if (version_ptr == 0)
	{
	  /* This segment is not loaded */
	  continue;
	}

      if (count++ != segment_index)
	{
	  continue;
	}
 
      cmcc_get_string (segname, name_ptr, MAX_SEG_NAMELEN);
 
      /* Now get the section info for the segment: */
 
      if (section_ptr != 0)
	{
	  int sizeofsection =
	    parse_and_eval_address ("sizeof(loader_section_info)");
	  for (; 1 ; section_ptr += sizeofsection)
	    {
	      int sect;
	      char section_name[MAX_SEG_NAMELEN];
	      CORE_ADDR sect_name_ptr;

	      sprintf (expr, "((loader_section_info*)0x%x)->name",
		       (int) section_ptr);
	      sect_name_ptr = parse_and_eval_address (expr);
	      cmcc_get_string (section_name, sect_name_ptr, MAX_SEG_NAMELEN);

	      if (section_name[0] == 0)
		{
		  break;
		}
                
	      for (sect = 0; sect < NUM_SECTIONS; sect++)
		{
		  if (strcmp (section_name, section_names[sect]) == 0)
		    {
		      sprintf (expr,
			       "((loader_section_info *)0x%x)->start_addr", 
			       (int) section_ptr);
		      offsets[sect] = parse_and_eval_address (expr);
		      break;
		    }
		}
	    }
      }
    break;
  }
}

/*
 * cmccLoadSegments - Load segments of a cmcc segmented image. Pranav Tiwari
 * Inputs: bin_suffix - BIN_SUFFIX of the segment files
 *         from_tty   - don't know; add for  consistency with other commands
 * Output: void
 */

void
cmccLoadSegments (char *bin_suffix, int from_tty)
{
  int i, j, k;
  CORE_ADDR offsets[NUM_SECTIONS];
  char segname[MAX_SEG_NAMELEN];
  char segfile[MAX_SEG_FILELEN];
  char cmd[MAX_COMMAND_LEN];
  char opt_scratch[MAX_SECT_NAMELEN + 16];

  struct stat buf;

  j = get_num_segments ();
  printf ("\nNumber of Segments: %d\n", j);
  for (i = 0; i < j; i++)
    {
      memset (offsets, 0, sizeof (offsets));
      get_seg_info (i, segname, offsets);

      if ((strcmp (segname, "seg_eca") == 0) ||
	  (strcmp (segname, "seg_pca") == 0))
        {
	  printf ("skipping %s ..\n", segname);
	  continue;
        }

      /* find the file containing this segment */

      strncpy (segfile, segname, MAX_SEG_NAMELEN);
      strcat (segfile, ".unstrip");
      if (bin_suffix != 0)
	{
	  if (strlen (bin_suffix) > MAX_BIN_SUF_NAMELEN) {
	    printf
	      ("Can't handle bin suffix ( = %s) of length greater than %d\n", 
	       bin_suffix, MAX_BIN_SUF_NAMELEN);
	    return;
	  }
	  strcat (segfile, bin_suffix);
	}

      /* Make sure the file exists */
      if (stat(segfile, &buf) == 0)
	{
	  printf ("loading segment '%s' from file '%s' ..\n", segname,
		  segfile);

	  sprintf (cmd, "add-symbol-file %s", segfile);
	  for (k = 0; k < NUM_SECTIONS; k++)
	    {
	      if (offsets[k] != 0)
		{
		  sprintf (opt_scratch, " -T%s 0x%x", section_names[k], 
			   (int) offsets[k]);
		  strcat (cmd, opt_scratch);
		}
	    }
	  execute_command (cmd, 0);    
	}
      else
	{
	  printf("file %s not found\n", segfile);
	}
    }
}

/* Prints out section info for all segments in core file or returns -1
   for error.  */

static void
printLoadMap (char *args, int from_tty)
{
  int i, j, k;
  CORE_ADDR offsets[NUM_SECTIONS];
  char segname[MAX_SEG_NAMELEN];

  j = get_num_segments ();
  printf ("\nNumber of Segments: %d\n", j);
  for (i = 0; i < j; i++)
    {
      memset (offsets, 0, sizeof (offsets));
      get_seg_info (i, segname, offsets);
      printf ("segment: %s\n", segname);
      for (k = 0; k < NUM_SECTIONS; k++)
	{
	  printf ("%s  %x ", section_names[k], (int) offsets[k]);
	}
      printf ("\n");
    }
}

/* Register that we are able to handle Cisco CIP core files */

static struct core_fns cisco_cip_core_fns =
{
  bfd_target_unknown_flavour,		/* core_flavour */
  cisco_cip_check_format,		/* check_format */
  cisco_cip_core_sniffer,		/* core_sniffer */
  fetch_cisco_core_registers,		/* core_read_registers */
  NULL					/* next */
};

void
_initialize_cisco_cip_core (void)
{
  struct cmd_list_element *c;

  /* FIXME:  These commands are named incorrectly.  They prevent using
     abbreviations for the "load" or "print" commands.  They should
     probably be renamed something like "cmcc load-symtab" and "cmcc
     print-map". */
  add_core_fns (&cisco_cip_core_fns);
  c = add_cmd ("print-load-map", class_files, printLoadMap,
	       "Print load map of core dump file", &cmdlist);
  add_cmd ("load-cmcc-symtab", class_files, cmccLoadSegments,
	   "Load CMCC symbol tables", &cmdlist);
  c = add_alias_cmd ("lcs", "load-cmcc-symtab", class_files, 1, &cmdlist);
}
