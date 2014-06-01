/* Reader for Cisco IOS-NG (ENA) format core files, for GDB.
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
#include "elf-bfd.h"
#include "inferior.h"
#include "gdb_string.h"

typedef int ios_ena_pthread_t;
typedef int ios_ena_pid_t;
typedef int ios_ena_uid_t;
typedef int ios_ena_gid_t;
typedef unsigned int ios_ena_clock_t;
typedef struct { unsigned char id[20]; } ios_ena_nid_t;

/* Cisco core files have some struct members that differ in size
   depending upon the size of pointers in the target environment.
   FIXME: This needs cleaning up. */

#if 0
#if TARGET_PTR_BIT <= 16
typedef unsigned short cisco_uintptr;
#define CISCO_UINTPTR_EXTERNAL 2
#elif TARGET_PTR_BIT <= 32
typedef unsigned int cisco_uintptr;
#define CISCO_UINTPTR_EXTERNAL 4
#elif TARGET_PTR_BIT <= 64
typedef unsigned long long cisco_uintptr;
#define CISCO_UINTPTR_EXTERNAL 8
#else
#error Unable to declare integral pointer type
#endif
#endif

/* following types are derived from IOS-ENA os/include/signal.h */

/* signal set type */

typedef struct
  {
    long bits[2];
  }
internal_ios_ena_sigset_t;

typedef struct
  {
    unsigned char bits0[4];
    unsigned char bits1[4];
  }
external_ios_ena_sigset_t;

union internal_ios_ena_sigval
  {
    int sival_int;
    void *sival_ptr;
  };

union external_ios_ena_sigval
  {
    unsigned char sival_int[4];
    unsigned char sival_ptr[4];
  };

/* following types are derived from IOS-ENA os/include/sys/siginfo.h */

typedef struct
  {
    int si_signo;
    int si_code;		/* if SI_NOINFO, only si_signo is valid */
    int si_errno;
    union
      {
	int __pad[7];
	struct
	  {
	    ios_ena_pid_t __pid;
	    union
	      {
		struct
		  {
		    ios_ena_uid_t __uid;
		    union internal_ios_ena_sigval __value;
		  }
		__kill;		/* si_code <= 0 SI_FROMUSER */
		struct
		  {
		    ios_ena_clock_t __utime;
		    int __status; /* CLD_EXITED status, else signo */
		    ios_ena_clock_t __stime;
		  }
		__chld;		/* si_signo=SIGCHLD si_code=CLD_* */
	      }
	    __pdata;
	  }
	__proc;
	struct
	  {
	    void *__addr;
	    int __fltno;
	    void *__fltip;
	  }
	__fault;		/* si_signo=SIGSEGV,ILL,FPE,TRAP,BUS */
      }
    __data;
  }
internal_ios_ena_siginfo_t;

typedef struct
  {
    unsigned char si_signo[4];
    unsigned char si_code[4];		/* if SI_NOINFO, only si_signo is valid */
    unsigned char si_errno[4];
    union
      {
	unsigned char __pad[4 * 7];
	struct
	  {
	    unsigned char __pid[4];
	    union
	      {
		struct
		  {
		    unsigned char __uid[4];
		    union external_ios_ena_sigval __value;
		  }
		__kill;		/* si_code <= 0 SI_FROMUSER */
		struct
		  {
		    unsigned char __utime[4];
		    unsigned char __status[4]; /* CLD_EXITED status, else signo */
		    unsigned char __stime[4];
		  }
		__chld;		/* si_signo=SIGCHLD si_code=CLD_* */
	      }
	    __pdata;
	  }
	__proc;
	struct
	  {
	    unsigned char __addr[4];
	    unsigned char __fltno[4];
	    unsigned char __fltip[4];
	  }
	__fault;		/* si_signo=SIGSEGV,ILL,FPE,TRAP,BUS */
      }
    __data;
  }
external_ios_ena_siginfo_t;

/* following types are derived from IOS-ENA os/include/sys/debug.h */

typedef struct
  {
    ios_ena_pid_t pid;
    ios_ena_pid_t parent;
    unsigned int flags;
    unsigned int umask;
    ios_ena_pid_t child;
    ios_ena_pid_t sibling;
    ios_ena_pid_t pgrp;
    ios_ena_pid_t sid;
    unsigned long long base_address;
    unsigned long long initial_stack;
    ios_ena_uid_t uid;
    ios_ena_gid_t gid;
    ios_ena_uid_t euid;
    ios_ena_gid_t egid;
    ios_ena_uid_t suid;
    ios_ena_gid_t sgid;
    internal_ios_ena_sigset_t sig_ignore;
    internal_ios_ena_sigset_t sig_queue;
    internal_ios_ena_sigset_t sig_pending;
    unsigned int num_chancons;
    unsigned int num_fdcons;
    unsigned int num_threads;
    unsigned int num_timers;
    unsigned long long reserved[20];
  }
internal_procfs_info;

typedef struct
  {
    unsigned char pid[4];
    unsigned char parent[4];
    unsigned char flags[4];
    unsigned char umask[4];
    unsigned char child[4];
    unsigned char sibling[4];
    unsigned char pgrp[4];
    unsigned char sid[4];
    unsigned char base_address[8];
    unsigned char initial_stack[8];
    unsigned char uid[4];
    unsigned char gid[4];
    unsigned char euid[4];
    unsigned char egid[4];
    unsigned char suid[4];
    unsigned char sgid[4];
    external_ios_ena_sigset_t sig_ignore;
    external_ios_ena_sigset_t sig_queue;
    external_ios_ena_sigset_t sig_pending;
    unsigned char num_chancons[4];
    unsigned char num_fdcons[4];
    unsigned char num_threads[4];
    unsigned char num_timers[4];
    unsigned char reserved[8 * 20];
  }
external_procfs_info;

#define INTERNAL_PROCFS_STATUS(N,cisco_uintptr)				\
typedef struct								\
  {									\
    ios_ena_pid_t pid;							\
    ios_ena_pthread_t tid;						\
    unsigned int flags;							\
    unsigned short why;							\
    unsigned short what;						\
    unsigned long long ip;						\
    unsigned long long sp;						\
    unsigned long long stkbase;						\
    unsigned long long tls;						\
    unsigned int stksize;						\
    unsigned int tid_flags;						\
    unsigned char priority;						\
    unsigned char real_priority;					\
    unsigned char policy;						\
    unsigned char state;						\
    short syscall;							\
    unsigned short last_cpu;						\
    unsigned int timeout;						\
    int last_chid;							\
    internal_ios_ena_sigset_t sig_blocked;				\
    internal_ios_ena_sigset_t sig_pending;				\
    internal_ios_ena_siginfo_t info;					\
    union								\
      {									\
	struct								\
	  {								\
	    ios_ena_pthread_t tid;					\
	  }								\
	join;								\
	struct								\
	  {								\
	    int id;							\
	    cisco_uintptr sync;						\
	  }								\
	sync;								\
	struct								\
	  {								\
	    ios_ena_nid_t nid;						\
	    ios_ena_pid_t pid;						\
	    int coid;							\
	    int chid;							\
	    int scoid;							\
	  }								\
	connect;							\
	struct								\
	  {								\
	    int chid;							\
	  }								\
	channel;							\
	struct								\
	  {								\
	    ios_ena_pid_t pid;						\
	    cisco_uintptr vaddr;					\
	    unsigned int flags;						\
	  }								\
	waitpage;							\
	struct								\
	  {								\
	    unsigned int size;						\
	  }								\
	stack;								\
	unsigned long long filler[4];					\
      }									\
    blocked;								\
    unsigned int pad;							\
    unsigned long long reserved2[8];					\
  }									\
internal_procfs_status_##N

INTERNAL_PROCFS_STATUS(16, unsigned short);
INTERNAL_PROCFS_STATUS(32, unsigned int);
INTERNAL_PROCFS_STATUS(64, unsigned long long);

#define EXTERNAL_PROCFS_STATUS(N,cisco_uintptr)				\
typedef struct								\
  {									\
    unsigned char pid[4];						\
    unsigned char tid[4];						\
    unsigned char flags[4];						\
    unsigned char why[2];						\
    unsigned char what[2];						\
    unsigned char ip[8];						\
    unsigned char sp[8];						\
    unsigned char stkbase[8];						\
    unsigned char tls[8];						\
    unsigned char stksize[4];						\
    unsigned char tid_flags[4];						\
    unsigned char priority;						\
    unsigned char real_priority;					\
    unsigned char policy;						\
    unsigned char state;						\
    unsigned char syscall[2];						\
    unsigned char last_cpu[2];						\
    unsigned char timeout[4];						\
    unsigned char last_chid[4];						\
    external_ios_ena_sigset_t sig_blocked;				\
    external_ios_ena_sigset_t sig_pending;				\
    external_ios_ena_siginfo_t info;					\
    union								\
      {									\
	struct								\
	  {								\
	    unsigned char tid[4];					\
	  }								\
	join;								\
	struct								\
	  {								\
	    unsigned char id[4];					\
	    unsigned char sync[N / 8 /*CISCO_UINTPTR_EXTERNAL*/];	\
	  }								\
	sync;								\
	struct								\
	  {								\
	    unsigned char nid[20];					\
	    unsigned char pid[4];					\
	    unsigned char coid[4];					\
	    unsigned char chid[4];					\
	    unsigned char scoid[4];					\
	  }								\
	connect;							\
	struct								\
	  {								\
	    unsigned char chid[4];					\
	  }								\
	channel;							\
	struct								\
	  {								\
	    unsigned char pid[4];					\
	    unsigned char vaddr[N / 8 /*CISCO_UINTPTR_EXTERNAL*/];	\
	    unsigned char flags[4];					\
	  }								\
	waitpage;							\
	struct								\
	  {								\
	    unsigned char size[4];					\
	  }								\
	stack;								\
	unsigned char filler[4 * 8];					\
      }									\
    blocked;								\
    unsigned char pad[4];						\
    unsigned char reserved2[8 * 8];					\
  }									\
external_procfs_status_##N

EXTERNAL_PROCFS_STATUS(16, unsigned short);
EXTERNAL_PROCFS_STATUS(32, unsigned int);
EXTERNAL_PROCFS_STATUS(64, unsigned long long);

typedef struct
  {
    unsigned long long padding[1024];
  }
internal_procfs_greg;

typedef struct
  {
    unsigned char padding[8 * 1024];
  }
external_procfs_greg;

/* Forward declarations of static functions */

static asection *make_bfd_asection (bfd *, CONST char *, flagword,
				    bfd_size_type, bfd_vma, file_ptr);
static void elf_swap_psinfo_in (bfd *, external_procfs_info *,
				internal_procfs_info *);
static void elf_swap_prstatus_in_16 (bfd *, external_procfs_status_16 *, internal_procfs_status_16 *);
static void elf_swap_prstatus_in_32 (bfd *, external_procfs_status_32 *, internal_procfs_status_32 *);
static void elf_swap_prstatus_in_64 (bfd *, external_procfs_status_64 *, internal_procfs_status_64 *);
static void process_corefile_notes (bfd *, asection *, PTR);
static int cisco_ena_core_sniffer (struct core_fns *, bfd *);

/* Possible NOTE types from the ELF NOTES program segment. */

#define NT_PROCFS_STATUS_ARRAY	1	/* Array of procfs_status, one per thread */
#define NT_PROCFS_FPREG_ARRAY	2	/* Array of saved fpregs, one per thread */
#define NT_PROCFS_INFO		3	/* procfs_info struct for process */
#define NT_PROCFS_GREG_ARRAY	4	/* Array of saved gpregs, one per thread */
#define NT_ROUTER_MODEL		5	/* String describing specific model of router */
#define NT_VERSION_STRING	6	/* Version string for the crashed process */


static asection *
make_bfd_asection (bfd *abfd, CONST char *name, flagword flags,
		   bfd_size_type _raw_size, bfd_vma vma, file_ptr filepos)
{
  asection *asect;
  char *newname;

  newname = bfd_alloc (abfd, strlen (name) + 1);
  if (!newname)
    {
      return NULL;
    }
  strcpy (newname, name);
  asect = bfd_make_section (abfd, newname);
  if (asect == NULL
      || ! bfd_set_section_size (abfd, asect, _raw_size)
      || ! bfd_set_section_alignment (abfd, asect, 2)
      || ! bfd_set_section_vma (abfd, asect, vma)
      || ! bfd_set_section_flags (abfd, asect, flags))
    {
      return (NULL);
    }
  asect->filepos = filepos;  /* FIXME! - accessing BFD private data */
  return (asect);
}

/* FIXME.  This code should be changed to use the same technique
   as BFD, where there are internal and external mappings of the
   struct, and the external mapping uses arrays of char for each
   member. */

static void
elf_swap_psinfo_in (bfd *abfd, external_procfs_info * src,
		    internal_procfs_info * dst)
{
  dst->pid = bfd_h_get_32 (abfd, (bfd_byte *) &src->pid);
  dst->parent = bfd_h_get_32 (abfd, (bfd_byte *) &src->parent);
  dst->flags = bfd_h_get_32 (abfd, (bfd_byte *) &src->flags);
  dst->umask = bfd_h_get_32 (abfd, (bfd_byte *) &src->umask);
  dst->child = bfd_h_get_32 (abfd, (bfd_byte *) &src->child);
  dst->sibling = bfd_h_get_32 (abfd, (bfd_byte *) &src->sibling);
  dst->pgrp = bfd_h_get_32 (abfd, (bfd_byte *) &src->pgrp);
  dst->sid = bfd_h_get_32 (abfd, (bfd_byte *) &src->sid);
  dst->base_address = bfd_h_get_64 (abfd, (bfd_byte *) &src->base_address);
  dst->initial_stack = bfd_h_get_64 (abfd, (bfd_byte *) &src->initial_stack);
  dst->uid = bfd_h_get_32 (abfd, (bfd_byte *) &src->uid);
  dst->gid = bfd_h_get_32 (abfd, (bfd_byte *) &src->gid);
  dst->euid = bfd_h_get_32 (abfd, (bfd_byte *) &src->euid);
  dst->egid = bfd_h_get_32 (abfd, (bfd_byte *) &src->egid);
  dst->suid = bfd_h_get_32 (abfd, (bfd_byte *) &src->suid);
  dst->sgid = bfd_h_get_32 (abfd, (bfd_byte *) &src->sgid);
  dst->sig_ignore.bits[0] =
    bfd_h_get_32 (abfd, (bfd_byte *) &src->sig_ignore.bits0[0]);
  dst->sig_ignore.bits[1] =
    bfd_h_get_32 (abfd, (bfd_byte *) &src->sig_ignore.bits1[0]);
  dst->sig_queue.bits[0] =
    bfd_h_get_32 (abfd, (bfd_byte *) &src->sig_queue.bits0[0]);
  dst->sig_queue.bits[1] =
    bfd_h_get_32 (abfd, (bfd_byte *) &src->sig_queue.bits1[0]);
  dst->sig_pending.bits[0] =
    bfd_h_get_32 (abfd, (bfd_byte *) &src->sig_pending.bits0[0]);
  dst->sig_pending.bits[1] =
    bfd_h_get_32 (abfd, (bfd_byte *) &src->sig_pending.bits1[0]);
  dst->num_chancons = bfd_h_get_32 (abfd, (bfd_byte *) &src->num_chancons);
  dst->num_fdcons = bfd_h_get_32 (abfd, (bfd_byte *) &src->num_fdcons);
  dst->num_threads = bfd_h_get_32 (abfd, (bfd_byte *) &src->num_threads);
  dst->num_timers = bfd_h_get_32 (abfd, (bfd_byte *) &src->num_timers);
}

#define ELF_SWAP_PRSTATUS_IN(N)						\
static void								\
elf_swap_prstatus_in_##N (abfd, src, dst)					\
     bfd *abfd;								\
     external_procfs_status_##N *src;					\
     internal_procfs_status_##N *dst;					\
{									\
  dst->pid = bfd_h_get_32 (abfd, (bfd_byte *) &src->pid);		\
  dst->tid = bfd_h_get_32 (abfd, (bfd_byte *) &src->tid);		\
  dst->flags = bfd_h_get_32 (abfd, (bfd_byte *) &src->flags);		\
  dst->why = bfd_h_get_16 (abfd, (bfd_byte *) &src->why);		\
  dst->what = bfd_h_get_16 (abfd, (bfd_byte *) &src->what);		\
  dst->ip = bfd_h_get_64 (abfd, (bfd_byte *) &src->ip);			\
  dst->sp = bfd_h_get_64 (abfd, (bfd_byte *) &src->sp);			\
  dst->stkbase = bfd_h_get_64 (abfd, (bfd_byte *) &src->stkbase);	\
  dst->stksize = bfd_h_get_32 (abfd, (bfd_byte *) &src->stksize);	\
  dst->tls = bfd_h_get_64 (abfd, (bfd_byte *) &src->tls);		\
  dst->tid_flags = bfd_h_get_32 (abfd, (bfd_byte *) &src->tid_flags);	\
  dst->syscall = bfd_h_get_16 (abfd, (bfd_byte *) &src->syscall);	\
  dst->last_cpu = bfd_h_get_16 (abfd, (bfd_byte *) &src->last_cpu);	\
  dst->timeout = bfd_h_get_32 (abfd, (bfd_byte *) &src->timeout);	\
  dst->last_chid = bfd_h_get_32 (abfd, (bfd_byte *) &src->last_chid);	\
  dst->sig_blocked.bits[0] =						\
    bfd_h_get_32 (abfd, (bfd_byte *) &src->sig_blocked.bits0[0]);	\
  dst->sig_blocked.bits[1] =						\
    bfd_h_get_32 (abfd, (bfd_byte *) &src->sig_blocked.bits1[0]);	\
  dst->sig_pending.bits[0] =						\
    bfd_h_get_32 (abfd, (bfd_byte *) &src->sig_pending.bits0[0]);	\
  dst->sig_pending.bits[1] =						\
    bfd_h_get_32 (abfd, (bfd_byte *) &src->sig_pending.bits1[0]);	\
  dst->info.si_signo =							\
    bfd_h_get_32 (abfd, (bfd_byte *) &src->info.si_signo);		\
}

ELF_SWAP_PRSTATUS_IN(16)
ELF_SWAP_PRSTATUS_IN(32)
ELF_SWAP_PRSTATUS_IN(64)

/* Locate the BFD section that came from the PT_NOTE program segment,
   iterate over the individual notes, and stash away whatever information
   we will need later. */

static void
process_corefile_notes (bfd *abfd, asection *sectp, PTR arg)
{
  const char *name;
  unsigned int sectsize;
  char *rawnotes;
  char *p;
  Elf_External_Note *xnotep;
  Elf_Internal_Note inote;
  external_procfs_info *external_psinfo;
  internal_procfs_info internal_psinfo;
  union {
    void *ptr;
    external_procfs_status_16 *ptr_16;
    external_procfs_status_32 *ptr_32;
    external_procfs_status_64 *ptr_64;
  } external_thread_status;
  union {
    void *ptr;
    internal_procfs_status_16 *ptr_16;
    internal_procfs_status_32 *ptr_32;
    internal_procfs_status_64 *ptr_64;
  } internal_thread_status;
  int external_thread_status_size = 0;
  char *fpregs;
  file_ptr gpregs_offset = 0;
  char secname[100];
  asection *newsect;
  int tindex;
  int *resultp = (int *) arg;

  long sizeof_external_procfs_status;
  long sizeof_internal_procfs_status;
  switch (TARGET_PTR_BIT)
    {
    case 16:
      sizeof_external_procfs_status = sizeof (external_procfs_status_16);
      sizeof_internal_procfs_status = sizeof (internal_procfs_status_16);
      break;
    case 32:
      sizeof_external_procfs_status = sizeof (external_procfs_status_32);
      sizeof_internal_procfs_status = sizeof (internal_procfs_status_32);
      break;
    case 64:
      sizeof_external_procfs_status = sizeof (external_procfs_status_64);
      sizeof_internal_procfs_status = sizeof (internal_procfs_status_64);
      break;
    default:
      internal_error (__FILE__, __LINE__,
		      "process_corefile_notes: unknown TARGET_PTR_BIT value %d",
		      TARGET_PTR_BIT);
    }

  name = bfd_get_section_name (abfd, sectp);
  if (strncmp (name, "note", 4) == 0)
    {
      sectsize = bfd_section_size (abfd, sectp);
      if (sectsize > 0)
	{
	  rawnotes = (unsigned char *) alloca (sectsize);
	  bfd_get_section_contents (abfd, sectp, (PTR) rawnotes, (file_ptr) 0, (bfd_size_type) sectsize);
	  p = rawnotes;
	  while (p < rawnotes + sectsize)
	    {
	      /* FIXME: Possibly bad alignment assumption. */
	      xnotep = (Elf_External_Note *) p;
	      inote.type = bfd_h_get_32 (abfd, (bfd_byte *) xnotep->type);
	      inote.namesz = bfd_h_get_32 (abfd, (bfd_byte *) xnotep->namesz);
	      inote.namedata = xnotep->name;
	      inote.descsz = bfd_h_get_32 (abfd, (bfd_byte *) xnotep->descsz);
	      inote.descdata = inote.namedata + align_power (inote.namesz, 2);
	      inote.descpos = sectp->filepos + (inote.descdata - rawnotes);
	      switch (inote.type)
		{
		case NT_PROCFS_STATUS_ARRAY:
		  external_thread_status.ptr = inote.descdata;
		  external_thread_status_size = inote.descsz;
		  break;
		case NT_PROCFS_FPREG_ARRAY:
		  fpregs = (char *) inote.descdata;
		  break;
		case NT_PROCFS_INFO:
		  external_psinfo = (external_procfs_info *) inote.descdata;
		  if (inote.descsz != sizeof (external_procfs_info))
		    {
		      bfd_set_error (bfd_error_wrong_format);
		      return;
		    }
		  elf_swap_psinfo_in (abfd, external_psinfo, &internal_psinfo);
		  break;
		case NT_PROCFS_GREG_ARRAY:
		  gpregs_offset = inote.descpos;
		  break;
		case NT_ROUTER_MODEL:
		  break;
		case NT_VERSION_STRING:
		  break;
		}
	      p = inote.descdata + align_power (inote.descsz, 2);
	    }
	  if ((internal_psinfo.num_threads * sizeof_external_procfs_status)
	      != external_thread_status_size)
	    {
	      bfd_set_error (bfd_error_wrong_format);
	      return;
	    }
	  internal_thread_status.ptr = alloca (internal_psinfo.num_threads
					       * sizeof_internal_procfs_status);
	  for (tindex = 0; tindex < internal_psinfo.num_threads; tindex++)
	    {
	      switch (TARGET_PTR_BIT)
		{
		case 16:
		  elf_swap_prstatus_in_16 (abfd,
					   &external_thread_status.ptr_16[tindex],
					   &internal_thread_status.ptr_16[tindex]);
		  sprintf (secname, ".reg/%d", internal_thread_status.ptr_16[tindex].tid);
		  break;
		case 32:
		  elf_swap_prstatus_in_32 (abfd,
					   &external_thread_status.ptr_32[tindex],
					   &internal_thread_status.ptr_32[tindex]);
		  sprintf (secname, ".reg/%d", internal_thread_status.ptr_32[tindex].tid);
		  break;
		case 64:
		  elf_swap_prstatus_in_64 (abfd,
					   &external_thread_status.ptr_64[tindex],
					   &internal_thread_status.ptr_64[tindex]);
		  sprintf (secname, ".reg/%d", internal_thread_status.ptr_64[tindex].tid);
		  break;
		default:
		  internal_error (__FILE__, __LINE__,
				  "process_corefile_notes: unknown TARGET_PTR_BIT value %d",
				  TARGET_PTR_BIT);
		}
	      newsect = make_bfd_asection (abfd, secname, SEC_HAS_CONTENTS,
					   (bfd_size_type) sizeof (external_procfs_greg),
					   (bfd_vma) 0,
					   gpregs_offset + (tindex * sizeof (external_procfs_greg)));
	      if (!newsect)
		{
		  bfd_set_error (bfd_error_no_memory);
		  return;
		}
	    }
	  /* Make a .reg section from the first procfs_greg array member */
	  newsect = make_bfd_asection (abfd, ".reg", SEC_HAS_CONTENTS,
				       (bfd_size_type) sizeof (external_procfs_greg),
				       (bfd_vma) 0,
				       gpregs_offset);
	  if (!newsect)
	    {
	      bfd_set_error (bfd_error_no_memory);
	      return;
	    }
	  switch (TARGET_PTR_BIT)
	    {
	    case 16:
	      elf_tdata (abfd)->core_signal = internal_thread_status.ptr_16[0].info.si_signo;
	      break;
	    case 32:
	      elf_tdata (abfd)->core_signal = internal_thread_status.ptr_32[0].info.si_signo;
	      break;
	    case 64:
	      elf_tdata (abfd)->core_signal = internal_thread_status.ptr_64[0].info.si_signo;
	      break;
	    }
	}
    }
  *resultp = 1;
}

static int
cisco_ena_core_sniffer (struct core_fns *cf, bfd *abfd)
{
  int result = 0;

  if ((bfd_get_format (abfd) == bfd_core) &&
      bfd_get_flavour (abfd) == cf -> core_flavour)
    {
      bfd_map_over_sections (abfd, process_corefile_notes, (PTR) &result);
    }
  return (result);
}

/* Register that we are able to handle Cisco ENA core files */

static struct core_fns cisco_ena_core_fns =
{
  bfd_target_elf_flavour,	/* core_flavour */
  default_check_format,		/* check_format */
  cisco_ena_core_sniffer,	/* core_sniffer */
  fetch_cisco_core_registers,	/* core_read_registers */
  NULL				/* next */
};

void
_initialize_cisco_ena_core (void)
{
  add_core_fns (&cisco_ena_core_fns);
}
