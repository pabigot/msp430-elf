# Support macros for the assembly test cases.

	.macro start
	.text
	.global _start
_start:
	.endm

	.data

	.globl pass_str
	.globl fail_str
	.globl ok_str
pass_str:
	.ascii "pass\n"
fail_str:
	.ascii "fail\n"
ok_str:
	.ascii "ok\n"

	.text

	.macro pass
	li	%r0, 4	# write syscall
	li	%r3, 1	# stdout
	lis	%r4, pass_str@ha
	addi	%r4, %r4, pass_str@l
	li	%r5, 5
	sc
	li	%r0, 1	# exit syscall
	li	%r3, 0
	sc
	.endm

	.macro fail
	li	%r0, 4	# write syscall
	li	%r3, 1	# stdout
	lis	%r4, fail_str@ha
	addi	%r4, %r4, fail_str@l
	li	%r5, 5
	sc
	li	%r0, 1	# exit syscall
	li	%r3, 1
	sc
	.endm

	.macro ok
	li	%r0, 4	# write syscall
	li	%r3, 1	# stdout
	lis	%r4, ok_str@ha
	addi	%r4, %r4, ok_str@l
	li	%r5, 3
	sc
	.endm

# AltiVec tests with "# mach: powerpc" produce seemingly correct results
# on little endian targets, but different to big endian targets.  For now
# just don't run them on little endian.  Maybe copy them and use the
# alternate answers.  See vmrghb.le.s.
	.macro load_vr vr gpr addr
	lis	\gpr, \addr@ha
	addi	\gpr, \gpr, \addr@l
	lvx	\vr, 0, \gpr
	.endm

