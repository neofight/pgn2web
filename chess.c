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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "chess.h"

void append_move_to_list(move_list_t *move_list, int from, int to,
												 piece_t promotion_piece, int flags)
{
	move_details_t *move_details;
	move_details = &move_list->moves[move_list->length].move_details;
	move_details->from = (unsigned char) from;
	move_details->to = (unsigned char) to;
	move_details->promotion_piece = (unsigned char) promotion_piece;
	move_details->flags = (unsigned char) flags;
	move_list->length++;
}

void append_pawn_move_to_list(move_list_t *move_list, color_t color, int from,
															int to, int flags)
{
	/* check if move is promotion */
	if (is_promotion_row(to)) {

		/* setup template move, copy to all, and then add promotion details */
		move_t template_move;
		int length = move_list->length;

		template_move.move_details.from = (unsigned char) from;
		template_move.move_details.to = (unsigned char) to;
		template_move.move_details.flags =
				(unsigned char) flags | PROMOTION_MOVE;

		/* create actual moves */
		move_list->moves[length] = template_move;
		move_list->moves[length++].move_details.promotion_piece =
				(unsigned char) KNIGHT | color;
		move_list->moves[length] = template_move;
		move_list->moves[length++].move_details.promotion_piece =
				(unsigned char) BISHOP | color;
		move_list->moves[length] = template_move;
		move_list->moves[length++].move_details.promotion_piece =
				(unsigned char) ROOK | color;
		move_list->moves[length] = template_move;
		move_list->moves[length++].move_details.promotion_piece =
				(unsigned char) QUEEN | color;
		move_list->length = length;
	} else {
		move_details_t *move_details;
		move_details = &move_list->moves[move_list->length].move_details;
		move_details->from = (unsigned char) from;
		move_details->to = (unsigned char) to;
		move_details->promotion_piece = (unsigned char) EMPTY;
		move_details->flags = (unsigned char) flags;
		move_list->length++;
	}
}

bool can_to_move(const position_t *position, const char *move_string, /*@out@ */ move_t *move)
{
	int index = 0;
	int from_col, from_row, to_col, to_row, from, to;
	piece_t promotion_piece;
	
	/* determine from square */
	from_col = (int) (move_string[index++] - 'a');
	from_row = (int) ('8' - move_string[index++]);
	
	/* determine to square */
	to_col = (int) (move_string[index++] - 'a');
	to_row = (int) ('8' - move_string[index++]);
	
	/* check if there is a promotion piece */
	if (strlen(move_string) >= 5) {
		promotion_piece =
				char_to_piece(move_string[index++]) | position->turn;
	} else {
		promotion_piece = EMPTY;
	}
	
	/* check validity of coordinates */
	if (from_col < 0 || from_col > 7 ||
			from_row < 0 || from_row > 7 ||
			to_col < 0 || to_col > 7 || to_row < 0 || to_row > 7) {
		
		move->move = 0;
		return false;
	}
	
	/* populate move structure */
	from = from_col + (from_row << 4);
	to = to_col + (to_row << 4);
	
	move->move_details.from = (unsigned char) from;
	move->move_details.to = (unsigned char) to;
	move->move_details.promotion_piece = (unsigned char) promotion_piece;
	
	/* fill in move details */
	move->move_details.flags = (unsigned char) 0;
	
	if (square_88(position, to) != EMPTY) {
		move->move_details.flags |= CAPTURING_MOVE;
	}
	
	if (is_pawn(square_88(position, from))) {
		move->move_details.flags |= PAWN_MOVE;
		
		if ((from_row - to_row == 2) || (to_row - from_row == 2)) {
			move->move_details.flags |= DOUBLE_PAWN_MOVE;
		} else {
			if (to == position->ep_square) {
				move->move_details.flags |= EN_PASSANT_MOVE;
			} else {
				if (is_promotion_row(to)) {
					move->move_details.flags |= PROMOTION_MOVE;
				}
			}
		}
	}
	
	if (is_king(square_88(position, from))) {
		if (to_col - from_col == 2) {
			move->move_details.flags |= CASTLE_KINGSIDE;
		} else {
			if (to_col - from_col == -2) {
				move->move_details.flags |= CASTLE_QUEENSIDE;
			}
		}
	}
	
	return true;
}

piece_t char_to_piece(char letter)
{
	switch (letter) {
	case 'P':
		return WPAWN;
	case 'N':
		return WKNIGHT;
	case 'B':
		return WBISHOP;
	case 'R':
		return WROOK;
	case 'Q':
		return WQUEEN;
	case 'K':
		return WKING;
	case 'p':
		return BPAWN;
	case 'n':
		return BKNIGHT;
	case 'b':
		return BBISHOP;
	case 'r':
		return BROOK;
	case 'q':
		return BQUEEN;
	case 'k':
		return BKING;
	default:
		return EMPTY;
	}
}

void do_move(position_t *position, const move_t *move)
{
	int from = (int)move->move_details.from;
	int to = (int)move->move_details.to;
	color_t color = position->turn;
	piece_t piece = square_88(position,from);
	int square_index;
	
	/* back up data to history stack */
	hist_t* history_item = position->history + position->hist_length++;
	history_item->move = *move;
	history_item->capture = square_88(position,(int)move->move_details.to);
	history_item->castling_rights = position->castling_rights;
	history_item->ep_square = position->ep_square;
  history_item->no_reversable_moves = position->no_reversable_moves;
	history_item->hash = position->hash;	
	
	/* update hash for captured piece and remove it, if applicable) */
	if(history_item->capture != EMPTY) {
		position->hash ^= BOARD_HASHES[history_item->capture][to];
		piece_remove(position, to);
	}
	
	/* move the piece */
	if(move->move_details.flags & PROMOTION_MOVE) {
		position->hash ^= BOARD_HASHES[piece][from] ^ BOARD_HASHES[(piece_t)move->move_details.promotion_piece][to];
		piece_promote(position, from, to, (piece_t)move->move_details.promotion_piece);
	}
	else {
		position->hash ^= BOARD_HASHES[piece][from] ^ BOARD_HASHES[piece][to];
		piece_move(position, from, to);
	}
	
	/* if en passant move, remove the captured pawn) */
	if(move->move_details.flags & EN_PASSANT_MOVE)  {
		square_index = to + (color ? ROW_DELTA : -ROW_DELTA); 
		position->hash ^= BOARD_HASHES[piece][square_index];
		piece_remove(position, square_index);
	}
	
	/* if a castling move the move the rook */
	if(move->move_details.flags & (CASTLE_KINGSIDE | CASTLE_QUEENSIDE)) {
		if(move->move_details.flags & CASTLE_KINGSIDE) {
			from = H8 + (color * 7 * ROW_DELTA);
			to = F8 + (color * 7 * ROW_DELTA);
			position->hash ^= BOARD_HASHES[ROOK | color][from] ^ BOARD_HASHES[ROOK | color][to];
			piece_move(position, from, to); 
		}
		else {
			from = A8 + (color * 7 * ROW_DELTA);
			to = D8 + (color * 7 * ROW_DELTA);
			position->hash ^= BOARD_HASHES[ROOK | color][from] ^ BOARD_HASHES[ROOK | color][to];
			piece_move(position, from, to); 
		}
	}
	
	/* switch turn */
	position->hash ^= TURN_HASHES[color];
	position->turn ^= WHITE;
	position->hash ^= TURN_HASHES[color];
	
	/* castling rights */
	position->hash ^= CASTLING_RIGHTS_HASHES[position->castling_rights];
	position->castling_rights &= CASTLING_MASK[(int)move->move_details.from] & CASTLING_MASK[(int)move->move_details.to];
	position->hash ^= CASTLING_RIGHTS_HASHES[position->castling_rights];
	
	/* update en passant square */
	position->hash ^= EP_COLUMN_HASHES[position->ep_square & 0x15];
	if(move->move_details.flags & DOUBLE_PAWN_MOVE) {
		position->ep_square = (int)move->move_details.to;
		position->ep_square += color ? ROW_DELTA : -ROW_DELTA; 
	}
	else {
		position->ep_square = NO_EP_SQUARE;
	}
	position->hash ^= EP_COLUMN_HASHES[position->ep_square & 0x15];
	
	/* update number of reversable moves */
	if(move->move_details.flags & (CAPTURING_MOVE | PAWN_MOVE)) {
		history_item->no_reversable_moves = 0;
	}
	else {
		history_item->no_reversable_moves++;
	}
}

void generate_disambiguation_squares(position_t *position, int target_square, piece_type_t piece_type, square_list_t *square_list)
{
	piece_t piece = piece_type | position->turn;
	const int *vector;
	int from;
	
	/* Initialize square list */
	square_list->length = 0;
	
	/* Not needed for pawns */
	if(is_pawn(piece)) {
		return;
	}
	
	/* Generate pseudo legal moves matching the given criteria */
	for(vector = PIECE_VECTORS[piece]; *vector; vector++) {
		for(from = target_square + *vector; square_88(position,from) == EMPTY; from += *vector); 
		if(square_88(position,from) == piece) {
			
			/* move found, ignore if the piece is pinned */
			square_88(position,from) = EMPTY;
			if(is_in_check(position, position->turn)) {
				square_88(position,from) = piece;
				continue;
			}
			square_88(position,from) = piece;
			
			/* add source square to the list */
			square_list->squares[square_list->length++] = from;
			continue;
		}
	}
}

void generate_legal_moves(position_t *position, /*@out@ */ move_list_t *move_list)
{
	int move_index; /* current pseudo legal move */
	move_t *move_ptr; /* end of verified legal moves */
	
	/* fetch pseudo legal moves */
	generate_pseudo_legal_moves(position, move_list);
	
	/* for each move, execute it and see if the resultant position is legal */
	move_ptr = move_list->moves;
	for(move_index = 0; move_index < move_list->length; move_index++) {
		do_move(position, &(move_list->moves[move_index]));
		
		/* copy move to end of legal list if it is legal */
		if(!is_in_check(position, position->turn ^ WHITE)) {
			*(move_ptr++) = move_list->moves[move_index];
		}
		 
		undo_move(position);
	}
	
	/* recalculate move list length */
	move_list->length = move_ptr - move_list->moves;
}

void generate_pseudo_legal_moves(const position_t *position,
																 move_list_t *move_list)
{
	const piece_t *piece_ptr;
	const int *location_ptr;
	color_t color;
	piece_t piece;
	int from, to;
	int pawn_delta;
	int vector_index;
	int vector;
	int captured;

	/* initialise move_list */
	move_list->length = 0;

	/* loop through each piece, generating the moves for it */
	color = position->turn;
	for (piece_ptr = position->pieces[color],
			 location_ptr = position->piece_locations[color];
			 *piece_ptr; piece_ptr++, location_ptr++) {

		piece = *piece_ptr;
		from = *location_ptr;

		if (is_pawn(piece)) {

			/* process pawn moves here */
			pawn_delta = color ? -ROW_DELTA : ROW_DELTA;

			to = from + pawn_delta - 1;
			if (is_opponent(position, to)) {
				append_pawn_move_to_list(move_list, color, from, to,
																 CAPTURING_MOVE | PAWN_MOVE);
			}
			
			if (to == position->ep_square) {
				append_pawn_move_to_list(move_list, color, from, to,
																 CAPTURING_MOVE | EN_PASSANT_MOVE | PAWN_MOVE);
			}

			to = from + pawn_delta + 1;
			if (is_opponent(position, to)) {
				append_pawn_move_to_list(move_list, color, from, to,
																 CAPTURING_MOVE | PAWN_MOVE);
			}
			
			if (to == position->ep_square) {
				append_pawn_move_to_list(move_list, color, from, to,
																 CAPTURING_MOVE | EN_PASSANT_MOVE | PAWN_MOVE);
			}

			to = from + pawn_delta;
			if (is_empty(position, to)) {
				append_pawn_move_to_list(move_list, color, from, to, PAWN_MOVE);

				to += pawn_delta;
				if (is_pawn_row(from) && is_empty(position, to)) {
					append_move_to_list(move_list, from, to, 0,
															PAWN_MOVE | DOUBLE_PAWN_MOVE);
				}
			}
		} else {

			/* process piece moves here */
			if (is_sliding_piece(piece)) {

				/* process sliding pieces here */
				vector_index = 0;
				vector = PIECE_VECTORS[piece][0];

				do {

					to = from + vector;
					captured = square_88(position, to);
					while (captured == EMPTY) {
						append_move_to_list(move_list, from, to, 0, 0);
						to += vector;
						captured = square_88(position, to);
					}

					if (is_opposite_side(piece, captured)) {
						append_move_to_list(move_list, from, to, 0, CAPTURING_MOVE);
					}

					vector = PIECE_VECTORS[piece][++vector_index];
				} while (vector);

			} else {

				/* process non-sliding pieces here */
				vector_index = 0;
				vector = PIECE_VECTORS[piece][0];

				do {

					to = from + vector;
					captured = square_88(position, to);

					if (captured == EMPTY) {
						append_move_to_list(move_list, from, to, 0, 0);
					} else {
						if (is_opposite_side(piece, captured)) {
							append_move_to_list(move_list, from, to, 0, CAPTURING_MOVE);
						}
					}

					vector = PIECE_VECTORS[piece][++vector_index];
				} while (vector);

			}

		}
	}
	
	/* generate castling moves */
	if(color == WHITE) {
		if(!is_attacked(position, E1, BLACK)) {
			if((position->castling_rights & WHITE_KINGSIDE) &&
				 ((square_88(position,F1) | square_88(position,G1)) == EMPTY) &&
				 !is_attacked(position, F1, BLACK)) {
				append_move_to_list(move_list, E1, G1, 0, CASTLE_KINGSIDE);
			}
			if((position->castling_rights & WHITE_QUEENSIDE) &&
				 ((square_88(position,B1) | square_88(position,C1) | square_88(position,D1)) == EMPTY) &&
				 !is_attacked(position, D1, BLACK)) {
				append_move_to_list(move_list, E1, C1, 0, CASTLE_QUEENSIDE);
			}
		}
	}
	else {
		if(!is_attacked(position, E8, WHITE)) {
			if((position->castling_rights & BLACK_KINGSIDE) &&
				 ((square_88(position,F8) | square_88(position,G8)) == EMPTY) &&
				 !is_attacked(position, F8, WHITE)) {
				append_move_to_list(move_list, E8, G8, 0, CASTLE_KINGSIDE);
			}
			if((position->castling_rights & BLACK_QUEENSIDE) &&
				 ((square_88(position,B8) | square_88(position,C8) | square_88(position,D8)) == EMPTY) &&
				 !is_attacked(position, D8, WHITE)) {
				append_move_to_list(move_list, E8, C8, 0, CASTLE_QUEENSIDE);
			}
		}
	}
}

void initialize_move_list(move_list_t *move_list)
{
	move_list->length = 0;
}

void initialize_square_list(square_list_t *square_list)
{
	square_list->length = 0;
}

bool initialize_position(position_t *position, const char *fen_string)
{
	int col, row, sq;
	color_t color;
	piece_t piece;
	int spaces;
	int i;

	/* use default position if fen_string is null */
	if (!fen_string) {
		fen_string = INITIAL_POSITION;
	}

	/* set all positions to off board initially */
	for (i = 0; i < 256; i++) {
		position->board[i] = OFF_BOARD;
	}

	/* kings are placed at the begining of their respective arrays
	   (they'll never be captured so this will save searching for the king
			when doing check calculations) */
	
	/* blank out piece arrays */
	position->no_pieces[0] = position->no_pieces[1] = 1; /* reserve index 0 */
	for (i = 0; i < 17; i++) {
		position->pieces[0][i] = 0;
		position->pieces[1][i] = 0;
		position->piece_locations[0][i] = 0;
		position->piece_locations[1][i] = 0;
	}

	/* loop round the FEN string adding pieces and setting squares */
	col = row = 0;
	while (row < 8) {

		/* piece ? */
		if (isalpha(*fen_string)) {

			piece = char_to_piece(*fen_string);
			color = piece_color(piece);
			sq = col + (row * ROW_DELTA);
			square_88(position, sq) = piece;
			if(is_king(piece)) {
				/* place kings at the start of the array */
				position->pieces[color][0] = piece;
				position->piece_locations[color][0] = sq;
			}	
			else {	
				position->pieces[color][position->no_pieces[color]] = piece;
				position->piece_locations[color][position->no_pieces[color]] = sq;
				position->no_pieces[color]++;
			}
			col++;
			if (col > 7) {
				col = 0;
				row++;
			}
			fen_string++;
			continue;
		}

		/* empty squares? */
		spaces = digittoint((int) *fen_string);
		if (spaces) {
			do {
				square_coords(position, col, row) = EMPTY;
				col++;
				spaces--;
			} while (spaces);
			if (col > 7) {
				col = 0;
				row++;
			}
			fen_string++;
			continue;
		}

		if (*fen_string == '/') {
			fen_string++;
			continue;
		}

		/* must be a dodgy character, abort */
		return false;
	}

	/* compute hashes for board */
	position->hash = 0;

	for (sq = 0; sq < 128; sq++) {
		if (sq & 0x88) {
			sq += 7;
			continue;
		}

		/*@ignore@ */
		piece = square_88(position, sq);
		/*@end@ */

		if (piece) {
			position->hash ^= BOARD_HASHES[piece][sq];
		}
	}

	/* skip space */
	fen_string++;

	/* turn to move */
	position->turn = (*fen_string++ == 'w') ? WHITE : BLACK;
	position->hash ^= TURN_HASHES[position->turn];

	/* skip space */
	fen_string++;

	/* castling rights */
	position->castling_rights = 0;
	for (i = 0; i < 4; i++) {
		switch (*fen_string) {
		case 'K':
			position->castling_rights |= WHITE_KINGSIDE;
			break;
		case 'Q':
			position->castling_rights |= WHITE_QUEENSIDE;
			break;
		case 'k':
			position->castling_rights |= BLACK_KINGSIDE;
			break;
		case 'q':
			position->castling_rights |= BLACK_QUEENSIDE;
			break;
		default:
			i = 4;										/* any other character then break out */
		}
		fen_string++;
	}
	position->hash ^= CASTLING_RIGHTS_HASHES[position->castling_rights];

	/* skip space */
	fen_string++;

	/* en passant square */
	if (*fen_string == '-') {
		position->ep_square = NO_EP_SQUARE;
		fen_string++;
	} else {
		position->ep_square = (int)(*fen_string++ - 'a');
		position->ep_square += (int)('8' - *fen_string++) * ROW_DELTA;
	}
	position->hash ^= EP_COLUMN_HASHES[position->ep_square];

	/* skip space */
	fen_string++;

	/* reversable moves made (for 50-move rule) */
	position->no_reversable_moves = (int) strtol(fen_string, NULL, 10);
	if (position->no_reversable_moves > 100) {	/* check the number is sane */
		return false;
	}

	/* no history information available from FEN */
	position->hist_length = 0;

	return true;
}
 
bool is_attacked(const position_t *position, int square_index, color_t color)
{
	const int *vector;
	int from, pawn_delta;
	
	/* check for bishop & queen attacks */
	for(vector = BISHOP_VECTORS; *vector; vector++) {
		for(from = square_index + *vector; square_88(position,from) == EMPTY; from += *vector);
		if((square_88(position,from) == (BISHOP | color)) || (square_88(position,from) == (QUEEN | color))) {
			return true;
		}
	}
	
	/* check for rook and queen attacks */
	for(vector = ROOK_VECTORS; *vector; vector++) {
		for(from = square_index + *vector; square_88(position,from) == EMPTY; from += *vector);
		if((square_88(position,from) == (ROOK | color)) || (square_88(position,from) == (QUEEN | color))) {
			return true;
		}
	}
	
	/* check for knight attacks */
	for(vector = KNIGHT_VECTORS; *vector; vector++) {
		if(square_88(position, square_index + *vector) == (KNIGHT | color)) {
			return true;
		}
	}
	
	/* check for pawn attacks */
	pawn_delta = color ? ROW_DELTA : -ROW_DELTA;
	
	if (square_88(position,square_index + pawn_delta - 1) == (PAWN | color)) {
				return true;
	}
	if (square_88(position,square_index + pawn_delta + 1) == (PAWN | color)) {
		return true;
	}

	/* check for king attacks */
	for(vector = KING_VECTORS; *vector; vector++) {
		if(square_88(position, square_index + *vector) == (KING | color)) {
			return true;
		}
	}
	
	return false;
}

bool is_check(const position_t *position)
{
	color_t color = position->turn;
	return is_attacked(position, position->piece_locations[color][0], opposite_color(color));
}

bool is_checkmate(position_t *position)
{
	move_list_t move_list;
	color_t color = position->turn;
	
	/* if there are no legal moves and the side is in check, it is checkmate */
	generate_legal_moves(position, &move_list);
	if(move_list.length != 0) {
		return false;
	}
		
	return is_attacked(position, position->piece_locations[color][0], opposite_color(color));
}

bool is_in_check(const position_t *position, color_t color)
{
	return is_attacked(position, position->piece_locations[color][0], opposite_color(color));
}

bool lan_to_move(const position_t *position, const char *move_string,
								 move_t *move)
{
	int index = 0;
	int from_col, from_row, to_col, to_row, from, to;
	piece_t promotion_piece;

	/* check for castling moves */
	if(strncmp(move_string, "O-O-O", 5) == 0) {
		if(position->turn == WHITE) {
			move->move_details.from = (unsigned char)E1;
			move->move_details.to = (unsigned char)C1;
		}
		else {
			move->move_details.from = (unsigned char)E8;
			move->move_details.to = (unsigned char)C8;
		}
		move->move_details.promotion_piece = (unsigned char)0;
		move->move_details.flags = (unsigned char)CASTLE_QUEENSIDE;
		return true;
	}
		
	if(strncmp(move_string, "O-O", 3) == 0) {
		if(position->turn == WHITE) {
			move->move_details.from = (unsigned char)E1;
			move->move_details.to = (unsigned char)G1;
		}
		else {
			move->move_details.from = (unsigned char)E8;
			move->move_details.to = (unsigned char)G8;
		}
		move->move_details.promotion_piece = (unsigned char)0;
		move->move_details.flags = (unsigned char)CASTLE_KINGSIDE;
		return true;
	}
	
	/* check if first char is a piece */
	if (move_string[index] < 'a' || move_string[index] > 'h') {
		index++;
	}

	/* determine from square */
	from_col = (int) (move_string[index++] - 'a');
	from_row = (int) ('8' - move_string[index++]);

	/* skip - or x */
	index++;

	/* determine to square */
	to_col = (int) (move_string[index++] - 'a');
	to_row = (int) ('8' - move_string[index++]);

	/* check if there is a promotion piece */
	if (strlen(move_string) >= 7 && move_string[index++] == '=') {
		promotion_piece =
				char_to_piece(move_string[index++]) & ~(position->turn ^ WHITE);
	} else {
		promotion_piece = EMPTY;
	}

	/* check validity of coordinates */
	if (from_col < 0 || from_col > 7 ||
			from_row < 0 || from_row > 7 ||
			to_col < 0 || to_col > 7 || to_row < 0 || to_row > 7) {

		move->move = 0;
		return false;
	}

	/* populate move structure */
	from = from_col + (from_row << 4);
	to = to_col + (to_row << 4);

	move->move_details.from = (unsigned char) from;
	move->move_details.to = (unsigned char) to;
	move->move_details.promotion_piece = (unsigned char) promotion_piece;

	/* fill in move details */
	move->move_details.flags = (unsigned char) 0;

	if (square_88(position, to) != EMPTY) {
		move->move_details.flags |= CAPTURING_MOVE;
	}

	if (is_pawn(square_88(position, from))) {
		move->move_details.flags |= PAWN_MOVE;

		if ((from_row - to_row == 2) || (to_row - from_row == 2)) {
			move->move_details.flags |= DOUBLE_PAWN_MOVE;
		} else {
			if (to == position->ep_square) {
				move->move_details.flags |= EN_PASSANT_MOVE;
			} else {
				if (is_promotion_row(to)) {
					move->move_details.flags |= PROMOTION_MOVE;
				}
			}
		}
	}

	if (is_king(square_88(position, from))) {
		if (to_col - from_col == 2) {
			move->move_details.flags |= CASTLE_KINGSIDE;
		} else {
			if (to_col - from_col == -2) {
				move->move_details.flags |= CASTLE_QUEENSIDE;
			}
		}
	}

	return true;
}

bool move_to_can(const move_t *move, char *buffer, int buffer_length)
{
	int index = 0;
	int from_col, from_row, to_col, to_row;
	move_details_t move_details = move->move_details;
	
	/* buffer must be at least 6 chars long */
	if (buffer_length <= 6) {
		return false;
	}
	
	/* determine from square */
	from_col = COL((int)move_details.from);
	from_row = ROW((int)move_details.from);
	buffer[index++] = 'a' + (char) from_col;
	buffer[index++] = '8' - (char) from_row;
	
	/* determine to square */
	to_col = COL((int)move_details.to);
	to_row = ROW((int)move_details.to);
	buffer[index++] = 'a' + (char) to_col;
	buffer[index++] = '8' - (char) to_row;
	
	/* add promotion if required */
	if(move_details.flags & PROMOTION_MOVE) {
		buffer[index++] = piece_to_char((int)move_details.promotion_piece & ~WHITE);
	}
	
	/* terminate string */
	buffer[index++] = '\0';
	
	return true;
}

bool move_to_lan(position_t *position, const move_t *move,
								 char *buffer, int buffer_length)
{
	piece_t piece;
	int index = 0;
	int from_col, from_row, to_col, to_row;
	move_details_t move_details = move->move_details;

	/* buffer must be at least 9 chars long */
	if (buffer_length <= 9) {
		return false;
	}
	
	/* check for castling moves */
	if(move_details.flags & CASTLE_KINGSIDE) {
		strcpy(buffer, "O-O");
		return true;
	}
	
	if(move_details.flags & CASTLE_QUEENSIDE) {
		strcpy(buffer, "O-O-O");
		return true;
	}
	
	/* determine piece char */
	piece = square_88(position, (int) move_details.from);
	if (is_piece(piece)) {
		buffer[index++] = piece_to_char(piece | WHITE);
	}

	/* determine from square */
	from_col = COL((int)move_details.from);
	from_row = ROW((int)move_details.from);
	buffer[index++] = 'a' + (char) from_col;
	buffer[index++] = '8' - (char) from_row;

	/* determine move type */
	if (move_details.flags & CAPTURING_MOVE) {
		buffer[index++] = 'x';
	} else {
		buffer[index++] = '-';
	}

	/* determine to square */
	to_col = COL((int)move_details.to);
	to_row = ROW((int)move_details.to);
	buffer[index++] = 'a' + (char) to_col;
	buffer[index++] = '8' - (char) to_row;
	
	/* add promotion if required */
	if(move_details.flags & PROMOTION_MOVE) {
		buffer[index++] = '=';
		buffer[index++] = piece_to_char((int)move_details.promotion_piece | WHITE);
	}
	
	/* add check or checkmate symbol if required */
	do_move(position, move);
	if(is_check(position)) {
		if(is_checkmate(position)) {
			buffer[index++] = '#';
		}
		else {
			buffer[index++] = '+';
		}
	}
	undo_move(position);

	/* terminate string */
	buffer[index++] = '\0';

	return true;
}

bool move_to_san(position_t *position, const move_t *move, char *buffer, int buffer_length)
{
	piece_t piece;
	int index = 0;
	int from_col, from_row, to_col, to_row;
	move_details_t move_details = move->move_details;
	square_list_t square_list;
	int square_index;
	bool col_required, row_required;
	col_required = row_required = false;
	
	/* buffer must be at least 8 chars long */
	if (buffer_length <= 8) {
		return false;
	}
	
	/* check for castling moves */
	if(move_details.flags & CASTLE_KINGSIDE) {
		strcpy(buffer, "O-O");
		return true;
	}
	
	if(move_details.flags & CASTLE_QUEENSIDE) {
		strcpy(buffer, "O-O-O");
		return true;
	}
	
	/* determine move coordinates */
	from_col = COL((int)move_details.from);
	from_row = ROW((int)move_details.from);
	to_col = COL((int)move_details.to);
	to_row = ROW((int)move_details.to);
	
	/* determine piece char */
	piece = square_88(position, (int) move_details.from);
	if (is_piece(piece)) {
		buffer[index++] = piece_to_char(piece | WHITE);
	}
	
	/* determine what is required for disambguation */	
	if(is_piece(piece)) {
		generate_disambiguation_squares(position, (int)move_details.to, piece, &square_list);
				
		for(square_index = 0; square_index < square_list.length; square_index++) {
			if(square_list.squares[square_index] != (int)move_details.from) {
				if(COL(square_list.squares[square_index]) == from_col) {
					row_required = true;
				}
				if(ROW(square_list.squares[square_index]) == from_row) {
					col_required = true;
				}
			}
		}
	}
	else {
		/* pawn captures require the column to be specified */
		if(move_details.flags & CAPTURING_MOVE) {
			col_required = true;
		}
	}
		
	/* add disambiguation row and/or column if required */
	if(col_required) {
		buffer[index++] = 'a' + (char) from_col;
	}
	
	if(row_required) {
		buffer[index++] = '8' - (char) from_row;
	}
	
	/* add capture symbol if required */
	if (move_details.flags & CAPTURING_MOVE) {
		buffer[index++] = 'x';
	}
	
	/* add destination square */
	buffer[index++] = 'a' + (char) to_col;
	buffer[index++] = '8' - (char) to_row;
	
	/* add promotion if required */
	if(move_details.flags & PROMOTION_MOVE) {
		buffer[index++] = '=';
		buffer[index++] = piece_to_char((int)move_details.promotion_piece | WHITE);
	}
	
	/* add check or checkmate symbol if required */
	do_move(position, move);
	if(is_check(position)) {
		if(is_checkmate(position)) {
			buffer[index++] = '#';
		}
		else {
			buffer[index++] = '+';
		}
	}
	undo_move(position);
	
	/* terminate string */
	buffer[index++] = '\0';
	
	return true;	
}

void piece_move(position_t *position, int from_index, int to_index)
{
	color_t color= position->turn;
	piece_t *piece_ptr = position->pieces[color];
	int *location_ptr = position->piece_locations[color];
	
	/* Locate piece */
	while(*location_ptr != from_index) {
		piece_ptr++;
		location_ptr++;
	}
	
	/* Change the location */
	*location_ptr = to_index;
	square_88(position,to_index) = *piece_ptr;
	square_88(position,from_index) = EMPTY;
}

void piece_promote(position_t *position, int from_index, int to_index,  piece_t piece)
{
	color_t color = position->turn;
	piece_t *piece_ptr = position->pieces[color];
	int *location_ptr = position->piece_locations[color];
	
	/* Locate piece */
	while(*location_ptr != from_index) {
		piece_ptr++;
		location_ptr++;
	}
	
	/* Change the piece and location */
	*piece_ptr = piece;
	*location_ptr = to_index;
	square_88(position,to_index) = piece;
	square_88(position,from_index) = EMPTY;
}

char piece_to_char(piece_t piece)
{
	switch (piece) {
	case WPAWN:
		return 'P';
	case WKNIGHT:
		return 'N';
	case WBISHOP:
		return 'B';
	case WROOK:
		return 'R';
	case WQUEEN:
		return 'Q';
	case WKING:
		return 'K';
	case BPAWN:
		return 'p';
	case BKNIGHT:
		return 'n';
	case BBISHOP:
		return 'b';
	case BROOK:
		return 'r';
	case BQUEEN:
		return 'q';
	case BKING:
		return 'k';
	default:
		return '?';
	}
}

void piece_remove(position_t *position, int square_index)
{
	color_t color = position->turn ^ WHITE;
	piece_t *piece_ptr = position->pieces[color];
	int *location_ptr = position->piece_locations[color];
	int last_index = --position->no_pieces[color]; /* list shortened here */
	
	/* Locate piece */
	while(*location_ptr != square_index) {
		piece_ptr++;
		location_ptr++;
	}
		
	/* Swap the last piece for the piece being removed and null out its old position*/
	*piece_ptr = position->pieces[color][last_index];
	*location_ptr = position->piece_locations[color][last_index];
	position->pieces[color][last_index] = 0;
	position->piece_locations[color][last_index] = 0;
	square_88(position,square_index) = EMPTY;
}

bool result_to_string(result_t result, char *buffer, int buffer_length)
{
	/* check that the buffer is big enough */
	if(buffer_length < 8) {
		return false;
	}
			
	switch(result & 7) {
		case DRAW:
			strcpy(buffer, "1/2-1/2");
			break;
		case WHITE_WIN:
			strcpy(buffer, "1-0");
			break;
		case BLACK_WIN:
			strcpy(buffer, "0-1");
			break;
		default:
			strcpy(buffer, "*");
			break;
	}
	
	return true;
}

bool result_reason_to_string(result_t result, char *buffer, int buffer_length)
{
	/* check that the buffer is big enough */
	if(buffer_length < (int)strlen("Fifty moves occured without a capture or pawn move")) {
		return false;
	}
	
	switch(result & 248) {
		case AGREED_DRAW:
			strcpy(buffer, "Draw agreed");
			break;
		case FIFTY_MOVE_RULE:
			strcpy(buffer, "Fifty moves occured without a capture or pawn move");
			break;
		case INSUFFICIENT_MATERIAL:
			strcpy(buffer, "Neither side has sufficient material to win");
			break;
		case STALEMATE:
			strcpy(buffer, "Stalemate");
			break;
		case CHECKMATE:
			strcpy(buffer, "Checkmate");
			break;
		default:
			strcpy(buffer, "");
			break;
	}
	
	return true;
}

bool san_to_move(position_t *position, const char *move_string,
								 move_t *move)
{
	int index;
	int to_row, to_col, from_row, from_col;
	piece_t piece;
	square_list_t square_list;
	int square_index;
	
	/* check for castling moves */
	if(strncmp(move_string, "O-O-O", 5) == 0) {
		if(position->turn == WHITE) {
			move->move_details.from = (unsigned char)E1;
			move->move_details.to = (unsigned char)C1;
		}
		else {
			move->move_details.from = (unsigned char)E8;
			move->move_details.to = (unsigned char)C8;
		}
		move->move_details.promotion_piece = (unsigned char)0;
		move->move_details.flags = (unsigned char)CASTLE_QUEENSIDE;
		return true;
	}
		
	if(strncmp(move_string, "O-O", 3) == 0) {
		if(position->turn == WHITE) {
			move->move_details.from = (unsigned char)E1;
			move->move_details.to = (unsigned char)G1;
		}
		else {
			move->move_details.from = (unsigned char)E8;
			move->move_details.to = (unsigned char)G8;
		}
		move->move_details.promotion_piece = (unsigned char)0;
		move->move_details.flags = (unsigned char)CASTLE_KINGSIDE;
		return true;
	}
	
	/* blank out move's flags */
	move->move_details.flags = (unsigned char)0;
	
	/* work from the end of the string to the beginning */
	index = (int)strlen(move_string - 1);
	
	/* skip any check and checkmate characters */
	if(move_string[index] == '+' || move_string[index] == '#') {
		index--;
	}
	
	/* check for a promotion piece */
	if(move_string[index] > 'A' && move_string[index] < 'Z') {
		move->move_details.promotion_piece = (unsigned char)(char_to_piece(move_string[index]) & ~WHITE) | position->turn;
		index -= 2;
		
		move->move_details.flags |= PAWN_MOVE | PROMOTION_MOVE;
	}
	
	/* read in destination square */
	to_row = (int)('8' - move_string[index--]);
	to_col = (int)(move_string[index--]);
	move->move_details.to = (unsigned char)SQ88(to_col,to_row);

	/* check if the move is a capture */
	if(index >= 0 && move_string[index] == 'x') {
		move->move_details.flags |= CAPTURING_MOVE;
	}
	
	/* -1 indicates row / col is unknown */
	from_row = from_col = -1;
	
	/* read in disambiguation row, if any */
	if(index >= 0 && move_string[index] >= '0' && move_string[index] <= '8') {
		from_row = (int)('8' - move_string[index--]);
	}
	
	/* read in disambiguation col, if any */
	if(index >= 0 && move_string[index] >= 'a' &&  move_string[index] <= 'h') {
		from_col = (int)(move_string[index--] - 'a');
	}
	
	/* determine piece, and from square */
	if(index == 0) {
		piece = (char_to_piece(move_string[index]) & ~WHITE) | position->turn;
		
		/* locate piece using disambiguation squares */
		generate_disambiguation_squares(position, (int)move->move_details.to, piece, &square_list);
				
		for(square_index = 0; square_index < square_list.length; square_index++) {
			if((COL(square_list.squares[square_index]) == from_col || from_col == -1) &&
				 (ROW(square_list.squares[square_index]) == from_row || from_row == -1)) {
				 move->move_details.from = (unsigned char)square_list.squares[square_index];
				 break;
				}
		}
		
	} else {
		piece = PAWN | position->turn;
		
		/* locate pawn */
		if(move->move_details.flags & CAPTURING_MOVE) {
			/* pawn capture */
			move->move_details.from = (unsigned char)SQ88(from_col,to_row + (position->turn == WHITE ? 1 : -1));
			move->move_details.to = (unsigned char)SQ88(to_col,to_row);
			
			/* check if the capture is en passant */
			if(square_coords(position,to_col,to_row) == EMPTY) {
				move->move_details.flags |= EN_PASSANT_MOVE;
			}
		}
		else {
			/* pawn advance, check if single or double move */
			if(square_coords(position,to_col,to_row + (position->turn == WHITE ? 1 : -1)) == EMPTY) {
				move->move_details.flags |= DOUBLE_PAWN_MOVE;
				move->move_details.from = (unsigned char)SQ88(to_col,to_row + (position->turn == WHITE ? 2 : -2));
			}
			else {
				move->move_details.from = (unsigned char)SQ88(to_col,to_row + (position->turn == WHITE ? 1 : -1));
			}
		}
	}
	
	return true;
}

void undo_move(position_t *position)
{
	move_t* move;
	color_t color, opp_color;
	int square_index, last_index;
	
	/* restore saved status information */
	hist_t* history_item = position->history + --position->hist_length;
	move = &(history_item->move);
	position->castling_rights = history_item->castling_rights;
	position->ep_square = history_item->ep_square;
  position->no_reversable_moves = history_item->no_reversable_moves;
	position->hash = history_item->hash;
	
	/* switch turn */
	opp_color = position->turn;
	position->turn ^= WHITE;
	color = position->turn;
	
	/* move the piece */
	if(move->move_details.flags & PROMOTION_MOVE) {
		piece_promote(position, (int)move->move_details.to, (int)move->move_details.from, PAWN | color);
	}
	else {
		piece_move(position, (int)move->move_details.to, (int)move->move_details.from);
	}
	
  /* if capture then add the captured piece */
	if(history_item->capture != EMPTY) {
		square_88(position,(int)move->move_details.to) = history_item->capture;
		last_index = position->no_pieces[opp_color]++;
		position->pieces[opp_color][last_index] = history_item->capture;
		position->piece_locations[opp_color][last_index] = (int)move->move_details.to;
	}
		
	/* if en passant move, add the captured pawn */
	if(move->move_details.flags & EN_PASSANT_MOVE)  {
		square_index = (int)move->move_details.to + (color ? ROW_DELTA : -ROW_DELTA); 
		square_88(position,square_index) = PAWN | opp_color;
		last_index = position->no_pieces[opp_color]++;
		position->pieces[opp_color][last_index] = PAWN | opp_color;
		position->piece_locations[opp_color][last_index] = square_index;
	}
	
	/* if a castling move the move the rook */
	if(move->move_details.flags & (CASTLE_KINGSIDE | CASTLE_QUEENSIDE)) {
		if(move->move_details.flags & CASTLE_KINGSIDE) {
			piece_move(position, F8 + (color * 7 * ROW_DELTA), H8 + (color * 7 * ROW_DELTA)); 
		}
		else {
			piece_move(position, D8 + (color * 7 * ROW_DELTA), A8 + (color * 7 * ROW_DELTA)); 
		}
	}
}

