/* sim-aarch64.h --- interface between AArch64  simulator and GDB.

   Copyright 2013 by Red Hat Inc.

   THIS FILE IS NOT TO BE CONTRIBUTED.

   This file is part of GDB.  */

#if !defined (SIM_AARCH64_H)
#define SIM_AARCH64_H

enum sim_aarch64_regnum
{
  sim_aarch64_r0_regnum,
  sim_aarch64_r1_regnum,
  sim_aarch64_r2_regnum,
  sim_aarch64_r3_regnum,
  sim_aarch64_r4_regnum,
  sim_aarch64_r5_regnum,
  sim_aarch64_r6_regnum,
  sim_aarch64_r7_regnum,
  sim_aarch64_r8_regnum,
  sim_aarch64_r9_regnum,
  sim_aarch64_r10_regnum,
  sim_aarch64_r11_regnum,
  sim_aarch64_r12_regnum,
  sim_aarch64_r13_regnum,
  sim_aarch64_r14_regnum,
  sim_aarch64_r15_regnum,
  sim_aarch64_sp_regnum,
  sim_aarch64_pc_regnum,
  sim_aarch64_num_regs
};

#endif /* SIM_AARCH64_H */
