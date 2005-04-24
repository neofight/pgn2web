/*
  pgn2web - Converts PGN files to interactive web pages

  Copyright (C) 2004, 2005 William Hoggarth <email: whoggarth@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "bool.h"

/* typdefs for discrete parameters */
typedef enum { FRAMESET, LINKED, INDIVIDUAL } STRUCTURE;

#ifdef __cplusplus
extern "C" {
#endif

int pgn2web(const char *resource_path, const char *pgn_filename, const char *html_filename,
	    bool credit, const char *pieces, STRUCTURE layout,
	    void (*progress)(float percentage, void *context), void *progress_context);

#ifdef __cplusplus
}
#endif
