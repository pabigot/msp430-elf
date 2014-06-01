/* 
 * Copyright (C) 1995, 2000 Advanced RISC Machines Limited. All rights reserved.
 * 
 * This software may be freely used, copied, modified, and distributed
 * provided that the above copyright notice is preserved in all copies of the
 * software.
 */

/* -*-C-*-
 *
 * $Revision: 1.1 $
 *     $Date: 2000/02/25 13:52:32 $
 *
 *
 * drivers.c - declares a NULL terminated list of device driver
 *             descriptors supported by the host.
 */
#include <stdio.h>

#include "drivers.h"

extern DeviceDescr angel_SerialDevice;
extern DeviceDescr angel_SerparDevice;
extern DeviceDescr angel_EthernetDevice;

DeviceDescr *devices[] =
{
    &angel_SerialDevice,
#if 0
    &angel_SerparDevice,
    &angel_EthernetDevice,
#endif
    NULL
};

/* EOF drivers.c */
