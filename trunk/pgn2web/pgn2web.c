/*
  pgn2web - Converts PGN files to interactive web pages

  Copyright (C) 2004 William Hoggarth <email: me@whoggarth.com>

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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* default installation path */
#ifndef INSTALL_PATH
#ifdef WINDOWS
#define INSTALL_PATH "C:\\Progra~1\\pgn2web\\"
#else
#define INSTALL_PATH "/usr/local/pgn2web/"
#endif
#endif

/* type definitions */
typedef enum { FALSE, TRUE } BOOL;

/* chess type definitions */
typedef enum { NEITHER, WHITE, BLACK } COLOUR;

typedef enum { NONE,
	       WPAWN, WKNIGHT, WBISHOP, WROOK, WQUEEN, WKING,
	       BPAWN, BKNIGHT, BBISHOP, BROOK, BQUEEN, BKING } PIECE;

typedef struct { 
  int from_col;
  int from_row;
  int to_col;
  int to_row;
} MOVE;

typedef struct {
  MOVE move_1;
  MOVE move_2;
} MOVEPAIR;

typedef struct {
  int col_vector;
  int row_vector;
  int range;
} MOVEVECTOR;

/* chess constants */
const MOVEPAIR WCASTLEKINGSIDE = { { 4, 7, 6, 7 },
				   { 7, 7, 5, 7 } };

const MOVEPAIR WCASTLEQUEENSIDE = { { 4, 7, 2, 7 },
				    { 0, 7, 3, 7 } };

const MOVEPAIR BCASTLEKINGSIDE = { { 4, 0, 6, 0 },
				   { 7, 0, 5, 0 } };

const MOVEPAIR BCASTLEQUEENSIDE = { { 4, 0, 2, 0 },
				    { 0, 0, 3, 0 } };

const PIECE initial_position[8][8] = { { BROOK,   BPAWN, NONE, NONE, NONE, NONE, WPAWN, WROOK },
				       { BKNIGHT, BPAWN, NONE, NONE, NONE, NONE, WPAWN, WKNIGHT },
				       { BBISHOP, BPAWN, NONE, NONE, NONE, NONE, WPAWN, WBISHOP },
				       { BQUEEN,  BPAWN, NONE, NONE, NONE, NONE, WPAWN, WQUEEN },
				       { BKING,   BPAWN, NONE, NONE, NONE, NONE, WPAWN, WKING },
				       { BBISHOP, BPAWN, NONE, NONE, NONE, NONE, WPAWN, WBISHOP },
				       { BKNIGHT, BPAWN, NONE, NONE, NONE, NONE, WPAWN, WKNIGHT },
				       { BROOK,   BPAWN, NONE, NONE, NONE, NONE, WPAWN, WROOK } };

MOVEVECTOR MOVEVECTORS[5][8] = {
  { {1, 2, 1}, {2, 1, 1},  {2, -1, 1},  {1, -2, 1}, {-1, -2, 1}, {-2, -1, 1}, {-2, 1, 1},  {-1, 2, 1} },
  { {1, 1, 7}, {1, -1, 7}, {-1, -1, 7}, {-1, 1, 7}, {0, 0, 0},   {0, 0, 0},   {0, 0, 0},   {0, 0, 0} },
  { {0, 1, 7}, {1, 0, 7},  {0, -1, 7},  {-1, 0, 7}, {0, 0, 0},   {0, 0, 0},   {0, 0, 0},   {0, 0, 0} },
  { {0, 1, 7}, {1, 0, 7},  {0, -1, 7},  {-1, 0, 7}, {1, 1, 7},   {1, -1, 7},  {-1, -1, 7}, {-1, 1, 7} },
  { {0, 1, 1}, {1, 0, 1},  {0, -1, 1},  {-1, 0, 1}, {1, 1, 1},   {1, -1, 1},  {-1, -1, 1}, {-1, 1, 1} }
};

/* constants */
const char *const template_filename = INSTALL_PATH "template.html";

/* function prototypes */
void append_move(char* string, const MOVEPAIR movepair);
MOVEPAIR convert_move(const char* algebraic, const COLOUR turn, const PIECE board[8][8]);
MOVE extract_coordinates(const char* algebraic);
void extract_game_list(FILE* file, const char* html_filename, char* game_list);
PIECE get_colour(const PIECE piece);
MOVEPAIR get_pawn_move(const MOVE move, const COLOUR colour, const PIECE board[8][8]);
PIECE identify_piece(const COLOUR colour, const char letter);
BOOL is_legal(const int from_col, const int from_row, const int to_col, const int to_row, const PIECE board[8][8]);
BOOL is_pinned(const int from_col, const int from_row, int to_col, int to_row, const PIECE board[8][8]);
void make_move(const MOVEPAIR movepair, PIECE board[8][8]);
void print_initial_position(FILE* file, const char* var);
void process_game(FILE *pgn, FILE *template, const char *html_filename, const int game, const char* game_list);
void process_moves(FILE* pgn, char* moves, char* notation);
void strip(FILE *pgn);

/* main function */
int main(int argc, char *argv[])
{
  FILE *pgn, *template;
  char game_list[1048576];
  int game = 0;
  char test;

  /* do copy-images option if present */
  if(argc == 2 && !strcmp(argv[1], "--copy-images")) {
#ifdef WINDOWS
    system("MD images");
    exit(system("COPY " INSTALL_PATH "images images"));
#else
    exit(system("cp -r " INSTALL_PATH "images ."));
#endif
  }

  /* check validity of arguments */
  if(argc != 3) {
    printf("Usage: pgn2web pgn-file html-file\n");
    exit(1);
  }

  /* open files */
  if((pgn = fopen(argv[1], "r")) == NULL) {
    perror("Unable to open pgn file");
    exit(1);
  }

  if((template = fopen(template_filename, "r")) == NULL) {
    perror("Unable to open template file");
    exit(1);
  }

  /* extract game list */
  extract_game_list(pgn, argv[2], game_list);
  rewind(pgn);

  /* ensure created files have access of 0644 */
#ifndef WINDOWS
  umask(0133);
#endif

  /* skip any whitespace (or garbage) */
  while((test = getc(pgn)) != '[' && test != EOF) {
  }
  ungetc(test, pgn);

  /* process games */
  while(!feof(pgn)) {
    rewind(template); /* go back to start of template */
  
    /* process game */
    process_game(pgn, template, argv[2], game, game_list);
    game++;

    /* skip remaining whitespace (and any garbage) */
    while((test = getc(pgn)) != '[' && test != EOF) {
    }
    ungetc(test, pgn);
  }

  /* close files */
  fclose(pgn);
  fclose(template);

  /* sucess! */
  exit(0);
}

/* append to string move as javascript data */
void append_move(char* string, const MOVEPAIR movepair)
{
  sprintf(string + strlen(string), "%d,%d,", movepair.move_1.from_col + 8 * movepair.move_1.from_row, movepair.move_1.to_col + 8 * movepair.move_1.to_row);
  string += strlen(string);

  if(movepair.move_2.from_col != -1) {
    if(movepair.move_2.from_col >= 0) {
      sprintf(string + strlen(string), "%d,%d,", movepair.move_2.from_col + 8 * movepair.move_2.from_row, movepair.move_2.to_col + 8 * movepair.move_2.to_row);
    }
    else {
      sprintf(string + strlen(string), "%d,-1,", movepair.move_2.from_col);      
    }
  }
  else {
    sprintf(string + strlen(string), "-1,-1,");
  }
}

/* converts an algebraic move to a set of co-ordinates */
MOVEPAIR convert_move(const char* algebraic, COLOUR turn, const PIECE board[8][8])
{
  PIECE piece;
  MOVEPAIR movepair;
  char *equals;
  char promotion;
  int col, row;

  /* check for castling moves */
  if(!strncmp("O-O-O", algebraic, 5)) {
    movepair = (turn == WHITE) ? WCASTLEQUEENSIDE : BCASTLEQUEENSIDE;
    return movepair;
  }

  if(!strncmp("O-O", algebraic, 3)) {
    movepair = (turn == WHITE) ? WCASTLEKINGSIDE : BCASTLEKINGSIDE;
    return movepair;
  }

  /* identify piece */
  piece = identify_piece(turn, algebraic[0]);

  /* add destination square and any qualifying rows or columns */
  movepair.move_1 = extract_coordinates(algebraic);

  /* process pawn moves */
  if(piece == WPAWN || piece == BPAWN) {
    movepair =  get_pawn_move(movepair.move_1, turn, board);

    /* is it a promotion? */
    if(movepair.move_1.to_row == 0 || movepair.move_1.to_row == 7) {
      equals = strchr(algebraic, '=');
      if(equals == NULL) {
	if(isalpha(algebraic[strlen(algebraic) - 1])) {
	  promotion = algebraic[strlen(algebraic) - 1];
	}
	else {
	  promotion = algebraic[strlen(algebraic) - 2];
	}
      }
      else {
        promotion = *(equals + 1);
      }
      movepair.move_2.from_col = -identify_piece(turn, promotion);
    }

    return movepair;
  }

  /* search for matching piece */
  for(col = 0; col < 8; col++) {
    if(movepair.move_1.from_col != -1 && movepair.move_1.from_col != col) {
      continue;
    }
    for(row = 0; row < 8; row++) {
      if(movepair.move_1.from_row != -1 && movepair.move_1.from_row != row) {
	continue;
      }

      /* check the piece is of the right type and can move to the specified square */
      if(board[col][row] == piece && !is_pinned(col, row, movepair.move_1.to_col, movepair.move_1.to_row, board) && is_legal(col, row, movepair.move_1.to_col, movepair.move_1.to_row, board)) {
	movepair.move_1.from_col = col;
	movepair.move_1.from_row = row;
	movepair.move_2.from_col = movepair.move_2.from_row = movepair.move_2.to_col = movepair.move_2.to_row = -1;
	break;
      }
    }
  }

  return movepair;
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

  return move;
}

/* constructs game list from STRs */
void extract_game_list(FILE* file, const char* html_filename, char* game_list)
{
  char buffer[256];
  char white[256];
  char black[256];
  char date[256];
  int game = 0;
  char url[256];
  char game_index[20];

  /* check filename is not too long */
  if(strlen(html_filename) > 200) {
    fprintf(stderr, "HTML filename too long.\n");
    exit(1);
  }

  strcpy(game_list, "");
  strcpy(white, "");
  strcpy(black, "");
  strcpy(date, "");

  while(fgets(buffer, 256, file) != NULL) {
    sscanf(buffer, "[Date \"%[^\"]\"]", date);
    sscanf(buffer, "[White \"%[^\"]\"]", white);
    sscanf(buffer, "[Black \"%[^\"]\"]", black);

    if(strcmp(white, "") && strcmp(black, "")) {
      /* generate filename */
      strcpy(url, html_filename);
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

      strcat(game_list, "<option value=\"");
      strcat(game_list, url);
      strcat(game_list, "\">");
      strcat(game_list, white);
      strcat(game_list, " - ");
      strcat(game_list, black);
      strcat(game_list, " ");
      strcat(game_list, date);
      strcat(game_list, "\n");

      strcpy(white, "");
      strcpy(black, "");
      strcpy(date, "");

      game++;
    }
  }
}

/* return the colour of a piece */
PIECE get_colour(const PIECE piece)
{
  switch(piece) {
  case WPAWN:
  case WKNIGHT:
  case WBISHOP:
  case WROOK:
  case WQUEEN:
  case WKING:
    return WHITE;
  case BPAWN:
  case BKNIGHT:
  case BBISHOP:
  case BROOK:
  case BQUEEN:
  case BKING:
    return BLACK;
  default:
    return NEITHER;
  }
}

/* generates pawn move */
MOVEPAIR get_pawn_move(MOVE move, COLOUR colour, const PIECE board[8][8])
{
  MOVEPAIR pawn_move;
  int vector = (colour == WHITE) ? 1 : -1;

  pawn_move.move_1 = move;
  pawn_move.move_2.from_col = pawn_move.move_2.from_row = pawn_move.move_2.to_col = pawn_move.move_2.to_row = -1;

  /* is it a capture? */
  if(pawn_move.move_1.from_col != -1) {
    pawn_move.move_1.from_row = pawn_move.move_1.to_row + vector;
    /* is it an en passant capture? */
    if(board[pawn_move.move_1.to_col][pawn_move.move_1.to_row] == NONE) {
      pawn_move.move_2 = pawn_move.move_1;
      pawn_move.move_1.from_col = pawn_move.move_2.to_col; 
      pawn_move.move_1.from_row = pawn_move.move_2.to_row + vector;
      pawn_move.move_1.to_col = pawn_move.move_2.to_col; 
      pawn_move.move_1.to_row = pawn_move.move_2.to_row;
    }
  }
  else {
    pawn_move.move_1.from_col = pawn_move.move_1.to_col;
    /* is it a double move? */
    if(board[pawn_move.move_1.to_col][pawn_move.move_1.to_row + vector] == NONE) {
      pawn_move.move_1.from_row = pawn_move.move_1.to_row + vector * 2;
    }   
    else {
      pawn_move.move_1.from_row = pawn_move.move_1.to_row + vector; 
    }
  }

    return pawn_move;
}

/* convert letter and colour to PIECE type */
PIECE identify_piece(COLOUR colour, char letter)
{
  switch(letter) {
  case 'N':
    return (colour == WHITE) ? WKNIGHT : BKNIGHT;
  case 'B':
    return (colour == WHITE) ? WBISHOP : BBISHOP;
  case 'R':
    return (colour == WHITE) ? WROOK : BROOK;
  case 'Q':
    return (colour == WHITE) ? WQUEEN : BQUEEN;
  case 'K':
    return (colour == WHITE) ? WKING : BKING;
  default:
    return (colour == WHITE) ? WPAWN : BPAWN;
  }
}

/* is a move legal (ignoring any checks)? */
BOOL is_legal(const int from_col, const int from_row, const int to_col, const int to_row, const PIECE board[8][8])
{
  int piece;
  int vector;
  MOVEVECTOR current_vector;
  int current_col, current_row;
  BOOL legal = FALSE;

  /* select right MOVEVECTOR array index for piece */ 
  switch(board[from_col][from_row]) {
  case WKNIGHT:
  case BKNIGHT:
    piece = 0;
    break;
  case WBISHOP:
  case BBISHOP:
    piece = 1;
    break;
  case WROOK:
  case BROOK:
    piece = 2;
    break;
  case WQUEEN:
  case BQUEEN:
    piece = 3;
    break;
  case WKING:
  case BKING:
    piece = 4;
    break;
  default:
    return FALSE;
  }

  /*loop through move vectors trying to match moves with the desired destiniation */
  for(vector = 0; vector < 8; vector++) {
    current_vector =  MOVEVECTORS[piece][vector];
    if(current_vector.range == 0) {
      break;
    }

    current_col = from_col;
    current_row = from_row;

    while(current_vector.range--) {
      current_col += current_vector.col_vector;
      current_row += current_vector.row_vector;

      /* if off board stop */
      if(current_col < 0 || current_col > 7 || current_row < 0 || current_row > 7) {
	break;
      }

      /* if hit own piece stop*/
      if(get_colour(board[current_col][current_row]) == get_colour(board[from_col][from_row])) {
	break;
      }

      /* check if square matches required destination */
      if(current_col == to_col && current_row == to_row) {
	legal = TRUE;
	break;
      }

      /* if hit enemy piece stop */
      if(get_colour(board[current_col][current_row]) != NEITHER) {
	break;
      }
    }

    if(legal) {
      break;
    }
  }

  return legal;  
}

/* checks if a piece is pinned */
BOOL is_pinned(const int from_col, const int from_row, int to_col, int to_row, const PIECE board[8][8])
{
  PIECE king;
  int row, col;
  int king_col, king_row;
  int col_vector, row_vector;

  /* if piece is king return false */
  if(board[from_col][from_row] == WKING || board[from_col][from_row] == BKING) {
    return FALSE;
  }

  /* locate king */
  king = (get_colour(board[from_col][from_row]) == WHITE) ? WKING : BKING;

  for(col = 0; col < 8; col++) {
    for(row = 0; row < 8; row++) {
      if(king == board[col][row]) {
	king_col = col;
	king_row = row;
	break;
      }
    }
  }

  /* check for direct line */
  if(from_col != king_col && from_row != king_row && abs(from_col - king_col) != abs(from_row - king_row)) {
    return FALSE;
  }

  /* find / generate direction */
  col_vector = 0;
  col_vector -= (from_col < king_col) ? 1 : 0;
  col_vector += (from_col > king_col) ? 1 : 0;

  row_vector = 0;
  row_vector -= (from_row < king_row) ? 1 : 0;
  row_vector += (from_row > king_row) ? 1 : 0;

  /* look along direct line for attacking pieces */
  col = king_col;
  row = king_row;

  for(;;) {
    col += col_vector;
    row += row_vector;

    /* check that we are not off board */
    if(col < 0 || col > 7 || row < 0 || row > 7) {
      return FALSE;
    }

    /* return false if destination square retains the pin */
    if(col == to_col && row == to_row) {
      return FALSE;
    }

    /* continue if empty square */
    if(board[col][row] == NONE) {
      continue;
    }

    /* continue if piece that is being checked */
    if(col == from_col && row == from_row) {
      continue;
    }

    /* return false if piece of same colour */
    if(get_colour(board[col][row]) == get_colour(board[from_col][from_row])) {
      return FALSE;
    }

    /* test if it is a possible checking piece */
    if(!col_vector || !row_vector) {
      switch(board[col][row]) {
      case WQUEEN:
      case BQUEEN:
      case WROOK:
      case BROOK:
	return TRUE;
      default:
	return FALSE;
      }
    }
    else {
      switch(board[col][row]) {
      case WQUEEN:
      case BQUEEN:
      case WBISHOP:
      case BBISHOP:
	return TRUE;
      default:
	return FALSE;
      }
    }
  }

  return FALSE;
}

/* execute move on board */
void make_move(const MOVEPAIR movepair, PIECE board[8][8])
{
  /* first move */
  board[movepair.move_1.to_col][movepair.move_1.to_row] = board[movepair.move_1.from_col][movepair.move_1.from_row];
  board[movepair.move_1.from_col][movepair.move_1.from_row] = NONE;
  
  /* second move */
  if(movepair.move_2.to_col >= 0) {
    board[movepair.move_2.to_col][movepair.move_2.to_row] = board[movepair.move_2.from_col][movepair.move_2.from_row];
    board[movepair.move_2.from_col][movepair.move_2.from_row] = NONE;
  }

  /* promotion */
  if(movepair.move_2.from_col < -1) {
    board[movepair.move_1.to_col][movepair.move_1.to_row] = -movepair.move_2.from_col;
  }
}

/* output javascript data for initial position */
void print_initial_position(FILE* file, const char* var)
{
  int col, row;

  fprintf(file, "var %s = new Array(", var);

  for(row = 0; row < 8; row++) {
    for(col = 0; col < 8; col++) {
      if(col == 7 && row == 7) {
	fprintf(file, "%d);\n", initial_position[col][row]);
      }
      else {
	fprintf(file, "%d,", initial_position[col][row]);
      }
    }
  }
}

/* process 1 pgn game */
void process_game(FILE *pgn, FILE *template, const char *html_filename, int game, const char* game_list)
{
  char game_filename[256];
  char game_index[16];
  FILE *html;
  char buffer[256];

  char event[256];
  char site[256];
  char date[256];
  char round[256];
  char white[256];
  char black[256];
  char result[256];
  char moves[16384];
  char notation[16384];

  strcpy(moves, "");
  strcpy(notation, "");

  /* open html file */
  if(strlen(html_filename) > 200) {
    fprintf(stderr, "HTML filename too long.\n");
    exit(1);
  }

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

    if(!strcmp("\n", buffer)) { break; }
    if(!strcmp("\r\n", buffer)) { break; }
  }

  /* process move text */
  process_moves(pgn, moves, notation);

  /* process template file, replacing XML-like tags */
  while(fgets(buffer, 256, template) != NULL) {
    if(strstr(buffer, "/>") == NULL) {
      fprintf(html, "%s", buffer);
    }
    else {
      if(strstr(buffer, "<black/>")) {
	fprintf(html, "%s\n", black);
      }
      if(strstr(buffer, "<board/>")) {
	print_initial_position(html, "board");
      }
      if(strstr(buffer, "<date/>") && strcmp(date, "?")) {
	fprintf(html, "%s\n", date);
      }
      if(strstr(buffer, "<event/>")  && strcmp(event, "?")) {
	fprintf(html, "%s\n", event);
      }
      if(strstr(buffer, "<gamelist/>")) {
	fprintf(html, "%s\n", game_list);
      }
      if(strstr(buffer, "<initial/>")) {
	print_initial_position(html, "initial");
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
    }
  }

  /* close html file */
  fclose(html);
}

/* create html & javascript data for moves in pgn file */
void process_moves(FILE* pgn, char* moves, char* notation)
{
  int moveno = 1;
  int col, row;
  PIECE board[8][8];
  char move[256];
  MOVEPAIR movepair;

#ifdef DEBUG
  static int game = 0;
  game++;
  printf("\n\n");
#endif
 
  /* setup board */
  for(col = 0; col < 8; col++) {
    for(row = 0; row < 8; row++) {
      board[col][row] = initial_position[col][row];
    }
  }

  /* process moves */
  sprintf(moves, "var moves = new Array(");

  for(;;) {
    /* process white move */
    strip(pgn);

    if(fscanf(pgn, "%*d.%s", move) != 1) {
      fscanf(pgn, "%s", move);
      break;
    }

#ifdef DEBUG
    printf("%d: %d. %s", game, moveno, move);
#endif

    sprintf(notation + strlen(notation), "<a href=\"javascript:jumpto(%d)\" class=\"move\" id=\"m%d\"=>%d.%s</a> ", moveno * 2 - 1, moveno * 2 - 1, moveno, move);
    movepair = convert_move(move, WHITE, board);
    append_move(moves, movepair);
    make_move(movepair, board);

    /* process black move */
    strip(pgn);

    if(fscanf(pgn, "%*d...%s", move) != 1) {
      if(fscanf(pgn, "%s", move) != 1) {
	break;
      }
    }

    /* check for result (note first digit is consumed by %*d above */
    if(!strcmp(move, "-0") || !strcmp(move, "-1") || !strcmp(move, "/2-1/2") || !strcmp(move, "*")) {
      break;
    }

#ifdef DEBUG
    printf(" %s\n", move);
#endif

    sprintf(notation + strlen(notation), "<a href=\"javascript:jumpto(%d)\" class=\"move\" id=\"m%d\">%s</a> ", moveno * 2, moveno * 2, move);
    movepair = convert_move(move, BLACK, board);
    append_move(moves, movepair);
    make_move(movepair, board);

    /* increase move numner */
    moveno++;
  }

  sprintf(moves + strlen(moves), "-1,-1,-1,-1);\n");
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
