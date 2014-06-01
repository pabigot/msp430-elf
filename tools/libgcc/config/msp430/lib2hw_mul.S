;   Copyright (C) 2014 Free Software Foundation, Inc.
;   Contributed by Red Hat.
; 
; This file is free software; you can redistribute it and/or modify it
; under the terms of the GNU General Public License as published by the
; Free Software Foundation; either version 3, or (at your option) any
; later version.
; 
; This file is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
; General Public License for more details.
; 
; Under Section 7 of GPL version 3, you are granted additional
; permissions described in the GCC Runtime Library Exception, version
; 3.1, as published by the Free Software Foundation.
;
; You should have received a copy of the GNU General Public License and
; a copy of the GCC Runtime Library Exception along with this program;
; see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
; <http://www.gnu.org/licenses/>.

;; This file contains hardware multiply functions for the libgcc library.
;;
;; Widening operations are not included here because gcc does not support
;; widening and multiply via library operations.  Instead the widening
;; versions of the hardware multiplies are defined as inline code in msp430.md.

;; Macro: Declares the start of a multiply function.  They all use the same
;;  start sequence.  Interrupts are disabled because the memory registers
;;  being used are not guaranteed to be preserved across interrupts.
	
.macro start_func name
	.pushsection .text.\name,"ax",@progbits
	.align 2
	.global \name
	.type \name , @function
\name:
	PUSH.W	sr			; Save current interrupt state
	DINT				; Disable interrupts
	NOP				; Account for latency
.endm


;;  Macro: Declares the end of a multiply function.

.macro end_func name
#ifdef __MSP430X_LARGE__
	POP.W  sr
        RETA
#else
	RETI
#endif
	.size \name , . - \name
	.popsection
.endm

;;  Macro: Multiplies two 16-bit values, producing a 16-bit result.
;;  The arguments are the memory registers to be used.
;;  The macro uses R12 and R13 and changes R12.
	
.macro mult16 OP1, OP2, RESULT
	MOV.W	r12, &\OP1		; Load operand 1 into multiplier
	MOV.W	r13, &\OP2		; Load operand 2 which triggers MPY
	MOV.W	&\RESULT, r12		; Move result into return register
.endm

;;  Macro: Multiplies two 32-bit values, producing a 32-bit result.
;;  The arguments are the memory registers to be used.
;;  The macro uses R12, R13, R14 and R15 and changes R12 and R13.
	
.macro mult32  OP1_LO  OP1_HI  OP2_LO  OP2_HI  RESULT_LO  RESULT_HI
	MOV.W	r12, &\OP1_LO		; Load operand 1 Low into multiplier
	MOV.W	r13, &\OP1_HI		; Load operand 1 High into multiplier
	MOV.W	r14, &\OP2_LO		; Load operand 2 Low into multiplier
	MOV.W	r15, &\OP2_HI		; Load operand 2 High, trigger MPY
	MOV.W	&\RESULT_LO, r12	; Ready low 16-bits for return
	MOV.W   &\RESULT_HI, r13	; Ready high 16-bits for return
.endm

;;  Macro: Multiplies two 32-bit values, producing a 32-bit result using
;;    two 16-bit hardware multiplies.
;;  The arguments are the memory registers to be used.
;;  The macro uses R12, R13, R14 and R15 and changes R12 and R13.
	
.macro mult32_synth OP1, OP2, MAC_OP1, MAC_OP2, RESULT_LO, RESULT_HI
	
	MOV.W	r12, &\OP1		; Load operand 1 Low into multiplier
	MOV.W	r14, &\OP2		; Load operand 2 Low which triggers MPY
	MOV.W	r12, &\MAC_OP1		; Load operand 1 Low into mac
	MOV.W   &\RESULT_LO, r12	; Low 16-bits of result ready for return
	MOV.W   &\RESULT_HI, &\RESULT_LO; MOV intermediate mpy high into low
	MOV.W	r15, &\MAC_OP2		; Load operand 2 High, trigger MAC
	MOV.W	r13, &\MAC_OP1		; Load operand 1 High
	MOV.W	r14, &\MAC_OP2		; Load operand 2 Lo, trigger MAC
	MOV.W	&\RESULT_LO, r13        ; Upper 16-bits result ready for return
.endm


	
;; The multiply functions.  Note - the F5xxx series of MCUs support
;; the same 16-bit and 32-bit hardware multiply operations, but they
;;  are accessed from different memory registers.

start_func __mulhi2
	mult16 0x0130, 0x0138, 0x013A
end_func __mulhi2

start_func __mulhi2_f5
	mult16 0x04C0, 0x04C8, 0x04CA
end_func __mulhi2_f5


;; Most MCUs do not have a 32-bit hardware multiply, but one
;; can be synthesised using 16-bit operations.
start_func __mulsi2
	mult32_synth 0x0130, 0x0138, 0x0134, 0x0138, 0x013A, 0x013C
end_func __mulsi2

start_func __mulsi2_hw32
	mult32 0x0140, 0x0142, 0x0150, 0x0152, 0x0154, 0x0156
end_func __mulsi2_hw32

start_func __mulsi2_f5
	mult32 0x04D0, 0x04D2, 0x04E0, 0x04E2, 0x04E4, 0x04E6
end_func __mulsi2_f5
