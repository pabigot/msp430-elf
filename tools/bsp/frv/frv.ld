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
    *(.rodata.*)
    *(.gnu.linkonce.r*)
    *(.rodata1)
    *(.rofixup)
    *(.glue_7t)
    *(.glue_7)
    *(.rdata)

    KEEP (*(.init))
    KEEP (*(.fini))

    _etext = .;
    PROVIDE (etext = .);

    *(.sdata2)
    *(.sbss2)

    . = ALIGN(8);

    rom_data_start = .;
    PROVIDE(_rom_data_start = rom_data_start);
    PROVIDE(__rom_data_start = rom_data_start);
  }

  .data ALIGN(8) : AT( rom_data_start )
  {
    PROVIDE(_ram_data_start = .);
    PROVIDE(__ram_data_start = .);

    *(.ram_vec)      /* RAM vectors */
    *(.ram_vectors)  /* RAM vectors */
    *(.data)
    *(.gnu.linkonce.d*)
    *(.fixup)
    *(.got1)
    *(.got2)
    /*
     * For object file formats which do not support
     * arbitrary sections, such as ECOFF and XCOFF (and a.out I suspect)
     */
    SORT(CONSTRUCTORS)

    *(.eh_frame)
    *(.gcc_except_table)

    /*
     * For ELF object file format
     */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *frvend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))

    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *frvend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))

    /*
     * Global pointer stuff
     */
    . = ALIGN(8);
    _gp = . + 2048;
    __global = _gp;
    *(.got.plt)
    *(.got)    
    *(.dynamic)
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)

    . = ALIGN(8);

    PROVIDE(_ram_data_end = .);
    PROVIDE(__ram_data_end = .);
  }

  .bss               :
  {
    PROVIDE(_bss_start = .);
    PROVIDE(__bss_start = .);

    *(.sbss)
    *(.sbss.*)
    *(.scommon)
    *(.bss)
    *(.bss.*)
    *(COMMON)

    . = ALIGN(8);
    PROVIDE(_bss_end = .);
    PROVIDE(__bss_end = .);
    PROVIDE(_end = .);
    PROVIDE(__end = .);
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
