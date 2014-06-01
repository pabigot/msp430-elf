/*
 * insn.h -- MIPS instruction descriptions.
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

#ifndef __MIPS_INSN_H__
#define __MIPS_INSN_H__

#if defined(__MIPSEB__)
/* immediate Type */
struct i_type {
    unsigned int op : 6;
    unsigned int rs : 5;
    unsigned int rt : 5;
    signed   int imm: 16;
};

/* jump type */
struct j_type {
    unsigned int op : 6;
    unsigned int target : 26;
};

/* register type */
struct r_type {
    unsigned int op : 6;
    unsigned int rs : 5;
    unsigned int rt : 5;
    unsigned int rd : 5;
    unsigned int re : 5;
    unsigned int func : 6;
};

/* fpu type */
struct f_type {
    unsigned int op : 6;
    unsigned int fmt : 5;
    unsigned int rt : 5;
    unsigned int rd : 5;
    unsigned int re : 5;
    unsigned int func : 6;
};


#else
/* little endian */

struct i_type {
    signed   int imm : 16;
    unsigned int rt  : 5;
    unsigned int rs  : 5;
    unsigned int op  : 6;
};

struct j_type {
    unsigned int target : 26;
    unsigned int op     : 6;
};

struct r_type {
    unsigned int func : 6;
    unsigned int re : 5;
    unsigned int rd : 5;
    unsigned int rt : 5;
    unsigned int rs : 5;
    unsigned int op : 6;
};

struct f_type {
    unsigned int func : 6;
    unsigned int re   : 5;
    unsigned int rd   : 5;
    unsigned int rt   : 5;
    unsigned int fmt  : 5;
    unsigned int op   : 6;
};

#endif

union mips_insn {
    unsigned int   word;
    struct i_type  i;
    struct j_type  j;
    struct r_type  r;
    struct f_type  f;
};


/*
 * OP Codes
 */
#define OP_SPECIAL 0
#define OP_REGIMM  1
#define OP_J	   2
#define OP_JAL     3
#define OP_BEQ     4
#define OP_BNE     5
#define OP_BLEZ    6
#define OP_BGTZ    7
#define OP_ADDI    8
#define OP_ADDIU   9
#define OP_SLTI    10
#define OP_SLTIU   11
#define OP_ANDI    12
#define OP_ORI     13
#define OP_XORI    14
#define OP_LUI     15
#define OP_COP0    16
#define OP_COP1    17
#define OP_COP2    18
#define OP_COP1X   19
#define OP_BEQL    20
#define OP_BNEL    21
#define OP_BLEZL   22
#define OP_BGTZL   23
#define OP_DADDI   24
#define OP_DADDIU  25
#define OP_LDL     26
#define OP_LDR     27
#define OP_LB      32
#define OP_LH      33
#define OP_LWL     34
#define OP_LW      35
#define OP_LBU     36
#define OP_LHU     37
#define OP_LWR     38
#define OP_LWU     39
#define OP_SB      40
#define OP_SH      41
#define OP_SWL     42
#define OP_SW      43
#define OP_SDL     44
#define OP_SDR     45
#define OP_SWR     46
#define OP_CACHE   47
#define OP_LL      48
#define OP_LWC1    49
#define OP_LWC2    50
#define OP_PREF    51
#define OP_LLD     52
#define OP_LDC1    53
#define OP_LDC2    54
#define OP_LD      55
#define OP_SC      56
#define OP_SWC1    57
#define OP_SWC2    58
#define OP_SCD     60
#define OP_SDC1    61
#define OP_SDC2    62
#define OP_SD      63



/*
 * 'func' field for OP_SPECIAL opcodes
 */
#define FUNC_SLL     0
#define FUNC_MOVC    1
#define FUNC_SRL     2
#define FUNC_SRA     3
#define FUNC_SLLV    4
#define FUNC_SRLV    6
#define FUNC_SRAV    7
#define FUNC_JR      8
#define FUNC_JALR    9
#define FUNC_MOVZ    10
#define FUNC_MOVN    11
#define FUNC_SYSCALL 12
#define FUNC_BREAK   13
#define FUNC_SPIM    14
#define FUNC_SYNC    15
#define FUNC_MFHI    16
#define FUNC_MTHI    17
#define FUNC_MFLO    18
#define FUNC_MTLO    19
#define FUNC_DSLLV   20
#define FUNC_DSRLV   22
#define FUNC_DSRAV   23
#define FUNC_MULT    24
#define FUNC_MULTU   25
#define FUNC_DIV     26
#define FUNC_DIVU    27
#define FUNC_DMULT   28
#define FUNC_DMULTU  29
#define FUNC_DDIV    30
#define FUNC_DDIVU   31
#define FUNC_ADD     32
#define FUNC_ADDU    33
#define FUNC_SUB     34
#define FUNC_SUBU    35
#define FUNC_AND     36
#define FUNC_OR      37
#define FUNC_XOR     38
#define FUNC_NOR     39
#define FUNC_SLT     42
#define FUNC_SLTU    43
#define FUNC_DADD    44
#define FUNC_DADDU   45
#define FUNC_DSUB    46
#define FUNC_DSUBU   47
#define FUNC_TGE     48
#define FUNC_TGEU    49
#define FUNC_TLT     50
#define FUNC_TLTU    51
#define FUNC_TEQ     52
#define FUNC_TNE     54
#define FUNC_DSLL    56
#define FUNC_DSRL    58
#define FUNC_DSRA    59
#define FUNC_DSLL32  60
#define FUNC_DSRL32  62
#define FUNC_DSRA32  63


/*
 *  RT field of OP_REGIMM opcodes.
 */
#define RT_BLTZ     0
#define RT_BGEZ     1
#define RT_BLTZL    2
#define RT_BGEZL    3
#define RT_SPIMI    4
#define RT_TGEI     8
#define RT_TGEIU    9
#define RT_TLTI     10
#define RT_TLTIU    11
#define RT_TEQI     12
#define RT_TNEI     14
#define RT_BLTZAL   16
#define RT_BGEZAL   17
#define RT_BLTZALL  18
#define RT_BGEZALL  19


/*
 *  COPz rs field
 */
#define COP_RS_MF   0
#define COP_RS_DMF  1
#define COP_RS_CF   2
#define COP_RS_MT   4
#define COP_RS_DMT  5
#define COP_RS_CT   6
#define COP_RS_BC   8
#define COP_RS_CO   16

/*
 *  COPz rt field
 */
#define COP_RT_BCF  0
#define COP_RT_BCT  1
#define COP_RT_BCFL 2
#define COP_RT_BCTL 3

/*
 *  COP0 func field for COP_RS_C0 opcodes.
 */
#define COP0_FUNC_TLBR   1
#define COP0_FUNC_TLBWI  2
#define COP0_FUNC_TLBWR  6
#define COP0_FUNC_TLBP   8
#define COP0_FUNC_RFE    16
#define COP0_FUNC_ERET   24


/*
 * COP1 fmt field
 */
#define COP1_FMT_MF   0
#define COP1_FMT_DMF  1
#define COP1_FMT_CF   2
#define COP1_FMT_MT   4
#define COP1_FMT_DMT  5
#define COP1_FMT_CT   6
#define COP1_FMT_BC   8
#define COP1_FMT_S    16
#define COP1_FMT_D    17
#define COP1_FMT_W    20
#define COP1_FMT_L    21

/*
 * COP1 rt field for COP1_FMT_BC opcodes
 */
#define COP1_BC_RT_F  0
#define COP1_BC_RT_T  1
#define COP1_BC_RT_FL 2
#define COP1_BC_RT_TL 3


/*
 * COP1 func field for COP1_FMT_S opcodes
 */
#define COP1_SFUNC_ADD      0
#define COP1_SFUNC_SUB      1
#define COP1_SFUNC_MUL      2
#define COP1_SFUNC_DIV      3
#define COP1_SFUNC_SQRT     4
#define COP1_SFUNC_ABS      5
#define COP1_SFUNC_MOV      6
#define COP1_SFUNC_NEG      7
#define COP1_SFUNC_ROUND_L  8
#define COP1_SFUNC_TRUNC_L  9
#define COP1_SFUNC_CEIL_L   10
#define COP1_SFUNC_FLOOR_L  11
#define COP1_SFUNC_ROUND_W  12
#define COP1_SFUNC_TRUNC_W  13
#define COP1_SFUNC_CEIL_W   14
#define COP1_SFUNC_FLOOR_W  15
#define COP1_SFUNC_MOVCF    17
#define COP1_SFUNC_MOVZ     18
#define COP1_SFUNC_MOVN     19
#define COP1_SFUNC_RECIP    21
#define COP1_SFUNC_RSQRT    22
#define COP1_SFUNC_CVT_D    33
#define COP1_SFUNC_CVT_W    36
#define COP1_SFUNC_CVT_L    37
#define COP1_SFUNC_C_F      48
#define COP1_SFUNC_C_UN     49
#define COP1_SFUNC_C_EQ     50
#define COP1_SFUNC_C_UEQ    51
#define COP1_SFUNC_C_OLT    52
#define COP1_SFUNC_C_ULT    53
#define COP1_SFUNC_C_OLE    54
#define COP1_SFUNC_C_ULE    55
#define COP1_SFUNC_C_SF     56
#define COP1_SFUNC_C_NGLE   57
#define COP1_SFUNC_C_SEQ    58
#define COP1_SFUNC_C_NGL    59
#define COP1_SFUNC_C_LT     60
#define COP1_SFUNC_C_NGE    61
#define COP1_SFUNC_C_LE     62
#define COP1_SFUNC_C_NGT    63


/*
 * COP1 func field for COP1_FMT_D opcodes
 */
#define COP1_DFUNC_ADD      0
#define COP1_DFUNC_SUB      1
#define COP1_DFUNC_MUL      2
#define COP1_DFUNC_DIV      3
#define COP1_DFUNC_SQRT     4
#define COP1_DFUNC_ABS      5
#define COP1_DFUNC_MOV      6
#define COP1_DFUNC_NEG      7
#define COP1_DFUNC_ROUND_L  8
#define COP1_DFUNC_TRUNC_L  9
#define COP1_DFUNC_CEIL_L   10
#define COP1_DFUNC_FLOOR_L  11
#define COP1_DFUNC_ROUND_W  12
#define COP1_DFUNC_TRUNC_W  13
#define COP1_DFUNC_CEIL_W   14
#define COP1_DFUNC_FLOOR_W  15
#define COP1_DFUNC_MOVCF    17
#define COP1_DFUNC_MOVZ     18
#define COP1_DFUNC_MOVN     19
#define COP1_DFUNC_RECIP    21
#define COP1_DFUNC_RSQRT    22
#define COP1_DFUNC_CVT_S    32
#define COP1_DFUNC_CVT_W    36
#define COP1_DFUNC_CVT_L    37
#define COP1_DFUNC_C_F      48
#define COP1_DFUNC_C_UN     49
#define COP1_DFUNC_C_EQ     50
#define COP1_DFUNC_C_UEQ    51
#define COP1_DFUNC_C_OLT    52
#define COP1_DFUNC_C_ULT    53
#define COP1_DFUNC_C_OLE    54
#define COP1_DFUNC_C_ULE    55
#define COP1_DFUNC_C_SF     56
#define COP1_DFUNC_C_NGLE   57
#define COP1_DFUNC_C_SEQ    58
#define COP1_DFUNC_C_NGL    59
#define COP1_DFUNC_C_LT     60
#define COP1_DFUNC_C_NGE    61
#define COP1_DFUNC_C_LE     62
#define COP1_DFUNC_C_NGT    63


/*
 * COP1 func field for COP1_FMT_W & COP1_FMT_L opcodes
 */
#define COP1_WLFUNC_CVT_S   32
#define COP1_WLFUNC_CVT_D   33

/*
 * COP1X func field for COP1X opcodes
 */
#define COP1X_FUNC_LWXC1     0
#define COP1X_FUNC_LDXC1     1
#define COP1X_FUNC_SWXC1     8
#define COP1X_FUNC_SDXC1     9
#define COP1X_FUNC_PREFX     15
#define COP1X_FUNC_MADD_S    32
#define COP1X_FUNC_MADD_D    33
#define COP1X_FUNC_MSUB_S    40
#define COP1X_FUNC_MSUB_D    41
#define COP1X_FUNC_NMADD_S   48
#define COP1X_FUNC_NMADD_D   49
#define COP1X_FUNC_NMSUB_S   56
#define COP1X_FUNC_NMSUB_D   57

#endif /* __MIPS_INSN_H__ */
