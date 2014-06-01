///**********************************************************************:
//*: Copyright © Intel Corporation 1998.  All rights reserved.
//***********************************************************************:
//:
//:       Macros for SA-1 Coprocessor Access
//:
//:       $Id: mmumacro.s,v 1.1 2000/02/01 21:19:23 msalter Exp $
//:
//***********************************************************************/:


//        .include "mmu_h.s"


//: SA-110 *only* supports Coprocessor number 15
//: Only MCR and MRC coprocessor instructions are supported - the others
//: ...generate an UNDEFINED exception
//:
//: CP15 registers are architected as per the ARM V4 architecture spec
//:
//:       Register0       ID register                   READ_ONLY
//:       Register1       Control                       READ_WRITE
//:       Register2       Translation Table Base        READ_WRITE
//:       Register3       Domain Access Control         READ_WRITE
//:       Register4       Reserved
//:       Register5       Fault Status                  READ_WRITE
//:       Register6       Fault Address                 READ_WRITE
//:       Register7       Cache Operations              WRITE_ONLY
//:       Register8       TLB Operations                WRITE_ONLY
//:       Register9-14    Reserved
//:       Register15      SA-110 specific tst/clk/idle  WRITE_ONLY

////: Defined Macros:      
////:
//RDCP15_ID:               Rx     read of ID register       
//RDCP15_Control:          Rx     read of Control register  
//WRCP15_Control:          Rx     write of Control register 
//RDCP15_TTBase:           Rx     read of Translation Table Base reg. 
//WRCP15_TTBase:           Rx     write of Translation Table Base reg.
//RDCP15_DAControl:        Rx     read of Domain Access Control reg.
//WRCP15_DAControl:        Rx     write of Domain Access Control reg. 
//RDCP15_FaultStatus:      Rx     read of Fault Status register
//WRCP15_FaultStatus:      Rx     write of Fault Status register
//RDCP15_FaultAddress:     Rx     read Fault Address register
//WRCP15_FaultAddress:     Rx     write of Fault Address register
//WRCP15_FlushIC_DC:       Rx     cache control   Flush ICache + DCache
////:                                              Rx redundant but rqud for MACRO
//WRCP15_FlushIC:          Rx     cache control   Flush ICache
////:                                              Rx redundant but rqud for MACRO
//WRCP15_FlushDC:          Rx     cache control   Flush DCache
////:                                              Rx redundant but rqud for MACRO
//WRCP15_CacheFlushDentry: Rx     cache control   Flush DCache entry
////:                                              Rx source for VA 
//WRCP15_CleanDCentry:     Rx     cache control   Clean DCache entry
////:                                              Rx source for VA 
//WRCP15_Clean_FlushDCentry: Rx   cache control   Clean + Flush DCache entry
////:                                              Rx source for VA 
//WRCP15_DrainWriteBuffer: Rx     Drain Write Buffer
////:                                              Rx redundant but rqud for MACRO
//WRCP15_FlushITB_DTB:     Rx     TLB control     Flush ITB's + DTB's
////:                                              Rx redundant but rqud for MACRO
//WRCP15_FlushITB:         Rx     TLB control     Flush ITB's
////:                                              Rx redundant but rqud for MACRO
//WRCP15_FlushDTB:         Rx     TLB control     Flush DTB's
////:                                              Rx redundant but rqud for MACRO
//WRCP15_FlushDTBentry:    Rx     TLB control     Flush DTB entry
////:                                              Rx source for VA 
//WRCP15_EnableClockSW:    Rx     test/clock/idle control -Enable Clock Switching
////:                                              Rx redundant but rqud for MACRO
//WRCP15_DisableClockSW:   Rx     test/clock/idle control -Disable Clock Switching
////:                                              Rx redundant but rqud for MACRO
//WRCP15_DisablenMCLK:     Rx     test/clock/idle control -Disable nMCLK output
////:                                              Rx redundant but rqud for MACRO
//WRCP15_WaitInt:          Rx     test/clock/idle control -Wait for Interrupt
////:                                              Rx redundant but rqud for MACRO
        
//Coprocessor: read of ID register 
//:
    .macro RDCP15_ID $reg_number
    MRC p15, 0, \$reg_number, c0, c0 ,0
    .endm

//Coprocessor: read of Control register 
//:
    .macro RDCP15_Control $reg_number
    MRC p15, 0, \$reg_number, c1, c0 ,0

    .endm

//Coprocessor: write of Control register 
//:
    .macro WRCP15_Control $reg_number
    MCR p15, 0, \$reg_number, c1, c0 ,0

    .endm

//Coprocessor: read of Translation Table Base reg. 
//:
    .macro RDCP15_TTBase $reg_number
    MRC p15, 0, \$reg_number, c2, c0 ,0

    .endm

//Coprocessor: write of Translation Table Base reg. 
//:
    .macro WRCP15_TTBase $reg_number
    MCR p15, 0, \$reg_number , c2, c0 ,0

    .endm

//Coprocessor: read of Domain Access Control reg. 
//:
    .macro RDCP15_DAControl $reg_number
    MRC p15, 0, \$reg_number, c3, c0 ,0

    .endm

//Coprocessor: write of Domain Access Control reg. 
//:
    .macro WRCP15_DAControl $reg_number
    MCR p15, 0, \$reg_number, c3, c0 ,0

    .endm

//Coprocessor: read of Fault Status register 
//:
    .macro RDCP15_FaultStatus $reg_number
    MRC p15, 0, \$reg_number, c5, c0 ,0

    .endm

//Coprocessor: write of Fault Status register 
//:
    .macro WRCP15_FaultStatus $reg_number
    MCR p15, 0, \$reg_number, c5, c0 ,0

    .endm

//Coprocessor: read of Fault Address register 
//:
    .macro RDCP15_FaultAddress $reg_number
    MRC p15, 0, \$reg_number, c6, c0 ,0

    .endm

//Coprocessor: write of Fault Address register 
//:
    .macro WRCP15_FaultAddress $reg_number
    MCR p15, 0, \$reg_number, c6, c0 ,0

    .endm

//Coprocessor: cache control 
//Flush: ICache + DCache
//:
    .macro WRCP15_FlushIC_DC $reg_number
    MCR p15, 0, \$reg_number, c7, c7 ,0

    .endm

//Coprocessor: cache control 
//Flush: ICache
//:
    .macro WRCP15_FlushIC $reg_number
    MCR p15, 0, \$reg_number, c7, c5 ,0

    .endm

//Coprocessor: cache control 
//Flush: DCache
//:
    .macro WRCP15_FlushDC $reg_number
    MCR p15, 0, \$reg_number, c7, c6 ,0

    .endm

//Coprocessor: cache control 
//Flush: DCache entry
//:
    .macro WRCP15_CacheFlushDentry $reg_number
    MCR p15, 0, \$reg_number, c7, c6 ,1

    .endm

//Coprocessor: cache control 
//Clean: DCache entry
//:
    .macro WRCP15_CleanDCentry $reg_number
    MCR p15, 0, \$reg_number, c7, c10 ,1

    .endm

//Coprocessor: cache control 
//Clean: + Flush DCache entry
//:
    .macro WRCP15_Clean_FlushDCentry $reg_number
    MCR p15, 0, \$reg_number, c7, c14 ,1

    .endm

//Coprocessor: Drain Write Buffer 
//:
    .macro WRCP15_DrainWriteBuffer $reg_number
    MCR p15, 0, \$reg_number, c7, c10 ,4

    .endm


//Coprocessor: TLB control 
//Flush: ITB + DTB
//:
    .macro WRCP15_FlushITB_DTB $reg_number
    MCR p15, 0, \$reg_number, c8, c7 ,0

    .endm

//Coprocessor: TLB control 
//Flush: ITB
//:
    .macro WRCP15_FlushITB $reg_number
    MCR p15, 0, \$reg_number, c8, c5 ,0

    .endm

//Coprocessor: TLB control 
//Flush: DTB
//:
    .macro WRCP15_FlushDTB $reg_number
    MCR p15, 0, \$reg_number, c8, c6 ,0

    .endm

//Coprocessor: TLB control 
//Flush: DTB entry

    .macro WRCP15_FlushDTBentry $reg_number
    MCR p15, 0, \$reg_number, c8, c6 ,1

    .endm

//Coprocessor: test/clock/idle control 
//Enable: Clock Switching
//:
    .macro WRCP15_EnableClockSW $reg_number
    MCR p15, 0, \$reg_number, c15, c1 ,2

    .endm

//Coprocessor: test/clock/idle control 
//Disable: Clock Switching
//:
    .macro WRCP15_DisableClockSW $reg_number
    MCR p15, 0, \$reg_number, c15, c2 ,2

    .endm

//Coprocessor: test/clock/idle control 
//Disable: nMCLK output
//:
    .macro WRCP15_DisablenMCLK $reg_number
    MCR p15, 0, \$reg_number, c15, c4 ,2

    .endm

//Coprocessor: test/clock/idle control 
//Wait: for Interrupt
//:
    .macro WRCP15_WaitInt $reg_number
    MCR p15, 0, \$reg_number, c15, c8 ,2

    .endm
    
