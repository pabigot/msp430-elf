/*  This file is part of the program psim.

    Copyright (C) 2002, Red Hat, Inc.

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

    */

#include "sim-main.h"
#include "vr_m32mipsIV_engine.h"
#include "vr_m32mipsIV_idecode.h"
#include "vr_m32vr4100_engine.h"
#include "vr_m32vr4100_idecode.h"
#include "vr_m16vr4111_engine.h"
#include "vr_m16vr4111_idecode.h"
#include "vr_m16vr4120_engine.h"
#include "vr_m16vr4120_idecode.h"
#include "bfd.h"

#define SD sd
#define CPU cpu

void
sim_engine_run (SIM_DESC sd,
		int next_cpu_nr,
		int nr_cpus, /* ignore */
		int siggnal) /* ignore */
{
  int mach;

  if (STATE_ARCHITECTURE (sd) == NULL)
    mach = 0;
  else
    mach = STATE_ARCHITECTURE (sd)->mach;

  switch (mach)
    {
    default:
    case bfd_mach_mips4000:
      vr_m32mipsIV_engine_run (sd, next_cpu_nr, nr_cpus, siggnal);
      break;
    case bfd_mach_mips4100:
      vr_m32vr4100_engine_run (sd, next_cpu_nr, nr_cpus, siggnal);
      break;
    case bfd_mach_mips4111:
      vr_m16vr4111_engine_run (sd, next_cpu_nr, nr_cpus, siggnal);
      break;
    case bfd_mach_mips4120:
      vr_m16vr4120_engine_run (sd, next_cpu_nr, nr_cpus, siggnal);
      break;
    case bfd_mach_mips5000:
      vr_m32vr5000_engine_run (sd, next_cpu_nr, nr_cpus, siggnal);
      break;
    case bfd_mach_mips5400:
      vr_m32vr5400_engine_run (sd, next_cpu_nr, nr_cpus, siggnal);
      break;
    case bfd_mach_mips5500:
      vr_m32vr5500_engine_run (sd, next_cpu_nr, nr_cpus, siggnal);
      break;
    }
}
