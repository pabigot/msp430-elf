/* simulator.h -- Prototypes for AArch64 simulator functions.

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

#ifndef _SIMULATOR_H
#define _SIMULATOR_H

#include "config.h"
#include <sys/types.h>
#include <setjmp.h>

#include "cpustate.h"
#include "decode.h"

#define TRACE_MEM_WRITES (1 << 0)
#define TRACE_REG_WRITES (1 << 1)
#define TRACE_FUNCTIONS  (1 << 2)
#define TRACE_MISC       (1 << 3)

#define TRACE_ALL       ((1 << 4) - 1)

extern int trace;
extern int disas;

/*
 * call this to set the start stack pointer, frame pointer and pc
 * before calling run or step.  also sets link register to a special
 * value (-20) so we can detect a top level return. this method
 * should be called from the sim setup routine running on the alt
 * stack, and should pass in the current top of C stack for SP and
 * the FP of the caller for fp. PC shoudl be sthe start of the ARM
 * code segment to be executed.
 */

extern int aarch64_init (void);

/*
 * call this to run from the current PC without stopping until we
 * either return from the top frame, execute a halt or hit an error
 *
 * return value is one of
 *   AArch64Simulator::STATUS_RETURN
 *   AArch64Simulator::STATUS_HALT
 *   AArch64Simulator::STATUS_ERROR
 */

extern StatusCode aarch64_run (void);

/*
 * call this to step by one instruction. in this mode execution
 * pauses before a callout to x86 code. the client may call
 * simulator methods to proceed with the callout and return a
 * result. alernatively, the clinet may explicitly handle the call
 * and return.
 *
 * return value is one of
 *
 *  STATUS_READY : the simulator is readu to continue stepping
 *
 *  STATUS_RETURN : the simulator has returned from the starting
 *  frame
 *
 *  STATUS_HALT : the simulator has executed a halt
 *  (pseudo-)instruction
 *
 *  STATUS_HALT : the simulator has executed a BRK instruction
 *
 *  STATUS_CALLOUT : the simulator has executed a callout
 *  (pseudo-)instruction. the callout address and arguments should
 *  have been marshalled on the stack. a descriptor for the called
 *  function (supplied as an operand to the pseudo instruction) may
 *  be retrieved by calling getDescriptor().
 *
 *  STATUS_ERROR : the simulator has identified an errro situation.
 *  details of the error can be obtained by checking the error
 *  status.
 *
 */
extern StatusCode aarch64_step (void);

/*
 * error codes which further qualify a STATUS_ERROR return value.
 */

typedef enum
{
  ERROR_NONE = 0,            // No error pending
  ERROR_UNALLOC = 1,         // Hit a bad instruction
  ERROR_STACK_OVERFLOW = 2,  // Ran out of stack ???
  ERROR_CLIENT = 3,          // Client call is invalid
  ERROR_NYI = 4,             // Sim feature not yet implemented
  ERROR_EXCEPTION = 5,	     // Illegal memory access
  ERROR_MAX = 6
} ErrorCode;

extern u_int64_t *  aarch64_get_stack (void);
extern u_int64_t *  aarch64_get_stack_limit (void);
extern const char * aarch64_print_func (u_int64_t);
extern void         aarch64_print_insn (u_int64_t);
extern const char * aarch64_get_error_text (void);
extern StatusCode   aarch64_set_status (StatusCode, ErrorCode);
extern StatusCode   aarch64_get_status (void);
extern ErrorCode    aarch64_get_ErrorCode (void);
extern u_int64_t    aarch64_get_sym_value (const char *);

#endif // ifndef SIMULATOR_H
