/*
 * singlestep.c -- AM30 specific single-step support.
 *
 * Copyright (c) 1998, 1999 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include <stdlib.h>
#include <bsp/bsp.h>
#include <bsp/cpu.h>

#define DEBUG_SINGLESTEP         0
#define DEBUG_SINGLESTEP_VERBOSE 0

/*
 * Structure to hold opcodes hoisted when breakpoints are
 * set for single-stepping or async interruption.
 */
struct _bp_save {
    unsigned char  *addr;
    unsigned char  opcode;
    unsigned char  pad[3];
};

static struct _bp_save _breaks[2];

/*
 *  Insert a breakpoint at 'pc' using first available
 *  _bp_save struct.
 */
static void
insert_ss_break(unsigned char *pc)
{
    struct _bp_save *p = _breaks;

    if (p->addr && (++p)->addr)
	return;

#if DEBUG_SINGLESTEP
    bsp_printf("Setting BP at <0x%x>\n", pc);
#endif /* DEBUG_SINGLESTEP */

    p->addr = pc;
    p->opcode = *pc;
    *pc = 0xff;
}


/*
 *  Cleanup after a singlestep.
 */
void
bsp_singlestep_cleanup(void *saved_regs)
{
    struct _bp_save *p = _breaks;

    if (p->addr) {
	*(p->addr) = p->opcode;
	p->addr = NULL;

	++p;
	if (p->addr) {
	    *(p->addr) = p->opcode;
	    p->addr = NULL;
	}
    }
}


/*
 *  Read a 16-bit displacement from address 'p'. The
 *  value is stored little-endian.
 */
static short
read_disp16(unsigned char *p)
{
    return (short)(p[0] | (p[1] << 8));
}

/*
 *  Read a 32-bit displacement from address 'p'. The
 *  value is stored little-endian.
 */
static int
read_disp32(unsigned char *p)
{
    return (int)(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
}


/*
 *  Get the contents of An register.
 */
static unsigned int
get_areg(ex_regs_t *regs, int n)
{
    switch (n) {
      case 0:
	return regs->_a0;
      case 1:
	return regs->_a1;
      case 2:
	return regs->_a2;
      case 3:
	return regs->_a3;
    }
    return 0;
}


/* Table of instruction sizes, indexed by first byte of instruction,
   used to determine the address of the next instruction for single stepping.
   If an entry is less than zero, the instruction size is the absolute value,
   but special code must handle the case (for example, branches or multi-byte
   opcodes).  */

static char opcode_size[256] =
{
     /*  0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f */
     /*----------------------------------------------------------------*/
/* 0 */	 1,  3,  3,  3,  1,  3,  3,  3,  1,  3,  3,  3,  1,  3,  3,  3,
/* 1 */  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* 2 */  2,  2,  2,  2,  3,  3,  3,  3,  2,  2,  2,  2,  3,  3,  3,  3,
/* 3 */  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  1,  1,  1,  1,
/* 4 */  1,  1,  2,  2,  1,  1,  2,  2,  1,  1,  2,  2,  1,  1,  2,  2,
/* 5 */  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,
/* 6 */  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* 7 */  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* 8 */  2,  1,  1,  1,  1,  2,  1,  1,  1,  1,  2,  1,  1,  1,  1,  2,
/* 9 */  2,  1,  1,  1,  1,  2,  1,  1,  1,  1,  2,  1,  1,  1,  1,  2,
/* a */  2,  1,  1,  1,  1,  2,  1,  1,  1,  1,  2,  1,  1,  1,  1,  2,
/* b */  2,  1,  1,  1,  1,  2,  1,  1,  1,  1,  2,  1,  1,  1,  1,  2,
/* c */ -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  1, -3, -5,  2,  2,
/* d */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -5, -7, -3, -3,
/* e */  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* f */ -2,  2,  2,  2,  2,  2,  2,  1, -3,  3, -4,  4, -6,  6,  7,  1
};


/*
 * Set breakpoint instructions for single stepping.
 */
void
bsp_singlestep_setup(void *saved_regs)
{
    ex_regs_t *regs = saved_regs;
    unsigned char *pc = (unsigned char *)regs->_pc;
    unsigned int opcode;
    int          displ;

    opcode = *pc;

    /* Check the table for the simple cases.  */
    displ = opcode_size[opcode];
    if (displ > 0)
    {
	insert_ss_break((char *)pc + displ);
	return;
    }

    switch (opcode) {
      case 0xc0:
      case 0xc1:
      case 0xc2:
      case 0xc3:
      case 0xc4:
      case 0xc5:
      case 0xc6:
      case 0xc7:
      case 0xc8:
      case 0xc9:
      case 0xca:
	/*
	 *  bxx (d8,PC)
	 */
	displ = *((char *)pc + 1);
	insert_ss_break(pc + 2);
	if (displ < 0 || displ > 2)
	    insert_ss_break(pc + displ);
	break;

      case 0xd0:
      case 0xd1:
      case 0xd2:
      case 0xd3:
      case 0xd4:
      case 0xd5:
      case 0xd6:
      case 0xd7:
      case 0xd8:
      case 0xd9:
      case 0xda:
	/*
	 *  lxx (d8,PC)
	 */
	if (pc != (unsigned char*)regs->_lar)
	    insert_ss_break((char *)regs->_lar);
	insert_ss_break(pc + 1);
	break;

      case 0xdb:
	/*
	 * setlb requires special attention. It loads the next four instruction
	 * bytes into the LIR register, so we can't insert a breakpoint in any
	 * of those locations.
	 */
	insert_ss_break(pc + 5);
	break;

      case 0xcc:
      case 0xcd:
	/*
	 * jmp (d16,PC) or call (d16,PC)
	 */
	displ = read_disp16((char *)(pc + 1));
	insert_ss_break(pc + displ);
	break;

      case 0xdc:
      case 0xdd:
	/*
	 * jmp (d32,PC) or call (d32,PC)
	 */
	displ = read_disp32((char *)(pc + 1));
	insert_ss_break(pc + displ);
	break;

      case 0xde:
	/*
	 *  retf
	 */
	insert_ss_break((char *)regs->_mdr);
	break;

      case 0xdf:
	/*
	 *  ret
	 */
	displ = *((char *)pc + 2);
	insert_ss_break((char *)read_disp32((unsigned char *)regs->_sp + displ));
	break;

      case 0xf0:
	/*
	 *  Some branching 2-byte instructions.
	 */
	opcode = *(pc + 1);
	if (opcode >= 0xf0 && opcode <= 0xf7) {
	    /* jmp (An) / calls (An) */
	    insert_ss_break((unsigned char *)get_areg(regs, opcode & 3));

	} else if (opcode == 0xfc) {
	    /* rets */
	    insert_ss_break((unsigned char *)read_disp32((unsigned char *)regs->_sp));
      
	} else if (opcode == 0xfd) {
	    /* rti */
	    insert_ss_break((unsigned char *)read_disp32((unsigned char *)regs->_sp + 4));

	} else 
	    insert_ss_break(pc + 2);

	break;

      case 0xf8:
	/*
	 *  Some branching 3-byte instructions.
	 */
	opcode = *(pc + 1);
	if (opcode >= 0xe8 && opcode <= 0xeb) {
	    displ = *((char *)pc + 2);
	    insert_ss_break(pc + 3);
	    if (displ < 0 || displ > 3)
		insert_ss_break(pc + displ);
      
	} else
	    insert_ss_break(pc + 3);
	break;

      case 0xfa:
	opcode = *(pc + 1);
	if (opcode == 0xff) {
	    /* calls (d16,PC) */
	    displ = read_disp16((char *)(pc + 2));
	    insert_ss_break(pc + displ);
	} else
	    insert_ss_break(pc + 4);
	break;

      case 0xfc:
	opcode = *(pc + 1);
	if (opcode == 0xff) {
	    /* calls (d32,PC) */
	    displ = read_disp32((char *)(pc + 2));
	    insert_ss_break(pc + displ);
	} else
	    insert_ss_break(pc + 6);
	break;
    }
}


void
bsp_skip_instruction(void *saved_regs)
{
    unsigned int opcode;
    int          nbytes;

    opcode = *(unsigned char *)(((ex_regs_t *)saved_regs)->_pc);

    /* Check the table for the simple cases.  */
    nbytes = opcode_size[opcode];
    if (nbytes < 0)
	nbytes = -nbytes;

    ((ex_regs_t *)saved_regs)->_pc += nbytes;
}


