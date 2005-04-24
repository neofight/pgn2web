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

#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include "bool.h"
#include "pgn2web.h"

/* default installation path */
#ifndef INSTALL_PATH
#define INSTALL_PATH "/usr/local/pgn2web/"
#endif

char usage[] = "usage: pgn2web\n"
               "       pgn2web [-c yes|no] [-p <pieces>] [-s frameset|linked|individual] pgn-filename html-filename\n";

/* main function */
int main(int argc, char *argv[])
{
  bool valid = true;
  bool in_options = true;
  int arg;
  bool credit_set = false;
  bool layout_set = false;
  int pgn_filename = 0;
  int html_filename = 0;
  int pieces = 0;

  char *path;
  struct stat stat_buf;

  /* default options */
  bool credit = true;
  STRUCTURE layout = FRAMESET;

  /* if no arguments provided then launch gui */
  if(argc == 1) {
    execvp("p2wgui", 0);
  }

  /* parse the arguments */
  arg = 1;
  while(arg < argc) {
    
    /* check for option and process it, else assume filenames */
    if(in_options && argv[arg][0] == '-') {

      if(!credit_set && !strcmp("-c", argv[arg])) {
	
	if(!strcmp("yes", argv[arg + 1])) {
	  credit = true;
	  credit_set = true;
	  arg +=2;
	  continue;
	}

	if(!strcmp("no", argv[arg + 1])) {
	  credit = false;
	  credit_set = true;
	  arg += 2;
	  continue;
	}

	valid = false;
	break;
      }

      if(!pieces && !strcmp("-p", argv[arg])) {

	/* check the piece set is valid */
	path = (char*)calloc(strlen(INSTALL_PATH) + strlen("/images/") + strlen(argv[arg + 1]) + 1, sizeof(char));
	strcpy(path, INSTALL_PATH);
	strcat(path, "/images/");
	strcat(path, argv[arg + 1]);

	if(!stat(path, &stat_buf)) {
	  free((void*)path);

	  pieces = arg + 1;
	  arg += 2;
	  continue;
	}
	else {
	  free((void*)path);
	  valid = false;
	  break;
	}
      }
	
      if(!layout_set && !strcmp("-s", argv[arg])) {

	if(!strcmp("frameset", argv[arg + 1])) {
	  layout_set = true;
	  layout = FRAMESET;
	  arg += 2;
	  continue;
	} 

	if(!strcmp("linked", argv[arg + 1])) {
	  layout_set = true;
	  layout = LINKED;
	  arg += 2;
	  continue;
	}

	if(!strcmp("individual", argv[arg + 1])) {
	  layout_set = true;
	  layout = INDIVIDUAL;
	  arg += 2;
	  continue;
	}

	valid = false;
	break;
      }

      /* invalid option as there is no match */
      valid = false;
      break;
    } 
    else {

      /* we now expect filename and not options */
      in_options = false;

      /* test for an option in the wrong place */
      if(argv[arg][0] == '-') {
	valid = false;
	break;
      }
      
      /* record which argument holds the filename */
      if(pgn_filename == 0) {
	pgn_filename = arg;
      }
      else if (html_filename == 0) {
	html_filename = arg;
      }
      else {
	/* we already have both filenames, so there must be an error */
	valid = false;
	break;
      }
    }

    /* move on to the next argument */
    arg++;
  }

  /* make sure that we have both filenames */
  if(!pgn_filename || !html_filename) {
    valid = false;
  }

  /* either execute or print error message */
  if(valid) {
    return pgn2web("/usr/local/pgn2web/", argv[pgn_filename], argv[html_filename], credit,
		   pieces ? argv[pieces] : "merida", layout, NULL, NULL);
  }
  else {
    printf(usage);
    return 1;
  }
}
