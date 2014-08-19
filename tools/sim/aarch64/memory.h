/* memory.h -- Prototypes for AArch64 memory accessor functions.

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

#ifndef _MEMORY_H
#define _MEMORY_H

#include <sys/types.h>
#include "bfd.h"
#include "simulator.h"

extern bfd_boolean  aarch64_load (bfd *);
extern void         aarch64_unload (void);

extern float        aarch64_get_mem_float (u_int64_t);
extern double       aarch64_get_mem_double (u_int64_t);
extern void         aarch64_get_mem_long_double (u_int64_t, FRegister *);

extern u_int64_t    aarch64_get_mem_u64 (u_int64_t);
extern int64_t      aarch64_get_mem_s64 (u_int64_t);
extern u_int32_t    aarch64_get_mem_u32 (u_int64_t);
extern int32_t      aarch64_get_mem_s32 (u_int64_t);
extern u_int32_t    aarch64_get_mem_u16 (u_int64_t);
extern int32_t      aarch64_get_mem_s16 (u_int64_t);
extern u_int32_t    aarch64_get_mem_u8  (u_int64_t);
extern int32_t      aarch64_get_mem_s8  (u_int64_t);
extern StatusCode   aarch64_get_mem_blk (u_int64_t, char *, unsigned);
extern const char * aarch64_get_mem_ptr (u_int64_t);

extern StatusCode   aarch64_set_mem_float (u_int64_t, float);
extern StatusCode   aarch64_set_mem_double (u_int64_t, double);
extern StatusCode   aarch64_set_mem_long_double (u_int64_t, FRegister);

extern StatusCode   aarch64_set_mem_u64 (u_int64_t, u_int64_t);
extern StatusCode   aarch64_set_mem_s64 (u_int64_t, int64_t);
extern StatusCode   aarch64_set_mem_u32 (u_int64_t, u_int32_t);
extern StatusCode   aarch64_set_mem_s32 (u_int64_t, int32_t);
extern StatusCode   aarch64_set_mem_u16 (u_int64_t, u_int16_t);
extern StatusCode   aarch64_set_mem_s16 (u_int64_t, int16_t);
extern StatusCode   aarch64_set_mem_u8  (u_int64_t, u_int8_t);
extern StatusCode   aarch64_set_mem_s8  (u_int64_t, int8_t);

#define STACK_TOP   0x07FFFF00

extern u_int64_t    aarch64_get_heap_start (void);
extern u_int64_t    aarch64_get_stack_start (void);
extern u_int64_t    aarch64_get_start_pc (void);

extern void         mem_add_blk (u_int64_t, char *, u_int64_t, bfd_boolean);

#endif /* _MEMORY_H */
