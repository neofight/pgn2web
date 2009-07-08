/*
  pgn2web - Converts PGN files to interactive web pages

  Copyright (C) 2004-2009 William Hoggarth <email: whoggarth@users.sourceforge.net>

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

#include <stdbool.h>

#include "types.h"

#define square_88(position,square_index) ((position)->board[64 + (square_index)])
#define square_coords(position,col,row) ((position)->board[64 + (col) + ((row) << 4)])

#define COL(square_index) ((square_index) & 0x07)
#define ROW(square_index) (((square_index) & 0x70) >> 4)
#define SQ88(col,row) ((col) + ((row * 16)))

#define is_empty(position,square_index) (!square_88(position,square_index))
#define is_king(piece) (((piece) | WHITE) == WKING)
#define is_opponent(position,square_index) ((((position)->turn) ^ (square_88(position,square_index) & (WHITE | 2))) == 3)
#define is_opposite_side(piece1,piece2) ((((piece1) & (WHITE)) ^ ((piece2) & (WHITE | 2))) == 3)
#define is_pawn(piece) ((piece) & 4)
#define is_pawn_row(square_index) ((((square_index) & 0x70) == 0x10) || (((square_index) & 0x70) == 0x60))
#define is_piece(piece) ((piece) & 8)
#define is_promotion_row(square_index) ((((square_index) & 0x70) == 0x00) || (((square_index) & 0x70) == 0x70))
#define is_sliding_piece(piece) ((piece) & 16)
#define is_white(piece) ((piece) & WHITE)
#define opposite_color(color) ((color) ^ WHITE)
#define piece_color(piece) ((piece) & WHITE)

/*** Function prototypes ***/

void append_move_to_list(move_list_t *move_list, int from, int to, piece_t promotion_piece, int flags);	/** Appends a move to a move list */
void append_pawn_move_to_list(move_list_t *move_list, color_t color, int from, int to, int flags);	/** Appends a pawn move to a move list */
bool can_to_move(const position_t *position, const char *move_string, move_t *move); /** Converts Coordinate Algebraic Notation to a move */
piece_t char_to_piece(char letter);	/** Converts a char to piece. */
void do_move(position_t *position, const move_t *move);	/** Executes a move in a given position */
void generate_disambiguation_squares(position_t *position, int target_square, piece_type_t piece_type, square_list_t *square_list); /** Generates a list of squares to aid disambiguation */
void generate_legal_moves(position_t *position, move_list_t *move_list); /** Generates legal moves for a position */
void generate_pseudo_legal_moves(const position_t *position, move_list_t *move_list); /** Generates pseudo legal moves for a position */
void initialize_move_list(move_list_t *move_list); /** Initializes a move list. */
void initialize_square_list(square_list_t *square_list); /** Initializes a square list. */
bool initialize_position(position_t *position, const char *fen_string);	/** Initializes a position. */
bool is_attacked(const position_t *position, int square_index, color_t color); /** Checks whether a square is attacked by a given color */
bool is_check(const position_t *position); /** Tests whether the side to move is in check */
bool is_checkmate(position_t *position); /** Tests whether the side to move has been checkmated */
bool is_in_check(const position_t *position, color_t color); /** Tests whether a side is in check */
bool lan_to_move(const position_t *position, const char *move_string, /*@out@ */ move_t *move); /** Converts Long Algebraic Notation to a move */
bool move_to_can(const move_t *move, char *buffer, int buffer_length); /** Converts a move to Coordinate Algebraic Notation */
bool move_to_lan(position_t *position, const move_t *move, char *buffer, int buffer_length); /** Converts a move to Long Algebraic Notation */
bool move_to_san(position_t *position, const move_t *move, char *buffer, int buffer_length); /** Converts a move to Short Algebraic Notation */
void piece_move(position_t *position, int from_index, int to_index); /** Moves a piece from one location to another */
void piece_promote(position_t *position, int from_index, int to_index,  piece_t piece); /** Moves and promotes a piece*/
char piece_to_char(piece_t piece); /** Converts a piece to a char */
void piece_remove(position_t *position, int square_index); /** Removes a piece at a given square */
bool result_to_string(result_t result, char *buffer, int buffer_length); /** Converts a result to a string */
bool result_reason_to_string(result_t result, char *buffer, int buffer_length); /** Converts a result to a string containing the type of result */
bool san_to_move(position_t *position, const char *move_string, move_t *move); /** Converts Short Algebraic Notation to a move */
void undo_move(position_t *position);	/** Undos the last move made in the position */

#endif
