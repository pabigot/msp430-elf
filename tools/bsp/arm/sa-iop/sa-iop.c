/*
 * sa-iop.c -- Support for Intel(R) SA-IOP Evaluation Board
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
 *
 * Intel is a Registered Trademark of Intel Corporation.
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */

#include __BOARD_HEADER__
#include <bsp_if.h>

/*
 * Array of memory region descriptors. We just list RAM and FLASH.
 */
struct bsp_mem_info _bsp_memory_list[] = 
{
    { (void *)0x00000000, (void *)0x00000000, 0, 0, BSP_MEM_RAM }
};

/*
 * Number of memory region descriptors.
 */
int _bsp_num_mem_regions = sizeof(_bsp_memory_list)/sizeof(_bsp_memory_list[0]);

/*
 * Toggle LED for debugging purposes.
 */
void flash_led(int n, int which)
{
    int led;

    if ((which < 0) || (which > 6))
        return;

    /*
     * Select the right LED
     */
    led = SA_IOP_SOFT_IO_LED(which);

    while (n--)
    {
        int i;

        /*
         * Turn on the LED
         */
        *SA_IOP_SOFT_IO_REGISTER |= led;

        i = 0xffff; while (--i);

        /*
         * Turn of the LED
         */
        *SA_IOP_SOFT_IO_REGISTER &= ~led;

        i = 0xffff; while (--i);
    }
}

/*
 * Early initialization of comm channels. Must not rely
 * on interrupts, yet. Interrupt operation can be enabled
 * in _bsp_board_init().
 */
void
_bsp_init_board_comm(void)
{
    extern void _bsp_init_sa110_comm(void);
    _bsp_init_sa110_comm();
}


/*
 * Set any board specific debug traps.
 */
void
_bsp_install_board_debug_traps(void)
{
}


/*
 * Install any board specific interrupt controllers.
 */
void
_bsp_install_board_irq_controllers(void)
{
}

/*
 *  Board specific BSP initialization.
 */
void
_bsp_board_init(void)
{
    /*
     * Override default platform info.
     */
    _bsp_platform_info.board = "Intel(R) SA-IOP Evaluation Board";

    /*
     * Finish setup of RAM description. Early initialization put a
     * pointer to the top of RAM in _bsp_ram_info_ptr.
     */
    _bsp_memory_list[0].nbytes = (long)_bsp_ram_info_ptr;
}

/*
 * Initialize the mmu and Page Tables
 * The MMU is actually turned on by the caller of this function.
 */
void
_bsp_mmu_init(int sdram_size)
{
    unsigned long ttb_base = (unsigned long)&page1;
    unsigned long i;

    BSP_ASSERT((ttb_base & ARM_TRANSLATION_TABLE_MASK) == ttb_base);

    /*
     * Set the TTB register
     */
    __mcr(ARM_CACHE_COPROCESSOR_NUM,
          ARM_COPROCESSOR_OPCODE_DONT_CARE,
          ttb_base,
          ARM_TRANSLATION_TABLE_BASE_REGISTER,
          ARM_COPROCESSOR_RM_DONT_CARE,
          ARM_COPROCESSOR_OPCODE_DONT_CARE);

    /*
     * Set the Domain Access Control Register
     */
    i = ARM_ACCESS_TYPE_MANAGER(0)    | 
        ARM_ACCESS_TYPE_NO_ACCESS(1)  |
        ARM_ACCESS_TYPE_NO_ACCESS(2)  |
        ARM_ACCESS_TYPE_NO_ACCESS(3)  |
        ARM_ACCESS_TYPE_NO_ACCESS(4)  |
        ARM_ACCESS_TYPE_NO_ACCESS(5)  |
        ARM_ACCESS_TYPE_NO_ACCESS(6)  |
        ARM_ACCESS_TYPE_NO_ACCESS(7)  |
        ARM_ACCESS_TYPE_NO_ACCESS(8)  |
        ARM_ACCESS_TYPE_NO_ACCESS(9)  |
        ARM_ACCESS_TYPE_NO_ACCESS(10) |
        ARM_ACCESS_TYPE_NO_ACCESS(11) |
        ARM_ACCESS_TYPE_NO_ACCESS(12) |
        ARM_ACCESS_TYPE_NO_ACCESS(13) |
        ARM_ACCESS_TYPE_NO_ACCESS(14) |
        ARM_ACCESS_TYPE_NO_ACCESS(15);
    __mcr(ARM_CACHE_COPROCESSOR_NUM,
          ARM_COPROCESSOR_OPCODE_DONT_CARE,
          i,
          ARM_DOMAIN_ACCESS_CONTROL_REGISTER,
          ARM_COPROCESSOR_RM_DONT_CARE,
          ARM_COPROCESSOR_OPCODE_DONT_CARE);

    /*
     * First clear all TT entries - ie Set them to Faulting
     */
    memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);

    /*
     * We only do direct mapping for the IOP board. That is, all
     * virt_addr == phys_addr.
     */

    /*               Actual  Virtual  Size   Attributes                                                    Function  */
    /*		     Base     Base     MB      cached?           buffered?        access permissions                 */
    /*             xxx00000  xxx00000                                                                                */
    X_ARM_MMU_SECTION(0x000,  0x000,  (sdram_size>>20),  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* SDRAM */
    X_ARM_MMU_SECTION(0x400,  0x400,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* 21285 regs */
    X_ARM_MMU_SECTION(0x410,  0x410,     4,  ARM_CACHEABLE,   ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* FLASH ROM  */
    X_ARM_MMU_SECTION(0x420,  0x420,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* 21285 CSR Space */
    X_ARM_MMU_SECTION(0x500,  0x500,    16,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* Zero Block */
    X_ARM_MMU_SECTION(0x780,  0x780,    16,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* Outbound Write Flush */
    X_ARM_MMU_SECTION(0x790,  0x790,    65,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* PCI IACK/Config/IO */
    X_ARM_MMU_SECTION(0x800,  0x800,  2048,  ARM_UNCACHEABLE, ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* PCI Memory */
}
