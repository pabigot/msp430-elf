/* syscalls.c --- system call handler for RL78 simulator.

   Copyright (C) 2011
   Free Software Foundation, Inc.
   Contributed by Red Hat, Inc.

   This file is part of the GNU simulators.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "opcode/rl78.h"
#include "mem.h"
#include "cpu.h"
#include "syscalls.h"

/* The RL78 sim syscall interface is as follows:

   AL = syscall number
   stack = parameters
   R8-11 = return value as needed

   The expected setup is:

   _sys:
      mov  A,#callnum
      mov  PMC,#0x55
      (if C is set, store AX into errno)
      ret

   A write of 0x55 to PMC is not a valid PMC value, it triggers the
   syscall (and does not change the actual PMC).

*/

#define tprintf if (trace) printf

extern jmp_buf decode_jmp_buf;
#define DO_RETURN(x) longjmp (decode_jmp_buf, x)

#define SIM_MAX_FDS 30

typedef struct {
  int native_fd;
  char is_valid;
  char opened_by_sim;
} SimFDEntry;

static int syscalls_initted = 0;
static SimFDEntry sim_file_descriptors[SIM_MAX_FDS];

static void
init_syscalls ()
{
  int i;
  for (i=0; i<3; i++)
    {
      sim_file_descriptors[i].native_fd = i;
      sim_file_descriptors[i].is_valid = 1;
      sim_file_descriptors[i].opened_by_sim = 0;
    }
  for (i=3; i<SIM_MAX_FDS; i++)
    {
      sim_file_descriptors[i].is_valid = 0;
    }
  syscalls_initted = 1;
}

static int
sim_alloc_fd ()
{
  int i;
  for (i=0; i<SIM_MAX_FDS; i++)
    if (! sim_file_descriptors[i].is_valid)
      return i;
  return -1;
}

static int
arg(int *sp)
{
  int a = memory[*sp] + 256 * memory[*sp+1];
  *sp += 2;
  return a;
}

static int
arg4(int *sp)
{
  int a = (memory[*sp]
	   + (memory[*sp+1] << 8)
	   + (memory[*sp+2] << 16)
	   + (memory[*sp+3] << 24));
  *sp += 4;
  if (a & 0x80000000UL)
    a = (int) ((long long)a - 0x1000000ULL);
  return a;
}

#define ARG() arg(&sp)
#define ARG4() arg4(&sp)

#define ERRNO(x)	\
  { set_carry (1);		\
    RET (-1);		\
    break;		\
  }

#define VALIDATE_FD(fd)				\
  if (fd < 0 || fd >= SIM_MAX_FDS		\
      || ! sim_file_descriptors [fd].is_valid)	\
    {						\
      ERRNO(1)					\
    }						\
  else						\
    fd = sim_file_descriptors [fd].native_fd

/* start-sanitize-rl78 */
#define RVREG (rl78_g10_mode ? 0xffec8 : 0xffef0)
#define RET(x) mem_put_hi (RVREG, x)
#define RET4(x) mem_put_si (RVREG, x)
#if 0
/* end-sanitize-rl78 */
#define RET(x) mem_put_hi (0xffef0, x)
#define RET4(x) mem_put_si (0xffef0, x)
/* start-sanitize-rl78 */
#endif
/* end-sanitize-rl78 */
    
static char *buf = NULL;
static int buflen = 0;

static void
need_buf (int len)
{
  if (buflen < len || buf == NULL)
    {
      buflen = len;
      if (buf)
	buf = (char *) realloc (buf, buflen);
      else
	buf = (char *) malloc (buflen);
    }
}

static void
fetch_buf (int addr, int len)
{
  need_buf (len);
  mem_get_blk (addr, buf, len);
}

static void
fetch_string (int addr)
{
  int i;
  for (i=addr; i<0xfffff; i++)
    if (mem_get_qi (i) == 0)
      break;
  need_buf (i - addr + 1);
  mem_get_blk (addr, buf, i - addr + 1);
}

static void
store_buf (int addr, int len)
{
  mem_put_blk (addr, buf, len);
}

void
rl78_syscall ()
{
  int a, b, c, d, fd;
  int sp;
  int syscall_number;

  if (!syscalls_initted)
    init_syscalls ();

  set_carry (0);

  syscall_number = get_reg (RL78_Reg_A);
  sp = get_reg (RL78_Reg_SP) + 0xf0000 + 4;

  switch (syscall_number)
    {

    case SYS_exit:	/* (int exit_code) */
      a = ARG();
      tprintf("SYSCALL: exit(%d)\n", a);
      DO_RETURN (RL78_MAKE_EXITED (a));
      break;

    case SYS_open:	/* (char *name, int flags) */
      a = ARG() + 0xf0000;
      b = ARG();
      c = ARG();

      fd = sim_alloc_fd ();
      if (fd < 0)
	ERRNO (1);

      fetch_string (a);
      d = 0;
      if (b & 0x0001) d |= O_WRONLY;
      if (b & 0x0002) d |= O_RDWR;
      if (b & 0x0200) d |= O_CREAT;
      if (b & 0x0400) d |= O_TRUNC;
      tprintf("SYSCALL: open(%s,0x%x,0%o)\n", buf, d, c);
      d = open (buf, d, c);
      if (d < 0)
	ERRNO (1);
      sim_file_descriptors[fd].native_fd = d;
      sim_file_descriptors[fd].is_valid = 1;
      sim_file_descriptors[fd].opened_by_sim = 1;
      RET (fd);
      break;

    case SYS_close:	/* (int fd) */
      VALIDATE_FD (a);
      /* Closing file descriptors 0, 1, or 2 does bad things when
         connected to GDB.  */
      if (a != 0 && a != 1 && a != 2)
	RET (close (a));
      break;

    case SYS_write:	/* (int fd, char *data, int length) */
      a = ARG();
      b = ARG() + 0xf0000;
      c = ARG();
      tprintf("SYSCALL: write(%d, 0x%x, %d)\n", a, b, c);
      VALIDATE_FD (a);
      if (b + c > 0xfffff)
	ERRNO (1);
      fetch_buf (b, c);
      RET (write (a, buf, c));
      break;

    case SYS_read:	/* (int fd, char *data, int length) */
      a = ARG();
      b = ARG() + 0xf0000;
      c = ARG();
      tprintf("SYSCALL: read(%d, 0x%x, %d)\n", a, b, c);
      VALIDATE_FD (a);
      if (b + c > 0xfffff)
	ERRNO (1);
      need_buf (c);
      a = read (a, buf, c);
      store_buf (b, c);
      RET (a);
      break;

    case SYS_lseek:	/* (int fd, off_t offset, int whence) */
      a = ARG();
      b = ARG4();
      c = ARG();
      tprintf("SYSCALL: lseek(%d, %d, %d)\n", a, b, c);
      VALIDATE_FD (a);
      a = lseek (a, b, c);
      RET4 (a);
      break;

    }
}
