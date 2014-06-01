/* mpc_random -- Generate a random complex number.

Copyright (C) 2002, 2009 Andreas Enge, Paul Zimmermann, Philippe Th\'eveny

This file is part of the MPC Library.

The MPC Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version.

The MPC Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the MPC Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. */

#include "mpc-impl.h"

void
mpc_random (mpc_ptr a)
{
   mpfr_set_prec (MPC_RE (a), MPFR_PREC (MPC_RE (a)) + 1);
   mpfr_random (MPC_RE(a));
   mpfr_mul_2ui (MPC_RE (a), MPC_RE (a), 1, GMP_RNDN);
   mpfr_sub_ui (MPC_RE (a), MPC_RE (a), 1, GMP_RNDN);
   mpfr_round_prec (MPC_RE (a), GMP_RNDZ, MPFR_PREC (MPC_RE (a)) - 1);
   
   mpfr_set_prec (MPC_IM (a), MPFR_PREC (MPC_IM (a)) + 1);
   mpfr_random (MPC_IM(a));
   mpfr_mul_2ui (MPC_IM (a), MPC_IM (a), 1, GMP_RNDN);
   mpfr_sub_ui (MPC_IM (a), MPC_IM (a), 1, GMP_RNDN);
   mpfr_round_prec (MPC_IM (a), GMP_RNDZ, MPFR_PREC (MPC_IM (a)) - 1);
}
