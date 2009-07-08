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

#ifndef _TYPES_H_
#define _TYPES_H_

/* bitfields 8 = offboard ,7 = moves diagonally, 6 = moves up & across,
 *					 5 = sliding piece, 4 = piece, 3 = pawn,  2 = occupied, 1 = colour
 */

#define color_t int
																/* ODASPPOC */
#define BLACK 0									/* 00000000 */
#define WHITE 1									/* 00000001 */

#define piece_type_t int
																/* ODASPPOC */
#define PAWN     6							/* 00000110 */
#define KNIGHT  10							/* 00001010 */
#define BISHOP  90							/* 01011010 */
#define ROOK    58							/* 00111010 */
#define QUEEN  122							/* 01111010 */
#define KING   106							/* 01101010 */

																/* ODASPPOC */
#define EMPTY     0							/* 00000000 */
#define OFF_BOARD 128						/* 10000000 */

#define piece_t int
																/* ODASPPOC */
#define BPAWN     6							/* 00000110 */
#define BKNIGHT  10							/* 00001010 */
#define BBISHOP  90							/* 01011010 */
#define BROOK		 58							/* 00111010 */
#define BQUEEN  122							/* 01111010 */
#define BKING   106							/* 01101010 */
#define WPAWN			7							/* 00000111 */
#define WKNIGHT	 11							/* 00001011 */
#define WBISHOP	 91							/* 01011011 */
#define WROOK    59							/* 00111011 */
#define WQUEEN  123							/* 01111011 */
#define WKING   107							/* 01101011 */

#define result_t int
#define UNKNOWN_RESULT        0
#define DRAW                  1
#define WHITE_WIN             2
#define BLACK_WIN             4
#define AGREED_DRAW           8
#define FIFTY_MOVE_RULE       16
#define INSUFFICIENT_MATERIAL 32
#define STALEMATE             64
#define CHECKMATE             128

typedef struct {
	
	int length;
	int squares[64];
	
} square_list_t;

#define castling_rights_t int
#define BLACK_KINGSIDE  1
#define BLACK_QUEENSIDE 2
#define WHITE_KINGSIDE  4
#define WHITE_QUEENSIDE 8

#define hash_t unsigned long long int

typedef struct {

	unsigned char from;
	unsigned char to;
	unsigned char promotion_piece;
	unsigned char flags;

} move_details_t;

#define CAPTURING_MOVE		1
#define PAWN_MOVE					2
#define DOUBLE_PAWN_MOVE	4
#define EN_PASSANT_MOVE		8
#define PROMOTION_MOVE		16
#define CASTLE_KINGSIDE		32
#define CASTLE_QUEENSIDE	64

typedef union {

	move_details_t move_details;
	int move;

} move_t;

typedef struct {

	int length;
	move_t moves[256];

} move_list_t;

typedef struct {

	move_t move;
	piece_t capture;
	castling_rights_t castling_rights;
	int ep_square;
	int no_reversable_moves;
	hash_t hash;

} hist_t;

typedef struct {

	int board[256];

	int no_pieces[2];
	piece_t pieces[2][17];
	int piece_locations[2][17];

	color_t turn;
	castling_rights_t castling_rights;
	int ep_square;
	hash_t hash;
	int no_reversable_moves;

	int hist_length;
	hist_t history[256];

} position_t;

#define NO_EP_SQUARE 9
#define ROW_DELTA 16
#define A8 0
#define B8 1
#define C8 2
#define D8 3
#define E8 4
#define F8 5
#define G8 6
#define H8 7
#define A1 112
#define B1 113
#define C1 114
#define D1 115
#define E1 116
#define F1 117
#define G1 118
#define H1 119

#endif
