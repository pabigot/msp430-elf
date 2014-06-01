/*
 * sa1100as.c -- Support for
 *               Intel(R) SA-1110 Assabet Board
 *
 * Copyright (c) 1998, 1999, 2000 Cygnus Support
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
 * StrongARM is a Registered Trademark of Advanced RISC Machines Limited.
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */

#include __BOARD_HEADER__
#include <bsp/defs.h>
#include <bsp_if.h>

/*
 * Array of memory region descriptors. We just list RAM and FLASH.
 */
struct bsp_mem_info _bsp_memory_list[] = 
{
    {(void *)ROM_VIRTUAL_BASE,   (void *)ROM_VIRTUAL_BASE,   0, ROM_TOTAL_SIZE,   BSP_MEM_ROM  },
    {(void *)FLASH_VIRTUAL_BASE, (void *)FLASH_VIRTUAL_BASE, 0, FLASH_TOTAL_SIZE, BSP_MEM_FLASH},
    {(void *)RAM_VIRTUAL_BASE,   (void *)RAM_VIRTUAL_BASE,   0, RAM_TOTAL_SIZE,   BSP_MEM_RAM  },
};

/*
 * Number of memory region descriptors.
 */
int _bsp_num_mem_regions = sizeof(_bsp_memory_list)/sizeof(_bsp_memory_list[0]);

/*
 * Toggle LED for debugging purposes.
 * which = 0 for LED9, 1 for LED8
 */
void flash_led(int n, int which)
{
    unsigned long led = BCR_SETTINGS;
    volatile unsigned long *led_reg = (volatile unsigned long *)0x12000000;
    unsigned long mask;

    if (which == 0)
	mask = 0x2000;  /* red led */
    else
	mask = 0x4000;  /* green led */

    /*
     * Flash one of the discrete LED's
     */
    while (n--) {
	int i;

            /*
             * Turn on the LED
             */
	*led_reg = led & ~mask;

	/*
	 * Delay
	 */
	i = 0x7fffff; while (--i);

	/*
	 * Turn off the LED
	 */
	*led_reg = led | mask;

	/*
	 * Delay
	 */
	i = 0x7fffff; while (--i);
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
    extern void _bsp_init_sa1100_comm(void);
    _bsp_init_sa1100_comm();
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
     _bsp_platform_info.board = "Intel(R) SA-1110 Assabet Board";
}

/*
 * Initialize the mmu and Page Tables
 * The MMU is actually turned on by the caller of this function.
 *
 * Returns: top of remapped memory.
 */
unsigned long *
_bsp_mmu_init(void)
{
    unsigned long ttb_base = ((unsigned long)&page1) + (RAM_ACTUAL_BASE - RAM_VIRTUAL_BASE);
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

    /*               Actual  Virtual  Size   Attributes                                                    Function  */
    /*		     Base     Base     MB      cached?           buffered?        access permissions                 */
    /*             xxx00000  xxx00000                                                                                */
    X_ARM_MMU_SECTION(0x000,  0x500,     1,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* Boot flash ROMspace */
    X_ARM_MMU_SECTION(0x080,  0x080,     4,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* Application flash ROM */
    X_ARM_MMU_SECTION(0x100,  0x100,   128,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SA-1101 Development Board Registers */
    X_ARM_MMU_SECTION(0x180,  0x180,   128,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* Ethernet Adaptor */
    X_ARM_MMU_SECTION(0x184,  0x184,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* XBusReg    */
    X_ARM_MMU_SECTION(0x188,  0x188,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SysRegA    */
    X_ARM_MMU_SECTION(0x18C,  0x18C,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SysRegB    */
    X_ARM_MMU_SECTION(0x190,  0x190,     4,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* CPLD A     */
    X_ARM_MMU_SECTION(0x194,  0x194,     4,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* CPLD B     */
    X_ARM_MMU_SECTION(0x200,  0x200,   512,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* PCMCIA Socket A */
    X_ARM_MMU_SECTION(0x300,  0x300,   512,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* PCMCIA Sockets B */
    X_ARM_MMU_SECTION(0x400,  0x400,   128,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SA1111 Daughter card */
    X_ARM_MMU_SECTION(0x480,  0x480,   128,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* Video Controller Daughter card */
    X_ARM_MMU_SECTION(0x800,  0x800, 0x400,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* StrongARM(R) Registers */
    X_ARM_MMU_SECTION(0xC00,      0,    32,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0 */
    X_ARM_MMU_SECTION(0xC00,  0xC00,    32,  ARM_UNCACHEABLE, ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0 */
    X_ARM_MMU_SECTION(0xE00,  0xE00,   128,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* Zeros (Cache Clean) Bank */

    return (unsigned long *)(RAM_VIRTUAL_BASE + RAM_TOTAL_SIZE);
}


#if 0
{
    /*
     * Find out how big the RAM is
     */
#define MEMSTART 0xC0000000
#define MEMEND   0xE0000000
    register unsigned long *ptr=(unsigned long *)MEMSTART, *end = (unsigned long *)MEMEND;
    register unsigned long data1=0xCAFEBABE, data2=0xDEADBEEF;

    /*
     * Fill all of suspected memory w/ 0xCAFEBABE and 0xDEADBEEF
     */
    while (ptr < end)
    {
        *(ptr+0) = data1;
        *(ptr+1) = data2;
        ptr = (unsigned long *)((unsigned long)ptr + SZ_4K);
    }

    /*
     * Now test all of suspected memory for expected pattern
     */
    ptr = (unsigned long *)MEMSTART;

    while (ptr < end)
    {
        /*
         * Check for 0xCAFEBABE
         */
        if (*(ptr+0) != data1) break;
        *(ptr+0) = 0;

        /*
         * Check for 0xDEADBEEF
         */
        if (*(ptr+1) != data2) break;
        *(ptr+1) = 0;

        ptr = (unsigned long *)((unsigned long)ptr + SZ_4K);
    }
}
{
    maxmem = ptr;
    bsp_printf("maxmem = 0x%08lx\n", ptr);
}
#endif /* 0 */
