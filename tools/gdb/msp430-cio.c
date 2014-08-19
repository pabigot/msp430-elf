/* Target-dependent code for the Texas Instruments MSP430 for GDB, the
   GNU debugger.

   Copyright (C) 2013 Free Software Foundation, Inc.

   Contributed by Red Hat, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "defs.h"
#include "arch-utils.h"
#include "target.h"
#include "value.h"
#include "gdbcore.h"
#include "observer.h"
#include "target.h"
#include "breakpoint.h"
#include "solist.h"
#include "solib.h"
#include "msp430-cio.h"
#include "inferior.h"

#include <time.h>


/* Key for accessing per-inferior cio-related data.  */
static const struct inferior_data *cio_inferior_data_key;

/* Maximum number of file CIO file descriptors.  Two bytes are used
   to store the file descriptors and in many instances, this needs
   to be a signed quantity in order to return -1, so that limits
   the number of descriptors to the value given.  */
#define CIO_FD_MAP_COUNT 128

struct cio_state
{
  /* Map of target to host file descriptors.  */
  int fd_map[CIO_FD_MAP_COUNT];

  /* Buffer address at which to load/store CIO commands, parameters, and
     data.  */
  CORE_ADDR ciobuf_addr;
};

/* Forward declaraations.  */
static void cio_do_cio (void);
static void init_cio_fd_map (struct cio_state *cio_state);


/* Fetch the CIO state associated with the current inferior.  If it
   doesn't exist yet, then create it.  */
static struct cio_state *
get_cio_state (void)
{
  struct inferior *inf = current_inferior ();
  struct cio_state *cio_state = inferior_data (inf, cio_inferior_data_key);

  if (cio_state == NULL)
    {
      cio_state = XZALLOC(struct cio_state);
      set_inferior_data (inf, cio_inferior_data_key, cio_state);
      init_cio_fd_map (cio_state);
    }
  return cio_state;
}

/* Initialize the target to host file descriptor map.  */
static void
init_cio_fd_map (struct cio_state *cio_state)
{
  int i;

  /* Preserve mappings for stdin, stdout, and stderr.  */
  cio_state->fd_map[0] = 0;
  cio_state->fd_map[1] = 1;
  cio_state->fd_map[2] = 2;

  /* All other descriptors are unallocated at the moment.  */
  for (i=3; i < CIO_FD_MAP_COUNT; i++)
    cio_state->fd_map[i] = -1;
}


/* Create the C I/O breakpoint when the `inferior_created' observer
   is invoked.  */

static void
cio_inferior_created (struct target_ops *target, int from_tty)
{
  int status;
  struct minimal_symbol *ciobuf_sym, *cio_bp_sym;
  struct breakpoint *cio_breakpoint;
  struct cio_state *cio_state = get_cio_state ();

  /* Look up the CIO buffer using several different variations.  */
  ciobuf_sym = lookup_minimal_symbol ("_CIOBUF_", NULL, NULL);

  if (ciobuf_sym == NULL)
    ciobuf_sym = lookup_minimal_symbol ("__CIOBUF_", NULL, NULL);

  if (ciobuf_sym == NULL)
    ciobuf_sym = lookup_minimal_symbol ("__CIOBUF__", NULL, NULL);

  /* Look up the symbol at which to place a breakpoint.  */
  cio_bp_sym = lookup_minimal_symbol ("C$$IO$$", NULL, NULL);

  /* Delete CIO breakpoint from earlier runs.  */
  remove_solib_event_breakpoints ();

  /* Can't do CIO if the required symbols do not exist.  */
  if (ciobuf_sym == NULL || cio_bp_sym == NULL)
    return;

  cio_state->ciobuf_addr = SYMBOL_VALUE_ADDRESS (ciobuf_sym);
  cio_breakpoint = create_solib_event_breakpoint
                    (get_current_arch (), SYMBOL_VALUE_ADDRESS (cio_bp_sym));
}

/* Cleanup associated with removing an inferior.  */
static void
cio_inferior_removed (struct inferior *inf)
{
  struct cio_state *cio_state = get_cio_state ();
  int i;

  for (i = 3; i < CIO_FD_MAP_COUNT; i++)
    if (cio_state->fd_map[i] != -1)
      {
	close (cio_state->fd_map[i]);
	cio_state->fd_map[i] = -1;
      }
}

/* CIO command constants.  */
enum
{
  CIO_CMD_OPEN = 0xf0,
  CIO_CMD_CLOSE,
  CIO_CMD_READ,
  CIO_CMD_WRITE,
  CIO_CMD_LSEEK,
  CIO_CMD_UNLINK,
  CIO_CMD_GETENV,
  CIO_CMD_RENAME,
  CIO_CMD_GETTIME,
  CIO_CMD_GETCLK,
  CIO_CMD_SYNC = 0xff
};

/* Given a host file descriptor, do a look up in the map to determine
   the one used by the target.  Create a suitable mapping if one does
   not yet exist.  */

static int
cio_host_to_target_fd (struct cio_state *cio_state, int hfd)
{
  int i;
  int tfd;


  tfd = -1;
  
  for (i = 0; i < CIO_FD_MAP_COUNT; i++)
    {
      if (hfd == cio_state->fd_map[i])
	return i;
      if (cio_state->fd_map[i] == -1 && tfd == -1)
	tfd = i;
    }

  if (tfd != -1)
    cio_state->fd_map[tfd] = hfd;

  return tfd;
}

/* Do the actual work associated with reading a CIO command, its parameters,
   etc, performing the I/O operation, and then writing back the result.  */

static void
cio_do_cio (void)
{
  gdb_byte lcp[11];	/* length, command, params. */
  gdb_byte *data = NULL;
  gdb_byte *in_params, *out_params;
  int len, cmd;
  int err;
  struct cio_state *cio_state = get_cio_state ();

  err = target_read_memory (cio_state->ciobuf_addr, lcp, sizeof(lcp));

  /* FIXME: output one warning if we can't read from the buffer.  */
  if (err)
    return;

  len = extract_unsigned_integer (lcp, 2, BFD_ENDIAN_LITTLE);
  if (len)
    {
      data = xmalloc (len);
      err = target_read_memory (cio_state->ciobuf_addr + sizeof (lcp),
                                data, len);
      if (err)
	goto cleanup;
    }

  cmd = lcp[2];
  in_params = lcp + 3;
  out_params = lcp + 2;

  switch (cmd)
    {
    case CIO_CMD_OPEN:
      {
	int t_flags, h_flags;
	int h_fd, t_fd;

	/* We ignore llv_fd at offset 0 as recommended by SLAU132F, p. 146.  */
	t_flags = extract_unsigned_integer (in_params + 2, 2, BFD_ENDIAN_LITTLE);
	h_flags = 0;
	switch (t_flags & 0xff)
	  {
	  case 0x0000:
	    h_flags = O_RDONLY;
	    break;
	  case 0x0001:
	    h_flags = O_WRONLY;
	    break;
	  case 0x0002:
	    h_flags = O_RDWR;
	    break;
	  }
	if (t_flags & 0x0008)
	  h_flags |= O_APPEND;
	if (t_flags & 0x0200)
	  h_flags |= O_CREAT;
	if (t_flags & 0x0400)
	  h_flags |= O_TRUNC;
	if (t_flags & 0x0800)
	  h_flags |= O_BINARY;

	h_fd = open (data, h_flags);

	if (h_fd == -1)
	  t_fd = -1;
	else
	  t_fd = cio_host_to_target_fd (cio_state, h_fd);

	store_unsigned_integer (out_params, 2, BFD_ENDIAN_LITTLE, t_fd);
      }
      break;

    case CIO_CMD_CLOSE:
      {
	int t_fd, h_fd, rv;

	t_fd = extract_unsigned_integer (in_params + 0, 2, BFD_ENDIAN_LITTLE);
	h_fd = cio_state->fd_map[t_fd];
	if (h_fd == -1)
	  rv = -1;
	else
	  rv = close (h_fd);
	store_unsigned_integer (out_params, 2, BFD_ENDIAN_LITTLE, rv);
      }
      break;

    case CIO_CMD_READ:
      {
	int t_fd, h_fd, rv, count;
	t_fd = extract_unsigned_integer (in_params + 0, 2, BFD_ENDIAN_LITTLE);
	h_fd = cio_state->fd_map[t_fd];
	count = extract_unsigned_integer (in_params + 2, 2, BFD_ENDIAN_LITTLE);

	if (h_fd == -1)
	  rv = -1;
	else
	  rv = read (h_fd, data, count);
	store_unsigned_integer (out_params, 2, BFD_ENDIAN_LITTLE, rv);
      }
      break;

    case CIO_CMD_WRITE:
      {
	int t_fd, h_fd, rv, count;
	t_fd = extract_unsigned_integer (in_params + 0, 2, BFD_ENDIAN_LITTLE);
	h_fd = cio_state->fd_map[t_fd];
	count = extract_unsigned_integer (in_params + 2, 2, BFD_ENDIAN_LITTLE);

	if (h_fd == -1)
	  rv = -1;
	else
	  rv = write (h_fd, data, count);
	store_unsigned_integer (out_params, 2, BFD_ENDIAN_LITTLE, rv);
      }
      break;

    case CIO_CMD_LSEEK:
      {
	int t_fd, h_fd, origin;
	off_t offset, rv;
	t_fd = extract_unsigned_integer (in_params + 0, 2, BFD_ENDIAN_LITTLE);
	h_fd = cio_state->fd_map[t_fd];
	offset = extract_unsigned_integer (in_params + 2, 4, BFD_ENDIAN_LITTLE);
	origin = extract_unsigned_integer (in_params + 6, 2, BFD_ENDIAN_LITTLE);

	if (h_fd == -1)
	  rv = (off_t) -1;
	else
	  rv = lseek (h_fd, offset, origin);
	store_unsigned_integer (out_params, 4, BFD_ENDIAN_LITTLE, rv);
      }
      break;

    case CIO_CMD_UNLINK:
      {
	int rv;
	rv = unlink (data);
	store_unsigned_integer (out_params, 2, BFD_ENDIAN_LITTLE, rv);
      }
      break;

    case CIO_CMD_GETENV:
      {
	char *ev;
	ev = getenv (data);
	if (ev != NULL)
	  {
	    strncpy (data, ev, len);
	    store_unsigned_integer (out_params, 2, BFD_ENDIAN_LITTLE, 0);
	  }
	else
	  {
	    data[0] = '\0';
	    store_signed_integer (out_params, 2, BFD_ENDIAN_LITTLE, -1);
	  }
      }
      break;

    case CIO_CMD_RENAME:
      {
	gdb_byte *old, *new;
	old = data;
	new = data;
	while (*new && new < data + len)
	  new++;
	if (new < data + len)
	  {
	    int rv;

	    /* Advance over NULL at end of "old" string, leaving
	       position at beginning of "new" string.  */
	    new++;
	    rv = rename (old, new);
	    store_signed_integer (out_params, 2, BFD_ENDIAN_LITTLE, rv);
	  }
	else
	  store_signed_integer (out_params, 2, BFD_ENDIAN_LITTLE, -1);

      }
      break;

    case CIO_CMD_GETTIME:
      {
	time_t rv;
	rv = time (NULL);
	store_unsigned_integer (out_params, 4, BFD_ENDIAN_LITTLE, rv);
      }
      break;

    case CIO_CMD_GETCLK:
      /* Not implemented.  */
      store_signed_integer (out_params, 2, BFD_ENDIAN_LITTLE, -1);
      break;

    case CIO_CMD_SYNC:
      /* Not implemented.  */
      store_signed_integer (out_params, 2, BFD_ENDIAN_LITTLE, -1);
      break;

    default:
      /* Unknown command: should we output a warning here?  */
      goto cleanup;
      break;
    }

  err = target_write_memory (cio_state->ciobuf_addr, lcp, sizeof (lcp) - 1);
  if (err)
    goto cleanup;

  err = target_write_memory (cio_state->ciobuf_addr + sizeof (lcp) - 1,
                             data, len);
  if (err)
    goto cleanup;

cleanup:
  if (data != NULL)
    xfree (data);
}

/* The functions below are mostly stubs for filling in various
   shared library operations.  This is _not_ a shared library
   implementation, but a shlib_event breakpoint does precisely
   what we need here

   The only work that actually occurs in in the `current_sos'
   operation.   It will invoke cio_do_cio() under suitable
   conditions.  */

static void
cio_relocate_section_addresses (struct so_list *so,
                                struct target_section *sec)
{
}

static void
cio_free_so (struct so_list *so)
{
}

static void
cio_clear_solib (void)
{
}

static void
cio_solib_create_inferior_hook (int from_tty)
{
  /* We could set the breakpoint here, but the observer
     works fine for doing this as well.  Plus we won't
     have to rewrite anything in the event that a more
     suitable breakpoint mechanism is introduced.  */
}

static void
cio_special_symbol_handling (void)
{
}

static int do_cio_okay;

static struct so_list *
cio_current_sos (void)
{
  /* current_sos() can get invoked without having hit a breakpoint.
     How do we determine when a breakpoint has been hit versus some
     other occasion? 

     Solution:  Use the about_to_proceed and normal_stop observers to
     toggle a flag indicating whether the breakpoint in question is a
     CIO breakpoint.  */

  if (do_cio_okay)
    cio_do_cio ();

  return NULL;
}


static int
cio_open_symbol_file_object (void *from_ttyp)
{
  return 0;
}

static int
cio_in_dynsym_resolve_code (CORE_ADDR pc)
{
  return 0;
}

static bfd *
cio_solib_bfd_open (char *pathname)
{
  return NULL;
}

/* target_resumed observer. */
static void
cio_target_resumed (ptid_t ptid)
{
  do_cio_okay = 1;
}

/* normal_stop observer */
static void
cio_normal_stop (struct bpstats *bs, int print_frame)
{
  do_cio_okay = 0;
}

static struct target_so_ops cio_so_ops;

/* Called from elsewhere to set up the observers and initialize
   put in place the necessary shared library operations.  As
   stated elsewhere, this is not a shared library implemention,
   but it does turn out that shared library breakpoints have
   properties important for being able to implement CIO.  */

void enable_cio (struct gdbarch *gdbarch)
{
  static int observers_initialized = 0;

  if (!observers_initialized)
    {
      observer_attach_inferior_created (cio_inferior_created);
      observer_attach_target_resumed (cio_target_resumed);
      observer_attach_normal_stop (cio_normal_stop);

      observers_initialized = 1;
    }

  set_solib_ops (gdbarch, &cio_so_ops);
}

/* Free data associated with per-inferior CIO state.  */

static void
cio_inferior_data_cleanup (struct inferior *inf, void *data)
{
  struct cio_state *cio_state = data;

  if (cio_state != NULL)
    {
      /* Close any open file descriptors.  */
      cio_inferior_removed (inf);

      xfree (cio_state);
    }
}

/* -Wmissing-prototypes */
extern initialize_file_ftype _initialize_msp430_cio;

void
_initialize_msp430_cio (void)
{
  cio_so_ops.relocate_section_addresses = cio_relocate_section_addresses;
  cio_so_ops.free_so = cio_free_so;
  cio_so_ops.clear_solib = cio_clear_solib;
  cio_so_ops.solib_create_inferior_hook = cio_solib_create_inferior_hook;
  cio_so_ops.special_symbol_handling = cio_special_symbol_handling;
  cio_so_ops.current_sos = cio_current_sos;
  cio_so_ops.open_symbol_file_object = cio_open_symbol_file_object;
  cio_so_ops.in_dynsym_resolve_code = cio_in_dynsym_resolve_code;
  cio_so_ops.bfd_open = cio_solib_bfd_open;

  cio_inferior_data_key
    = register_inferior_data_with_cleanup (NULL, cio_inferior_data_cleanup);
}
