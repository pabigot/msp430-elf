SECTIONS
{
  .text :
  {
    *(.rom_vec)      /* ROM vectors */
    *(.rom_vectors)  /* ROM vectors */
    CREATE_OBJECT_SYMBOLS
    *(.bsp_init)    /* board-specific init code */
    *(.text)
    *(.gnu.linkonce.t*)
    *(.mips16.fn.*)
    *(.mips16.call.*)

    *(.rodata)
    *(.glue_7t)
    *(.glue_7)
    *(.rdata)

    /*
     * For object file formats which do not support
     * arbitrary sections, such as ECOFF and XCOFF (and a.out I suspect)
     */
    SORT(CONSTRUCTORS)

    /*
     * For ELF object file format
     */
    ___CTOR_LIST__ = .;
    LONG((___CTOR_END__ - ___CTOR_LIST__) / 4 - 2)
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    LONG(0)
    ___CTOR_END__ = .;

    ___DTOR_LIST__ = .;
    LONG((___DTOR_END__ - ___DTOR_LIST__) / 4 - 2)
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    LONG(0)
    ___DTOR_END__ = .;

    . = ALIGN(8);

    KEEP (*(.init))
    KEEP (*(.fini))

    *(.gcc_except_table)

    _etext = .;
    PROVIDE (etext = .);

    *(.sdata2)
    *(.sbss2)
  }

  .rom_ptab ALIGN(0x4000) :
  {
    *(.rom_sec*)
    . = ALIGN(0x400);
    *(.rom_smp*)
  }
  . = ALIGN(8);
  rom_data_start = .;
  PROVIDE(_rom_data_start = rom_data_start);
  PROVIDE(__rom_data_start = rom_data_start);

  .data ALIGN(8) : AT( rom_data_start )
  {
    PROVIDE(_ram_data_start = .);
    PROVIDE(__ram_data_start = .);

    *(.ram_vec)      /* RAM vectors */
    *(.ram_vectors)  /* RAM vectors */
    *(.data)
    *(.fixup)
    *(.got1)
    *(.got2)
    *(.got)

    *(.eh_frame)

    /*
     * Global pointer for MIPS stuff
     */
    . = ALIGN(8);
    _gp = . + 0x8000;
    __global = _gp;
    *(.lit8)
    *(.lit4)
    *(.sdata)

    . = ALIGN(8);

    PROVIDE(_ram_data_end = .);
    PROVIDE(__ram_data_end = .);
  }

  .bss               :
  {
    PROVIDE(_bss_start = .);
    PROVIDE(__bss_start = .);

    *(.sbss)
    *(.scommon)
    *(.bss)
    *(COMMON)

    . = ALIGN(8);
    PROVIDE(_bss_end = .);
    PROVIDE(__bss_end = .);
    PROVIDE(end = .);
    PROVIDE(_end = .);
    PROVIDE(__end = .);
  }

  /* 
   * Make sure we are on a 16K boundary
   */
  .arm_page_table ALIGN(0x8000):
  {
    *(.arm_page_ta*)
  }

  /*
   * Setup the Reset Vector for the FR30 port.
   */
  .fr30_reset_vector 0x000ffffc:
  {
	*(.fr30_reset_vector)
  }

  .ppc_init_403 0xfffffe00:
  {
    *(.init_403)
  }

  .ppc_reset_403 0xfffffffc:
  {
    *(.reset_403)
  }

  /*
   * Stabs debugging sections.
   */
  .stab 0            : { *(.stab)		}
  .stabstr 0         : { *(.stabstr)		}
  .stab.excl 0       : { *(.stab.excl)		}
  .stab.exclstr 0    : { *(.stab.exclstr)	}
  .stab.index 0      : { *(.stab.index)		}
  .stab.indexstr 0   : { *(.stab.indexstr)	}
  .comment 0         : { *(.comment)		}

  /*
   * DWARF debug sections.
   * Symbols in the DWARF debugging sections are relative to 
   * the beginning of the section so we begin them at 0.
   */

  /* DWARF 1 */
  .debug          0  : { *(.debug)		}
  .line           0  : { *(.line)		}

  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0  : { *(.debug_srcinfo)	}
  .debug_sfnames  0  : { *(.debug_sfnames)	}

  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0  : { *(.debug_aranges)	}
  .debug_pubnames 0  : { *(.debug_pubnames)	}

  /* DWARF 2 */
  .debug_info     0  : { *(.debug_info)		}
  .debug_abbrev   0  : { *(.debug_abbrev)	}
  .debug_line     0  : { *(.debug_line)		}
  .debug_frame    0  : { *(.debug_frame)	}
  .debug_str      0  : { *(.debug_str)		}
  .debug_loc      0  : { *(.debug_loc)		}
  .debug_macinfo  0  : { *(.debug_macinfo)	}

  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames)	}
  .debug_funcnames 0 : { *(.debug_funcnames)	}
  .debug_typenames 0 : { *(.debug_typenames)	}
  .debug_varnames  0 : { *(.debug_varnames)	}
}
