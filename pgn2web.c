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

#include "pgn2web.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "chess.h"
#include "nag.h"

/* default installation path */
#ifndef INSTALL_PATH
#ifdef WINDOWS
#define INSTALL_PATH "C:\\Progra~1\\pgn2web\\"
#else
#define INSTALL_PATH "/usr/local/pgn2web/"
#endif
#endif

/* define constant for system dependent file seperator */
#ifdef WINDOWS
const char SEPERATOR = '\\';
#define SEPERATOR_STRING "\\"
#else
const char SEPERATOR = '/';
#define SEPERATOR_STRING "/"
#endif

/* variation type definition */
typedef struct variation {
  struct variation *parent;
  struct variation *siblings;
  struct variation *children;

  int parent_move;
  int actual_move;
  int relative_move;
  POSITION position;
  POSITION previous_position;

  int id;
  char *buffer;
  long int buffer_size;
} VARIATION;

/* constants */
const char *piece_filenames[] = {"", "wp", "wn", "wb", "wr", "wq", "wk", "bp", "bn", "bb", "br", "bq", "bk"};
const char *credit_html = "This page was created with <a href=\"http://pgn2web.sourceforge.net\" target=\"_top\">pgn2web</a>.";

#ifdef DEBUG
const char *board_template = "templates" SEPERATOR_STRING "board.html";
const char *frame_template = "templates" SEPERATOR_STRING "frame.html";
const char *template_filename = "templates" SEPERATOR_STRING "template.html";
#else
const char *board_template = INSTALL_PATH "templates" SEPERATOR_STRING "board.html";
const char *frame_template = INSTALL_PATH "templates" SEPERATOR_STRING "frame.html";
const char *game_filename = INSTALL_PATH "templates" SEPERATOR_STRING "template.html";
const char *single_filename = INSTALL_PATH "templates" SEPERATOR_STRING "single.html";
#endif

/* function prototypes */
void append_move(char *string, const MOVE *move, const POSITION *position);
void create_board(const char *board_filename, const char *html_filename, const char *pieces, const char *game_list, bool credit);
void create_frame(const char* frame_filename, const char* html_filename); 
void delete_variation(VARIATION *variation, char **moves, long int *moves_size);
MOVE extract_coordinates(const char* algebraic);
void extract_game_list(FILE* file, const char* html_filename, char** game_list); /* !! allocates memory which must be freed by caller !! */
void print_board(FILE* html, const char* FEN);
void print_initial_position(FILE* file, const char* FEN, const char* var);
void process_game(FILE *pgn, FILE *template, const char *html_filename, const int game, const char *pieces, const char* game_list, bool credit, STRUCTURE layout);
void process_moves(FILE* pgn, const char* FEN, char **moves, char **notation, STRUCTURE layout); /* !! allocates memory which must be freed by caller !! */
void strip(FILE *pgn);


/* main function */

int pgn2web(const char *pgn_filename, const char *html_filename, bool credit, const char *pieces, STRUCTURE layout)
{
  const char *template_filename;
  char *path;
  char *command;
  FILE *pgn, *template;
  char *game_list;
  int game = 0;
  char test;

  /* open files */
  if((pgn = fopen(pgn_filename, "r")) == NULL) {
      exit(1);
  }

  /* select and open the right template file */
  template_filename = (layout == FRAMESET) ? game_filename : single_filename;

  if((template = fopen(template_filename, "r")) == NULL) {
    perror("Unable to open template file");
      exit(1);
  }

  /* allocate required space for path and command strings */
  path = (char*)calloc(strlen(html_filename) + 32, sizeof(char));
  command = (char*)calloc(strlen(html_filename) + strlen(INSTALL_PATH) + 32, sizeof(char));

  /* copy images */
  if(strrchr(html_filename, SEPERATOR) == NULL) {
    path[0] = '.';
    path[1] = SEPERATOR;
    path[2] = '\0';
  }
  else {
    strncpy(path, html_filename, strrchr(html_filename, SEPERATOR) - html_filename + 1);
    path[strrchr(html_filename, SEPERATOR) - html_filename + 1] = '\0';
  }

#ifdef WINDOWS
  strcpy(command, "MD \"");
  strcat(command, path);
  strcat(command, "images\"");
  system(command);

  strcpy(command, "COPY \"" INSTALL_PATH "images" SEPERATOR_STRING);
  strcat(command, pieces);
  strcat(command, "\" \"");
  strcat(command, path);
  strcat(command, "images\"");
  system(command);
#else
  strcpy(command, "cp -r \"" INSTALL_PATH "images" SEPERATOR_STRING);
  strcat(command, pieces);
  strcat(command, "\" \"");
  strcat(command, path);
  strcat(command, "\"");
  system(command);
#endif

  /* extract game list */
  extract_game_list(pgn, html_filename, &game_list); /* !! allocates memory to game_list, free after use !! */
  rewind(pgn);

  /* ensure created files have access of 0644 */
#ifndef WINDOWS
  umask(0133);
#endif

  /* if frameset layout then create board & frameset pages */
  if(layout == FRAMESET) {
    create_board(board_template, html_filename, pieces, game_list, credit);
    create_frame(frame_template, html_filename);
  }

  /* skip any whitespace (or garbage) */
  while((test = getc(pgn)) != '[' && test != EOF) {
  }
  ungetc(test, pgn);

  /* process games */
  while(!feof(pgn)) {
    rewind(template); /* go back to start of template */
  
    /* process game */
    process_game(pgn, template, html_filename, game, pieces, game_list, credit, layout);
    game++;

    /* skip remaining whitespace (and any garbage) */
    while((test = getc(pgn)) != '[' && test != EOF) {
    }
    ungetc(test, pgn);
  }

  /* close files */
  fclose(pgn);
  fclose(template);

  /* free allocated memory */
  free((void*)path);
  free((void*)command);
  free((void*)game_list);

  /* sucess! */
  return 0;
}

/* append to string move as javascript data */
void append_move(char *string, const MOVE *move, const POSITION *position)
{
  /* special moves must be broken down into 2 moves for simple javascript code e.g. castling requires moving two pieces */
  int js_move[4] = {-1, -1, -1, -1};

  js_move[0] = move->from_col + 8 * (7 - move->from_row);
  js_move[1] = move->to_col + 8 * (7 - move->to_row);

  /* check for special pawn moves */
  if(piece_to_piece_type(position->board[move->from_col][move->from_row]) == PAWN) {

    /* check if move is a promotion */
    if(move->promotion_piece) {
      js_move[2] = -(int)piece_type_and_colour_to_piece(move->promotion_piece, position->turn);
    }
    else {

      /* check if move is an en passant capture */
      if(move->to_col == position->ep_col && move->to_row == (position->turn == WHITE ? 5 : 2)) {
	js_move[2] = js_move[0];
	js_move[3] = js_move[1];
	js_move[0] = move->to_col + 8 * (7 - move->from_row);
	js_move[1] = move->to_col + 8 * (7 - move->to_row);
      }
    }
  }
  else {
    
    /* check if move is a castling move */
    if(piece_to_piece_type(position->board[move->from_col][move->from_row]) == KING) {
      
      /* kingside? */
      if(move->to_col - move->from_col == 2) {
	js_move[2] = 7 + 8 * (7 - move->from_row);
	js_move[3] = js_move[2] - 2;
      }

      /* queenside? */
      if(move->from_col - move->to_col == 2) {
	js_move[2] = 0 + 8 * (7 - move->from_row);
	js_move[3] = js_move[2] + 3;
      }
    }
  }

  /* now write javascript move to string */
  sprintf(string + strlen(string), "%d,%d,%d,%d,", js_move[0], js_move[1], js_move[2], js_move[3]);
}

/* creates board child frame from template */
void create_board(const char* template_filename, const char *html_filename, const char* pieces, const char* game_list, bool credit)
{
  char *filename;
  char buffer[256];
  char *tag;
  FILE *template, *board;

  filename = (char*)calloc(strlen(html_filename) + strlen(".board") + 1, sizeof(char));
  
  /* generate filename */
  strcpy(filename, html_filename);

  if(strrchr(filename, '.') == NULL) {
    strcat(filename, ".board");
  }
  else {
    (*strrchr(filename, '.')) = '\0';
    strcat(filename, ".board");
    strcat(filename, strrchr(html_filename, '.'));
  }

  /* open template and output file */
  if((template = fopen(board_template, "r")) == NULL) {
    perror("Unable to open template file");
    exit(1);
  }
  if((board = fopen(filename, "w")) == NULL) {
    perror("Unable to create html file");
    exit(1);
  }

  /* process template file, replacing XML-like tags */
  while(fgets(buffer, 256, template) != NULL) {

    if(!strstr(buffer, "/>")) {
      fprintf(board, "%s", buffer);
      continue;
    }
	  
    if((tag = strstr(buffer, "<credit/>"))) {
      *tag = '\0';
      fprintf(board, "%s", buffer);
      if(credit) {
        fprintf(board, credit_html);
      }
      fprintf(board, "%s", tag + strlen("<credit/>"));
    }

    if((tag = strstr(buffer, "<pieces/>"))) {
      *tag = '\0';
      fprintf(board, "%s", buffer);
      fprintf(board, pieces);
      fprintf(board, "%s", tag + strlen("<pieces/>"));
    }


    if(strstr(buffer, "<gamelist/>")) {
	fprintf(board, "<SELECT name=\"game\" onchange=\"if(this.value != 'null') parent.game.location=this.value;\">\n");
	fprintf(board, "<option value=\"null\">Select a game...\n");
	fprintf(board, "%s\n", game_list);
    }
  }

  /* close files & free memory*/
  free((void*)filename);
  fclose(template);
  fclose(board);
}

void create_frame(const char *frame_filename, const char *html_filename)
{
  char *board_url, *game_url;
  FILE *template, *frame;
  char buffer[256];
  
  /* allocate memory */
  board_url = (char*)calloc(strlen(html_filename) + strlen(".board") + 1, sizeof(char));
  game_url = (char*)calloc(strlen(html_filename) + 32, sizeof(char));
  
  /* generate filenames */
  if(strrchr(html_filename, SEPERATOR) != NULL) {
    strcpy(game_url, strrchr(html_filename, SEPERATOR) + 1);
  }
  else {
    strcpy(game_url, html_filename);
  }

  if(strrchr(game_url, '.') == NULL) {
    strcat(game_url, "0");
  }
  else {
    (*strrchr(game_url, '.')) = '\0';
    strcat(game_url, "0");
    strcat(game_url, strrchr(html_filename, '.'));
  }

  if(strrchr(html_filename, SEPERATOR) != NULL) {
    strcpy(board_url, strrchr(html_filename, SEPERATOR) + 1);
  }
  else {
    strcpy(board_url, html_filename);
  }

  if(strrchr(board_url, '.') == NULL) {
    strcat(board_url, ".board");
  }
  else {
    (*strrchr(board_url, '.')) = '\0';
    strcat(board_url, ".board");
    strcat(board_url, strrchr(html_filename, '.'));
  }

  /* open template and output file */
  if((template = fopen(frame_filename, "r")) == NULL) {
    perror("Unable to open template file");
    exit(1);
  }
  if((frame = fopen(html_filename, "w")) == NULL) {
    perror("Unable to create html file");
    exit(1);
  }

  /* process template file, replacing XML-like tags */
  while(fgets(buffer, 256, template) != NULL) {
    if(strstr(buffer, "/>") == NULL) {
      fprintf(frame, "%s", buffer);
    }
    else {
      if(strstr(buffer, "<board/>")) {
	fprintf(frame, "<frame name=\"board\" src=\"%s\">\n", board_url);
      }
      if(strstr(buffer, "<game/>")) {
	fprintf(frame, "<frame name=\"game\" src=\"%s\">\n", game_url);
      }
    }
  }

  /* close files & free memory*/
  free((void*)board_url);
  free((void*)game_url);
  fclose(template);
  fclose(frame);
}

/* deletes a variation adding its data to the moves string */
void delete_variation(VARIATION *variation, char **moves, long int *moves_size)
{
  /* add parent information to moves */
#ifdef DEBUG
  printf("Adding moves: %d\n", variation->id);
  printf("%s\n", variation->buffer);
#endif

  *moves_size += 64;
  *moves = (char*)realloc((void*)*moves, *moves_size * sizeof(char));
  sprintf(*moves + strlen(*moves), "parents[%d] = new Array(", variation->id);
  if(variation->parent) {
    sprintf(*moves + strlen(*moves), "%d,%d);\n", variation->parent->id, variation->parent_move); 
  }
  else {
    sprintf(*moves + strlen(*moves), "-1,%d);\n", variation->parent_move); 
  }
 
  /* add buffer to moves */
#ifdef DEBUG
  printf("Added moves: %d\n", variation->id);
  printf("%s\n", variation->buffer);
#endif

  *moves_size += strlen(variation->buffer);
  *moves = (char*)realloc((void*)*moves, *moves_size * sizeof(char));
  strcat(*moves, variation->buffer);

  /* if there are children delete these first */
  if(variation->children) {
    delete_variation(variation->children, moves, moves_size);
  }

  /* if there are siblings delete these second */
  if(variation->siblings) {
    delete_variation(variation->siblings, moves, moves_size);
  }

#ifdef DEBUG
  printf("Deleting: %d\n", variation->id);
#endif

  /* finally delete the variation itself */ 
  free((void*)variation->buffer);
  free((void*)variation);

#ifdef DEBUG
  printf("Deleted: %d\n", variation->id);
#endif
}

/* extract any co-ordinates contained in the notation */
MOVE extract_coordinates(const char* algebraic)
{
  MOVE move = {-1, -1, -1, -1};
  int i;

  for(i = strlen(algebraic) - 1; i >= 0; i--) {
    if(islower(algebraic[i]) && algebraic[i] != 'x') {
      if(move.to_col == -1) {
	move.to_col = algebraic[i] - 'a';
      }
      else {
	move.from_col = algebraic[i] - 'a';
      }
    }

    if(isdigit(algebraic[i])) {      
      if(move.to_row == -1) {
	move.to_row = '8' - algebraic[i];
      }
      else {
	move.from_row = '8' - algebraic[i];
      }
    }
  }

  /* check for error by looking for invalid values */
  if(move.to_col < 0 || move.to_col > 7 || move.to_row < 0 || move.to_row > 7) {
    move.from_col = -1;
    move.from_row = -1;
    move.to_col = -1;
    move.to_row = -1;
  }

  return move;
}

/* constructs game list from STRs */
void extract_game_list(FILE* file, const char* html_filename, char **game_list) /* !! allocates memory to game_list, it must be freed by the caller !! */
{
  char buffer[256];
  char white[256];
  char black[256];
  char date[256];

  int game = 0;

  char *url;
  char game_index[32];
  long int buffer_size;

  /* allocate memory */
  url = (char*)calloc(strlen(html_filename) + 32, sizeof(char));
  *game_list = (char*)calloc(4096, sizeof(char)); /* use initial buffer of 4k */
  buffer_size = 4096;

  strcpy(*game_list, "");
  strcpy(white, "");
  strcpy(black, "");
  strcpy(date, "");

  while(fgets(buffer, 256, file) != NULL) {
    sscanf(buffer, "[Date \"%[^\"]\"]", date);
    sscanf(buffer, "[White \"%[^\"]\"]", white);
    sscanf(buffer, "[Black \"%[^\"]\"]", black);

    if(strcmp(white, "") && strcmp(black, "")) {
      /* generate filename */
      if(strrchr(html_filename, SEPERATOR) != NULL) {
	strcpy(url, strrchr(html_filename, SEPERATOR) + 1);
      }
      else {
	strcpy(url, html_filename);
      }

      sprintf(game_index, "%d", game);

      if(strrchr(url, '.') == NULL) {
	strcat(url, game_index);
      }
      else {
	(*strrchr(url, '.')) = '\0';
	strcat(url, game_index);
	strcat(url, strrchr(html_filename, '.'));
      }

#ifdef DEBUG
      printf("(%s) %s - %s %s\n", url, white, black, date);
#endif

      /* don't display date if it is unknown */
      if(!strcmp(date, "????.??.??")) {
	strcpy(date, "");
      }

      /* generate html for option list */
      strcat(*game_list, "<option value=\"");
      strcat(*game_list, url);
      strcat(*game_list, "\">");
      strcat(*game_list, white);
      strcat(*game_list, " - ");
      strcat(*game_list, black);
      strcat(*game_list, " ");
      strcat(*game_list, date);
      strcat(*game_list, "\n");

      strcpy(white, "");
      strcpy(black, "");
      strcpy(date, "");

      game++;

      /* allocate more memory if buffer is running low */
      if(strlen(*game_list) + 1024 + strlen(html_filename) > buffer_size) {
	buffer_size += 4096;
	*game_list = (char*)realloc((void*)*game_list, buffer_size);
      }      
    }
  }

  /* free memory */
  free((void*)url);
}

/* output javascript data for initial position */
void print_initial_position(FILE* file, const char* FEN, const char* var)
{
  POSITION position;
  int col, row;

  /* convert FEN to position */
  setup_board(&position, FEN);

  /* print out position */
  fprintf(file, "var %s = new Array(", var);

  for(row = 7; row >= 0; row--) {
    for(col = 0; col < 8; col++) {
      if(col == 7 && row == 0) {
	fprintf(file, "%d);\n", position.board[col][row]);
      }
      else {
	fprintf(file, "%d,", position.board[col][row]);
      }
    }
  }
}

/* process 1 pgn game */
void process_game(FILE *pgn, FILE *template, const char *html_filename, int game, const char* pieces, const char* game_list, bool credit, STRUCTURE layout)
{
  char *game_filename;
  char game_index[32];
  FILE *html;

  char buffer[256];
  char *tag;
  char event[256];
  char site[256];
  char date[256];
  char round[256];
  char white[256];
  char black[256];
  char result[256];
  char FEN[256];

  char *moves;
  char *notation;

  *event = *site = *date = *round = *white = *black = *result = *FEN = '\0';

  /* allocate memory for filename */
  game_filename = (char*)calloc(strlen(html_filename) + 32, sizeof(char));

  /* open html file */
  strcpy(game_filename, html_filename);
  sprintf(game_index, "%d", game);

  if(strrchr(game_filename, '.') == NULL) {
    strcat(game_filename, game_index);
  }
  else {
    (*strrchr(game_filename, '.')) = '\0';
    strcat(game_filename, game_index);
    strcat(game_filename, strrchr(html_filename, '.'));
  }
  
  if((html = fopen(game_filename, "w")) == NULL) {
    perror("Unable to create html file");
    exit(1);
  }

  /* process STR */
  while(fgets(buffer, 256, pgn) != NULL) {
    sscanf(buffer, "[Event \"%[^\"]\"]", event);
    sscanf(buffer, "[Site \"%[^\"]\"]", site);
    sscanf(buffer, "[Date \"%[^\"]\"]", date);
    sscanf(buffer, "[Round \"%[^\"]\"]", round);
    sscanf(buffer, "[White \"%[^\"]\"]", white);
    sscanf(buffer, "[Black \"%[^\"]\"]", black);
    sscanf(buffer, "[Result \"%[^\"]\"]", result);
    sscanf(buffer, "[FEN \"%[^\"]\"]", FEN);

    if(!strcmp("\n", buffer)) { break; }
    if(!strcmp("\r\n", buffer)) { break; }
  }

  /* decide on start position */
  if(*FEN == '\0') {
    strcpy(FEN, INITIAL_POSITION);
  }

  /* process move text */
  process_moves(pgn, FEN, &moves, &notation, layout); /* !! allocates memory for move and notation, must be freed by caller !! */

  /* process template file, replacing XML-like tags */
  while(fgets(buffer, 256, template) != NULL) {
    if(strstr(buffer, "/>") == NULL) {
      fprintf(html, "%s", buffer);
    }
    else {
      if(strstr(buffer, "<black/>")) {
	fprintf(html, "%s\n", black);
      }
      if(strstr(buffer, "<current/>")) {
	print_initial_position(html, FEN, "board");
      }
      if(strstr(buffer, "<date/>") && strcmp(date, "????.??.??")) {
	fprintf(html, "%s\n", date);
      }
      if(strstr(buffer, "<event/>")  && strcmp(event, "?")) {
	fprintf(html, "%s\n", event);
      }
      if(strstr(buffer, "<gamelist/>")) {
	switch(layout) {
	case FRAMESET:
	  fprintf(html, "<SELECT name=\"game\" onchange=\"if(this.value != 'null') parent.game.location=this.value;\">\n");
	  fprintf(html, "<option value=\"null\">Select a game...\n");
	  fprintf(html, "%s\n", game_list);
	  break;
	case LINKED:
	  fprintf(html, "<SELECT name=\"game\" onchange=\"if(this.value != 'null') location=this.value;\">\n");
          fprintf(html, "<option value=\"null\">Select a game...\n");
          fprintf(html, "%s\n", game_list);
          break;
	case INDIVIDUAL:
	  break;
	}
      }
      if(strstr(buffer, "<initial/>")) {
	print_initial_position(html, FEN, "initial");
      }
      if(strstr(buffer, "<moves/>")) {
	fprintf(html, "%s\n", moves);
      }
      if(strstr(buffer, "<notation/>")) {
	fprintf(html, "%s\n", notation);
      }   
      if(strstr(buffer, "<result/>") && strcmp(result, "?")) {
	fprintf(html, "%s\n", result);
      }
      if(strstr(buffer, "<round/>") && strcmp(round, "?")) {
	fprintf(html, "Round %s\n", round);
      }
      if(strstr(buffer, "<site/>") && strcmp(site, "?")) {
	fprintf(html, "%s\n", site);
      }
      if(strstr(buffer, "<white/>")) {
	fprintf(html, "%s\n", white);
      }
      if((tag = strstr(buffer, "<credit/>"))) {
        *tag = '\0';
        fprintf(html, "%s", buffer);
        if(credit) {
          fprintf(html, credit_html);
        }
        fprintf(html, "%s", tag + strlen("<credit/>"));
      }
      if((tag = strstr(buffer, "<pieces/>"))) {
        *tag = '\0';
        fprintf(html, "%s", buffer);
        fprintf(html, pieces);
        fprintf(html, "%s", tag + strlen("<pieces/>"));
      }
    }
  }

  /* close html file */
  fclose(html);

  /* free memory */
  free((void*)game_filename);
  free((void*)moves);
  free((void*)notation);
}

/* create html & javascript data for moves in pgn file */
void process_moves(FILE *pgn, const char *FEN, char **moves, char **notation, STRUCTURE layout) /* !! allocates memory which must be freed by caller !! */
{
  VARIATION *root, *current, *new;
  int new_id = 0;
  long int moves_size;
  long int notation_size;

  bool in_comment = false;
  bool left_comment = false;
  bool left_variation = false;
  bool entered_variation = false;
  char move_string[256];
  char token[256];
  char temp[256];
  char *temp_pointer;
  int nag;
  MOVE move;

  /* create root variation */
  root = (VARIATION*)malloc(sizeof(VARIATION));
  root->parent = root->siblings = root->children = 0;
  root->id = new_id++;
  root->buffer_size = 1024;
  root->buffer = (char*)calloc(root->buffer_size, sizeof(char));
  sprintf(root->buffer, "moves[%d] = new Array(", root->id);

  root->parent_move = 0;
  root->actual_move = 1;
  root->relative_move = 1;

  setup_board(&root->position, FEN);
  
  current = root;

  /* allocate notation buffer */
  notation_size = 8192;
  *notation = (char*)calloc(notation_size, sizeof(char));
  strcpy(*notation, "<b>");

  /* parse move text */
  while(current) {
    /* fetch next token */
    token[255] = '\0';
    if(fscanf(pgn, "%255s", token) != 1) {
      strcat(*notation, "</b>");
      strcat(current->buffer, "-1,-1,-1,-1);\n"); /* exit loop if none */
      break;
    }

    /* enlarge buffers if necessary */
    if(strlen(*notation) + strlen(token) + 256 > notation_size) {
      notation_size += 8192;
      *notation = (char*)realloc((void*)*notation, notation_size * sizeof(char));
    }

    if(strlen(current->buffer) + 256 > current->buffer_size) {
      current->buffer_size += 1024;
      current->buffer = (char*)realloc((void*)current->buffer, current->buffer_size * sizeof(char));
    }

    /* parse token */
    while(strcmp(token, "")) {

#ifdef DEBUG
    printf("Token: \"%s\"\n", token);
#endif

      /* if in comment copy to notation */
      if(in_comment) {
	if(strchr(token, '}')) {
	  temp_pointer = strchr(token, '}');
	  *temp_pointer = '\0';
	  strcat(*notation, " ");
	  strcat(*notation, token);

	  temp_pointer++;
	  strcpy(temp, temp_pointer);
	  strcpy(token, temp);
	  in_comment = false;
	  left_comment = true;
	  continue;
	}
	else {
	  strcat(*notation, " ");
	  strcat(*notation, token);
	  strcpy(token, "");
	  continue;
	}
      }

      /* Start of a comment? */
      if(token[0] == '{') {
	if(current->id == 0) {
	  strcat(*notation, "</b>");
	}
	strcat(*notation, "\n");
	strcpy(temp, token + 1);
	strcpy(token, temp);
	in_comment = true;
	continue;
      }

      /* check for result */
      if(!strcmp(token, "1-0") || !strcmp(token, "0-1") || !strcmp(token, "1/2-1/2") || !strcmp(token, "*")) {
	strcat(current->buffer, "-1,-1,-1,-1);\n");
	current = 0;
	break;
      }
      
      /* Replace NAGS with comment/symbol */
      if(token[0] == '$') {
	sscanf(token + 1, "%d", &nag);

	if(nag < 140) {
	  if(isalpha(NAGS[nag][0])) {
	    strcat(*notation, " ");
	  }
	  strcat(*notation, NAGS[nag]);
	}

	temp_pointer = token + 1;
	while(isdigit(*temp_pointer)) {
	  temp_pointer++;
	}
	strcpy(temp, temp_pointer);
	strcpy(token, temp);
	continue;
      }

      /* Start of a variation? */
      if(token[0] == '(') {
	strcpy(temp, token + 1);
	strcpy(token, temp);
	if(current->id == 0) {
	  strcat(*notation, "</b>");
	}
	if(!entered_variation) {
	  strcat(*notation, "\n");
	}
	strcat(*notation, "(");

	/* create child variation */
	new = (VARIATION*)malloc(sizeof(VARIATION));
	new->parent = current;
	new->siblings = new->children = 0;
	new->parent_move = current->relative_move - 2;
	new->actual_move = current->actual_move - 1;
	new->relative_move = 1;

	new->position = current->previous_position;
	
	new->id = new_id++;
	new->buffer_size = 1024;
	new->buffer = (char*)calloc(new->buffer_size, sizeof(char));
	sprintf(new->buffer, "moves[%d] = new Array(", new->id);
      
	/* add variation to tree */
	if(current->children) {
	  current = current->children;
	  while(current->siblings) {
	    current = current->siblings;
	  }      
	  current->siblings = new;
	}
	else {
	  current->children = new;
	}

	/* make current variation */
	current = new;
	entered_variation = true;
	left_comment = false;
	left_variation = false;
	continue;
      }

      /* end of variation? */
      if(token[0] == ')') {
	strcpy(temp, token + 1);
	strcpy(token, temp);
	strcat(*notation, ")");

	/* terminate variation */
	if(current->parent) {
	  strcat(current->buffer, "-1,-1,-1,-1);\n");
	  current = current->parent;
	}
	entered_variation = false;
	left_variation = true;
	continue;
      }

      /* move number? */
      if(isdigit(token[0]) || token[0] == '.') {
	strcpy(temp, token + 1);
	strcpy(token, temp);
	continue;
      }
     
#ifdef DEBUG
      printf("Move token: \"%s\"\n", token);
#endif

      /* parse move */
      strcpy(temp, token);
      temp_pointer = temp;
      while(isalnum(*temp_pointer) || *temp_pointer == '+' || *temp_pointer == '-' || *temp_pointer == '#' || *temp_pointer == '=') {
	temp_pointer++;
      }
      /* if no valid characters, we are confused so abort */
      if(temp_pointer == temp) {
	strcpy(token, "");
	continue;
      }
      strcpy(token, temp_pointer);
      *temp_pointer = '\0';
      strcpy(move_string, temp);

      /* convert the move, checking for failure */
      move = algebraic_to_move(move_string, &current->position);  
      if(move.from_col == -1) {
	continue;
      }

#ifdef DEBUG
      printf("Move: \"%s\"\n", move_string);
#endif

      /* convert move */
      if(!entered_variation || left_comment) {
	strcat(*notation, "\n");
      }

      if(current->id == 0 && (left_comment || left_variation)) {
	strcat(*notation, "<p><b>");
      }

      if(current->position.turn == WHITE) {
	sprintf(*notation + strlen(*notation), "%d.", (current->actual_move + 1) / 2);
      }
      else {
	if(current->relative_move == 1 || left_comment || left_variation) {
	  sprintf(*notation + strlen(*notation), "%d... ", (current->actual_move + 1) / 2);
	}
      } 

      if(layout == FRAMESET) {
	sprintf(*notation + strlen(*notation), "<a class=\"move\" href=\"javascript:parent.board.jumpto(%d, %d);\" id=\"v%dm%d\">%s</a>", current->id, current->relative_move, current->id, current->relative_move, move_string);
      }
      else {
	sprintf(*notation + strlen(*notation), "<a class=\"move\" href=\"javascript:jumpto(%d, %d);\" id=\"v%dm%d\">%s</a>", current->id, current->relative_move, current->id, current->relative_move, move_string);
      }
      append_move(current->buffer, &move, &current->position);
      
      /* execute move */
      current->previous_position = current->position;
      make_move(&current->position, &move);
      left_comment = false;
      left_variation = false;
      entered_variation = false;
      current->actual_move++;
      current->relative_move++;
    }
  }

  /* delete tree structure merging buffers into moves buffer */
  *moves = (char*)calloc(1, sizeof(char));
  **moves = '\0';
  moves_size = 1;
  delete_variation(root, moves, &moves_size);
}

/* strips comments and variations (including NAGs) */
void strip(FILE *pgn)
{
  char test;
  int depth;
  
  for(;;) {
    while((test = getc(pgn)) == ' ' || test == '\n' || test == '\r') {
    }

    if(test != '{' && test != '(' && test != '$') {
      ungetc(test, pgn);
      return;
    }
    
    if(test == '$') {
        while((test = getc(pgn)) != ' ' && test != '\n' && test != '\r') {
        }
        ungetc(test, pgn);
        continue;
    }

    if(test == '{') {
      while(getc(pgn) != '}') {
      }
      continue;
    }

    depth = 1;
    
    while(depth) {
      test = getc(pgn);

      if(test == '{') {
	while(getc(pgn) != '}') {
	}
      }

      if(test == '(') {
	depth++;
      }

      if(test == ')') {
	depth--;
      }
    }
  }
}
