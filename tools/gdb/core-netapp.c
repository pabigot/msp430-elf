/* Extract registers from a NetApp software core file, for GDB.

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

/* core.c is supposed to be the more machine-independent aspects of this;
   this file is more machine-specific although, like "coredep.c" whence
   it was derived, it may be processor-independent even though it's
   OS-dependent.  */

#include "defs.h"
#include <sys/types.h>
#include <sys/param.h>
#include "gdbcore.h"
#include "value.h" /* For supply_register.  */
#include "inferior.h" /* For registers. */

/* Work with core files, for GDB. */

static void
fetch_core_registers (char *core_reg_sect, unsigned core_reg_size, int which,
		      unsigned reg_addr)
{
  int val;

  switch (which) {
  case 0:
  case 1:
    memcpy (registers, core_reg_sect, core_reg_size);
    break;

  case 2:
    /*
     * We don't do floating-point under Bozo's Big Tent.
     */
    break;
  }
}


/* Register that we are able to handle netapp core file formats. */

static struct core_fns netapp_core_fns =
{
  bfd_target_aout_flavour,
  default_check_format,
  default_core_sniffer,
  fetch_core_registers,
  NULL
};

void
_initialize_core_netapp (void)
{
  add_core_fns (&netapp_core_fns);
}


#ifdef REGISTER_U_ADDR

/* Return the address in the core dump or inferior of register REGNO.
   BLOCKEND is the address of the end of the user structure.  */

unsigned int
register_addr (int regno, int blockend)
{
  int addr;

  if (regno < 0 || regno >= NUM_REGS)
    error ("Invalid register number %d.", regno);

  REGISTER_U_ADDR (addr, blockend, regno);

  return addr;
}

#endif /* REGISTER_U_ADDR */
