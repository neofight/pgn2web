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

#ifndef _data_h_
#define _data_h_

#include "types.h"

/** FEN string for the standard start position */
extern const char *INITIAL_POSITION;

/* Piece move vectors */
extern const int KNIGHT_VECTORS[9];
extern const int BISHOP_VECTORS[5];
extern const int ROOK_VECTORS[5];
extern const int QUEEN_VECTORS[9];
extern const int KING_VECTORS[9];
extern const int *PIECE_VECTORS[128];

/* Hash values */
extern const hash_t BPAWN_HASHES[128];
extern const hash_t BKINGHT_HASHES[128];
extern const hash_t BBISHOP_HASHES[128];
extern const hash_t BROOK_HASHES[128];
extern const hash_t BQUEEN_HASHES[128];
extern const hash_t BKING_HASHES[128];
extern const hash_t WPAWN_HASHES[128];
extern const hash_t WKNIGHT_HASHES[128];
extern const hash_t WBISHOP_HASHES[128];
extern const hash_t WROOOK_HASHES[128];
extern const hash_t WQUEEN_HASHES[128];
extern const hash_t QKING_HASHES[128];
extern const hash_t *BOARD_HASHES[128];
extern const hash_t TURN_HASHES[2];
extern const hash_t CASTLING_RIGHTS_HASHES[16];
extern const hash_t EP_COLUMN_HASHES[10];

/* Castling mask */
extern const castling_rights_t CASTLING_MASK[128];

#endif
