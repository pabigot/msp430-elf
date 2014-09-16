# msp430-elf sources archive

This repository contains the public releases of the MSP430 GCC toolchain
developed by Red Hat and published by Texas Instruments.  The original
releases may be found on [TI's web site][msp430-elf].

Two branches are present in this repository:

 * ``sources`` contains the public releases of the aggregate toolchain
   sources, in the format supplied by Red Hat which includes the
   complete sources to the various tools.

 * ``gcc_rh`` contains the public releases of the MSP430 headers and
   linker scripts supplied by Texas Instruments.

All material in this repository is believed to be open source, licensed
under terms described within each release.  The debug stack is not
archived in this repository as it contains proprietary material and is
not available from the [TI MSPGCC site][msp430-elf].

Each commit message describes the location from which the corresponding
file was downloaded, along with MD5 and SHA256 checksums of the released
archive.  Note that the recorded URL is unlikely to be a permanent link;
however, the original files should be obtainable from TI's web site
under the "Older Releases" section reachable from the "Get Software"
link.

Note that msp430-elf is not [mspgcc]: it is neither binary- nor
source-compatible with existing applications developed for mspgcc.

From [this message](http://www.mail-archive.com/mspgcc-users@lists.sourceforge.net/msg12169.html):

    The versioning of our package works as follows:

    1. Digit: GCC Version
    2. Digit: Debug stack version
    3. Digit: Header/linker file version
    4. Digit: Build number

The material in this repository is current to release: 2.01.01.00

The sources version matches: 2.00.00.00

[msp430-elf]: http://www.ti.com/tool/msp430-gcc-opensource "TI MSP430 GCC"
[mspgcc]: http://sourceforge.net/projects/mspgcc/ "MSPGCC"
