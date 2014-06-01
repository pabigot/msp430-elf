# XSTORMY16 startup code

# Interrupt vectors at 0x8000.
	.section .int_vec,"ax"
	.global	_start
	.align 1
_start:
	;; Reset, watchdog timer interrupt
	jmpf _int_reset
	;; base timer interrupt
	jmpf _int_basetimer
	;; timer 0
	jmpf _int_timer0
	;; timer 1
	jmpf _int_timer1
	;; SIO0 interrupt
	jmpf _int_sio0
	;; SIO1 interrupt
	jmpf _int_sio1
	;; port0 interrupt
	jmpf _int_port0
	;; port1 interrupt
	jmpf _int_port1

# Reset code, set up memory and call main.
        .section        .rodata
2:	.word	__rdata
	.text
_int_reset:
# Check if watchdog timer was expired
	bn      0x7f0c, #4, _after_reset
	bp      0x7f0c, #5, _after_reset
	jmpf    _int_watchdog

# Reset code, set up memory and call main.
_after_reset:
	;; Set up the stack pointer.
	mov     r0, #__stack
        bz      r0, #0, 0f
	mov     sp, r0
0:
	;; Zero the .bss data space
	mov	r8, #@hi(__bss_start)
	mov	r9, #@hi(__bss_end)
	bnz	r8, r9, 3f	; We do not support more then 64K of .bss.
	mov	r0, #@lo(__bss_start)
	mov	r1, #@lo(__bss_end)
	bz      r0, r1, 1f	; Skip the loop if there is nothing to do.
	mov	r2, #0
0:	movf.w	(r8, r0++, 0), r2
	blt	r0, r1, 0b	
1:
	;; Copy data from ROM into RAM.  ROM area may be above 64k,
	;; but RAM may not.
	mov	r9, #@hi(__data)
	mov	r10,#@hi(_edata)
	bnz	r9, r10, 3f	; We do not support more than 64K of data.
	mov	r1, #@lo(__data)
	mov	r3, #@lo(_edata)
	mov	r0, #@lo(__rdata)
	mov	r8, #@hi(__rdata)

	;; If __data == __rdata there's no need to copy anything.
	bnz	r0, r1, 0f
	bz	r8, r9, 1f
0:	movf.w	r2, (r8, r0++, 0)
	movf.w	(r9, r1++, 0), r2
	blt	r1, r3, 0b
1:
	;; Call hardware init routine
	callf   _hwinit

	;; Call initialization routines
	callf   _init

	;; Set up fini routines to be called from exit
#ifdef __xstormy16_ptr32__
	mov     r2, #@lo(_fini)
	mov     r3, #@hi(_fini)
#else
	mov     r2, #@fptr(_fini)
#endif
	callf   atexit

	;; Call main() with empty argc/argv/envp
	mov     r2, #0
	mov     r3, #0
	mov     r4, #0
#ifdef __xstormy16_ptr32__
	mov     r5, #0
	mov     r6, #0
#endif
	callf   main

	;; Exit.
	callf   exit

	;; Should never reach this code.
3:	
	halt
1:	.size _int_reset, 1b - _int_reset
		
# Stub interrupt routines.
	.globl _int_watchdog
	.weak  _int_watchdog
	.globl _int_basetimer
	.weak  _int_basetimer
	.globl _int_timer0
	.weak _int_timer0
	.globl _int_timer1
	.weak _int_timer1
	.globl _int_sio0
	.weak _int_sio0
	.globl _int_sio1
	.weak _int_sio1
	.globl _int_port0
	.weak _int_port0
	.globl _int_port1
	.weak _int_port1
	.globl _int_basetimer
	.weak _int_basetimer
	.globl __int_default
_int_watchdog:
	br _after_reset
_int_basetimer:
_int_timer0:
_int_timer1:
_int_sio0:
_int_sio1:
_int_port0:
_int_port1:
_int_basetimer:
__int_default:
	iret
1:	.size _int_watchdog,1b-_int_watchdog

# Stub hardware init
	.globl _hwinit
	.weak _hwinit
_hwinit:
	ret
1:	.size _int_hwinit,1b-_int_hwinit

# The first word in .data has address 0, so it's not a good
# idea to use it as its address conflicts with NULL.
# Place a HALT instruction there to try to catch NULL pointer
# dereferences.
	.data
	halt
