/*

Copyright (c) 2013 Red Hat Incorporated.
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met: 

    Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    The name of Red Hat Incorporated may not be used to endorse 
    or promote products derived from this software without specific 
    prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

*/

MEMORY {
	VEC (r)   : ORIGIN = 0x00000, LENGTH = 0x00002
	IVEC (r)  : ORIGIN = 0x00004, LENGTH = 0x0007c
	OPT (r)   : ORIGIN = 0x000c0, LENGTH = 0x00004
	ROM (r)   : ORIGIN = 0x000ce, LENGTH = 0x400 - 0xce
	RAM (w)   : ORIGIN = 0xffe60, LENGTH = 0xffec8 - 0xffe60
}

PROVIDE(__stacklen = 10);

SIM__family = 0x00313032; /* "102" */
SIM__romsize = 2048;
SIM__ramstart = 0xffe00;
SIM__mirrorromstart = 0;
SIM__mirrorramstart = 0;
SIM__mirrorsize = 0;

INCLUDE g12-common.ld
