/* libguestfs - the guestfsd daemon
 * Copyright (C) 2009 Red Hat Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/guestfs_protocol.h"
#include "daemon.h"
#include "actions.h"

char **
do_initrd_list (char *path)
{
  FILE *fp;
  char *cmd;
  char filename[PATH_MAX];
  char **filenames = NULL;
  int size = 0, alloc = 0;
  size_t len;

  NEED_ROOT (NULL);
  ABS_PATH (path, return NULL);

  /* "zcat /sysroot/<path> | cpio --quiet -it", but path must be quoted. */
  if (asprintf_nowarn (&cmd, "zcat %R | cpio --quiet -it", path) == -1) {
    reply_with_perror ("asprintf");
    return NULL;
  }

  if (verbose)
    fprintf (stderr, "%s\n", cmd);

  fp = popen (cmd, "r");
  if (fp == NULL) {
    reply_with_perror ("popen: %s", cmd);
    free (cmd);
    return NULL;
  }
  free (cmd);

  while (fgets (filename, sizeof filename, fp) != NULL) {
    len = strlen (filename);
    if (len > 0 && filename[len-1] == '\n')
      filename[len-1] = '\0';

    if (add_string (&filenames, &size, &alloc, filename) == -1) {
      pclose (fp);
      return NULL;
    }
  }

  if (add_string (&filenames, &size, &alloc, NULL) == -1) {
    pclose (fp);
    return NULL;
  }

  if (pclose (fp) != 0) {
    reply_with_perror ("pclose");
    free_strings (filenames);
    return NULL;
  }

  return filenames;
}
