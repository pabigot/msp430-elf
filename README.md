# msp430-elf sources archive

Origin: [https://github.com/pabigot/msp430-elf](https://github.com/pabigot/msp430-elf)

This repository contains the public releases of the MSP430 GCC toolchain
developed by Red Hat and published by Texas Instruments.  The original
releases may be found on [TI's web site][msp430-elf].

Two branches are present in this repository:

 * [``sources``](https://github.com/pabigot/msp430-elf/tree/sources)
   contains the public releases of the aggregate toolchain sources, in
   the format supplied by Red Hat which includes the complete sources to
   the various tools.

 * [``gcc_rh``](https://github.com/pabigot/msp430-elf/tree/gcc_rh)
   contains the public releases of the MSP430 headers and linker scripts
   supplied by Texas Instruments.

All material in this repository is believed to be open source, licensed
under terms described within each release.  The [MSP Debug
Stack](http://processors.wiki.ti.com/index.php/MSP_Debug_Stack) is not
archived in this repository since [it contains proprietary
material](http://forum.43oh.com/topic/5364-msp-debug-stack-no-longer-open-source/).

Each commit message describes the location from which the corresponding
file was downloaded, along with MD5 and SHA256 checksums of the released
archive.  Note that the recorded URL is unlikely to be a permanent link;
however, the original files should be obtainable from TI's web site
under the "Older Releases" section reachable from the "Get Software"
link.

**Note**: msp430-elf is not [mspgcc]: it is neither binary- nor
source-compatible with existing applications developed for mspgcc.

**Note**: Although I (pabigot) maintained mspgcc for several years, I am
not involved in maintaining or evolving msp430-elf nor do I provide
support for it.  For visibility,
if toolchain problems are reproducible on upstream builds please file
them first at the appropriate Bugzilla:
[GCC](https://gcc.gnu.org/bugzilla) or
[Binutils/newlib](https://sourceware.org/bugzilla/).  Per [this
message](http://e2e.ti.com/support/development_tools/compiler/f/343/t/384148.aspx)
bugs should also be reported on the [E2E TI C/C++ Compiler
forum](http://e2e.ti.com/support/development_tools/compiler/default.aspx),
though the people there do not maintain the headers or linker scripts.
For TI it is sufficient to reference the upstream bugzilla report.  See
also the [TI MSP430 GCC homepage][msp430-elf].  

From [this message](http://www.mail-archive.com/mspgcc-users@lists.sourceforge.net/msg12169.html):

    The versioning of our package works as follows:

    1. Digit: GCC Version
    2. Digit: Debug stack version
    3. Digit: Header/linker file version
    4. Digit: Build number

The material in this repository is current to release: 3.02.03.00

The sources version matches: 3.02.03.00

[msp430-elf]: http://www.ti.com/tool/msp430-gcc-opensource "TI MSP430 GCC"
[mspgcc]: http://sourceforge.net/projects/mspgcc/ "MSPGCC"
