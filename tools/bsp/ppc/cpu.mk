

CPU_SOURCE = cpu.c         \
	     gdb-cpu.c     \
	     singlestep.c  \
	     irq-cpu.c     \
	     trap.S        \
	     generic-mem.c \
	     generic-reg.c \
	     c_start.c     \
	     c_crt0.c      \
	     rst.S


CPU_DEFINES = -DINIT_FUNC=__init -DFINI_FUNC=__fini