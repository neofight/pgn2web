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

#ifndef _CHESS_H_
#define _CHESS_H_

/* typedefs */
typedef enum {false, true} bool;

typedef enum {NO_COLOUR, WHITE, BLACK} COLOUR;
typedef enum {NO_PIECE_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING} PIECE_TYPE;
typedef enum {NO_PIECE, WPAWN, WKNIGHT, WBISHOP, WROOK, WQUEEN, WKING, BPAWN, BKNIGHT, BBISHOP, BROOK, BQUEEN, BKING} PIECE;
typedef enum {NO_RESULT, DRAW, WHITE_WIN, BLACK_WIN} RESULT;
typedef enum {NO_RESULT_TYPE, CHECKMATE, STALEMATE, THREE_FOLD_REPETITION, FIFTY_MOVE_RULE, INSUFFICIENT_MATERIAL} RESULT_TYPE;

typedef struct {
  int from_col, from_row, to_col, to_row;
  PIECE_TYPE promotion_piece;
} MOVE;

typedef struct node {
  struct node* previous;
  MOVE move;
  struct node* next;
} MOVE_LIST_NODE;

typedef struct {
  PIECE board[8][8];
  COLOUR turn;
  bool wkcr, wqcr, bkcr, bqcr; /* castling rights */
  int ep_col; /* en passant column */
  int reversable_moves; /* for 50-move rule */
} POSITION;

typedef struct {
  int col_vector;
  int row_vector;
  int range;
} MOVEVECTOR;

#ifdef DEBUG
/* globals */
extern int allocated;
extern int freed;
#endif

/* constants */
#define INITIAL_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
extern const MOVE NULL_MOVE;
extern const MOVE WCASTLEKINGSIDE;
extern const MOVE WCASTLEQUEENSIDE;
extern const MOVE BCASTLEKINGSIDE;
extern const MOVE BCASTLEQUEENSIDE;
extern const MOVEVECTOR MOVEVECTORS[5][8];

/* function prototypes */
void add_move_to_list(MOVE_LIST_NODE **move_list, int from_col, int from_row, int to_col, int to_row);
void add_promotions_to_list(MOVE_LIST_NODE **move_list, int from_col, int from_row, int to_col, int to_row);
MOVE algebraic_to_move(const char *notation, const POSITION *position);
PIECE char_to_piece(char character);
PIECE_TYPE char_to_piece_type(char character);
void delete_move_list(MOVE_LIST_NODE *move_list);
MOVE_LIST_NODE* get_legal_moves(const POSITION *position); /* !!! Allocates memory, free list when done with it !!! */
MOVE_LIST_NODE* get_pseudo_legal_moves(const POSITION *position); /* !!! Allocates memory, free list when done with it !!! */
RESULT get_result(const POSITION *position);
RESULT_TYPE get_result_type(const POSITION *position);
bool is_legal_position(const POSITION *position);
bool is_legal_move(const POSITION *position, MOVE const *move);
int is_square_attacked(const POSITION *position, int col, int row, COLOUR attacking_colour);
void make_move(POSITION *position, const MOVE *move);
char* move_to_string(const MOVE *move); /* !!! Allocates memory, free string when done with it !!! */
char piece_to_char(PIECE piece);
COLOUR piece_to_colour(PIECE piece);
PIECE_TYPE piece_to_piece_type(PIECE piece);
PIECE piece_type_and_colour_to_piece(PIECE_TYPE piece_type, COLOUR colour);
void setup_board(POSITION* position, const char *FEN);
MOVE string_to_move(const char *notation);

#endif
