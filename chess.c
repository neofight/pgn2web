/*
  neophyte - A Winboard compatible chess engine

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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "chess.h"

#ifdef DEBUG
/* globals */
int allocated = 0;
int freed = 0;
#endif

/* constants */
const MOVEVECTOR MOVEVECTORS[5][8] = {
  { {1, 2, 1}, {2, 1, 1},  {2, -1, 1},  {1, -2, 1}, {-1, -2, 1}, {-2, -1, 1}, {-2, 1, 1},  {-1, 2, 1} },
  { {1, 1, 7}, {1, -1, 7}, {-1, -1, 7}, {-1, 1, 7}, {0, 0, 0},   {0, 0, 0},   {0, 0, 0},   {0, 0, 0} },
  { {0, 1, 7}, {1, 0, 7},  {0, -1, 7},  {-1, 0, 7}, {0, 0, 0},   {0, 0, 0},   {0, 0, 0},   {0, 0, 0} },
  { {0, 1, 7}, {1, 0, 7},  {0, -1, 7},  {-1, 0, 7}, {1, 1, 7},   {1, -1, 7},  {-1, -1, 7}, {-1, 1, 7} },
  { {0, 1, 1}, {1, 0, 1},  {0, -1, 1},  {-1, 0, 1}, {1, 1, 1},   {1, -1, 1},  {-1, -1, 1}, {-1, 1, 1} }
};

/* !!! Allocates memory, free node when done with list !!! */
void add_move_to_list(MOVE_LIST_NODE** move_list, int from_col, int from_row, int to_col, int to_row)
{
  MOVE_LIST_NODE* node = (MOVE_LIST_NODE*)malloc(sizeof(MOVE_LIST_NODE));

#ifdef DEBUG
  allocated++;
#endif

  /* create move list node */
  node->move.from_col = from_col;
  node->move.from_row = from_row;
  node->move.to_col = to_col;
  node->move.to_row = to_row;
  node->move.promotion_piece = NO_PIECE_TYPE;

  /* add node to list */
  node->previous = 0;
  if(*move_list) {
    node->next = *move_list;
    (*move_list)->previous = node;
  }
  else {
    node->next = 0;
  }
  *move_list = node;
}

void add_promotions_to_list(MOVE_LIST_NODE** move_list, int from_col, int from_row, int to_col, int to_row)
{
  PIECE_TYPE promotion_piece;
  MOVE_LIST_NODE* node;

  for(promotion_piece = KNIGHT; promotion_piece <= QUEEN; promotion_piece++) {

    node = (MOVE_LIST_NODE*)malloc(sizeof(MOVE_LIST_NODE));

#ifdef DEBUG
    allocated++;
#endif

    /* create move list node */
    node->move.from_col = from_col;
    node->move.from_row = from_row;
    node->move.to_col = to_col;
    node->move.to_row = to_row;
    node->move.promotion_piece = promotion_piece;
    
    /* add node to list */
    node->previous = 0;
    if(*move_list) {
      node->next = *move_list;
      (*move_list)->previous = node; 
    }
    else {
      node->next = 0;
    }
    *move_list = node;
  }
}

PIECE char_to_piece(char character)
{
  switch(character) {
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
    return NO_PIECE;
  }
}

void delete_move_list(MOVE_LIST_NODE* move_list)
{
  MOVE_LIST_NODE* current;
  MOVE_LIST_NODE* next;

  current = move_list;
  while(current) {
    next = current->next;
    free((void*)current);
    current = next;

#ifdef DEBUG
    freed++;
#endif
  }
}

MOVE_LIST_NODE* get_legal_moves(const POSITION *position) /* !!! Allocates memory, free list when done with it !!! */
{
  /* get pseudo legal moves and delete those that leave player in check */
  MOVE_LIST_NODE* move_list = get_pseudo_legal_moves(position);
  MOVE_LIST_NODE* current = move_list;
  MOVE_LIST_NODE* illegal_move;
  POSITION test_position;
 
  while(current) {
    test_position = *position;
    make_move(&test_position, &(current->move));
    
    /* if the resultant position is illegal delete the move from the list */
    if(!is_legal_position(&test_position)) { 

      illegal_move = current;
      if(current->previous) {
	current->previous->next = current->next;
      }
      if(current->next) {
	current->next->previous = current->previous;
      }
      if(current == move_list) {
	move_list = current->next;
      }
      
      current = current->next;

      free((void*)illegal_move);

#ifdef DEBUG
      freed++;
#endif
    }
    else {
      current = current->next;
    }
  }

  return move_list;
}

/* !!! Allocates memory, free list when done with it !!! */
MOVE_LIST_NODE* get_pseudo_legal_moves(const POSITION* position)
{
  MOVE_LIST_NODE* move_list = 0;

  int from_col, from_row, to_col, to_row, vector, range;
  PIECE piece;
  COLOUR players_colour, opponents_colour;
  PIECE_TYPE piece_type;
  MOVEVECTOR move_vector;

  players_colour = position->turn;
  opponents_colour = (position->turn == WHITE) ? BLACK : WHITE;

  for(from_col = 0; from_col < 8; from_col++) {
    for(from_row = 0; from_row < 8; from_row++) {

      piece = position->board[from_col][from_row];

      /* don't bother if it's not a piece of the right colour */
      if(piece_to_colour(piece) != players_colour) {
	continue;
      }

      /* check type of piece */
      piece_type = piece_to_piece_type(piece);
      if(piece_type != PAWN) {

	/* process piece moves */
	for(vector = 0; vector < 8; vector++) {
	  move_vector = MOVEVECTORS[(int)piece_to_piece_type(piece) - 2][vector];
	  to_col = from_col;
	  to_row = from_row;

	  for(range = 0; range < move_vector.range; range++) {
	    to_col += move_vector.col_vector;
	    to_row += move_vector.row_vector;
	    
	    /* if off board stop */
	    if (to_col < 0 || to_col > 7 || to_row < 0 || to_row > 7) {
	      break;
	    }
	    
	    /* if empty square add move to list and continue */
	    if(position->board[to_col][to_row] == NO_PIECE) {
	      add_move_to_list(&move_list, from_col, from_row, to_col, to_row);
	      continue;
	    }
	    
	    /* if opponent's piece add move to list and stop */
	    if(piece_to_colour(position->board[to_col][to_row]) == opponents_colour) {
	      add_move_to_list(&move_list, from_col, from_row, to_col, to_row);
	      break;
	    }
	    
	    /* if own piece stop */
	    if(piece_to_colour(position->board[to_col][to_row]) == players_colour) {
	      break;
	    }
	  }
	}
      }
      else {
	/* process pawn moves */
	vector = (piece_to_colour(piece) == WHITE) ? 1 : -1;
	
	/* single advance? */
	if(position->board[from_col][from_row + vector] == NO_PIECE) {
	  
	  /* promotion? */
	  if((from_row + vector) == 0 || (from_row + vector) == 7) {
	    add_promotions_to_list(&move_list, from_col, from_row, from_col, from_row + vector);
	  }
	  else {
	    add_move_to_list(&move_list, from_col, from_row, from_col, from_row + vector);
	    
	    /* double advance? */
	    if(from_row == ((position->turn == WHITE) ? 1 : 6) && position->board[from_col][from_row + (vector * 2)] == NO_PIECE) {
	      add_move_to_list(&move_list, from_col, from_row, from_col, (from_row + vector * 2));
	      }
	  }
	}
	
	/* capture left? */
	if(from_col > 0) {
	  
	  /* e.p. capture?*/
	  if((from_col - 1) == position->ep_col && from_row == ((position->turn == WHITE) ? 4 : 3)) {

	    /* promotion? */
	    if(from_row + vector == 0 || from_row + vector == 7) {
	      add_promotions_to_list(&move_list, from_col, from_row, from_col -1, from_row + vector);
	    }
	    else {
	      add_move_to_list(&move_list, from_col, from_row, from_col - 1, from_row + vector);
	    }
	  }
	  else {
	    
	    /* normal capture? */
	    if(piece_to_colour(position->board[from_col - 1][from_row + vector]) == opponents_colour) {

	      /* promotion? */
	      if(from_row + vector == 0 || from_row + vector == 7) {
		add_promotions_to_list(&move_list, from_col, from_row, from_col - 1, from_row + vector);
	      }
	      else {
		add_move_to_list(&move_list, from_col, from_row, from_col - 1, from_row + vector);
	      }
	    }
	  }
	}
	
	/* capture right? */
	if(from_col < 7) {
	  
	  /* e.p. capture?*/
	  if((from_col + 1) == position->ep_col && from_row == ((position->turn == WHITE) ? 4 : 3)) {

	    /* promotion? */
	    if(from_row + vector == 0 || from_row + vector == 7) {
	      add_promotions_to_list(&move_list, from_col, from_row, from_col + 1, from_row + vector);
	    }
	    else {
	      add_move_to_list(&move_list, from_col, from_row, from_col + 1, from_row + vector);
	    }
	  }
	  else {
	    
	    /* normal capture? */
	    if(piece_to_colour(position->board[from_col + 1][from_row + vector]) == opponents_colour) {
	      
	      /* promotion? */
	      if(from_row + vector == 0 || from_row + vector == 7) {
		add_promotions_to_list(&move_list, from_col, from_row, from_col + 1, from_row + vector);
	      }
	      else {
		add_move_to_list(&move_list, from_col, from_row, from_col + 1, from_row + vector);
	      }
	    }
	  }
	}
      }

      /* process castling moves */
      if(piece_type == KING) {
	
	/* castle kingside? */
	if(((players_colour == WHITE) ? position->wkcr : position->bkcr) &&
	   position->board[from_col + 1][from_row] == NO_PIECE &&
	   position->board[from_col + 2][from_row] == NO_PIECE &&
	   !is_square_attacked(position, from_col, from_row, opponents_colour) &&
	   !is_square_attacked(position, from_col + 1, from_row, opponents_colour) &&
	   !is_square_attacked(position, from_col + 2, from_row, opponents_colour)) {

	    add_move_to_list(&move_list, from_col, from_row, from_col + 2, from_row);
	}

	/* castle queenside? */
	if(((players_colour == WHITE) ? position->wqcr : position->bqcr) &&
	   position->board[from_col - 1][from_row] == NO_PIECE &&
	   position->board[from_col - 2][from_row] == NO_PIECE &&
	   position->board[from_col - 3][from_row] == NO_PIECE &&
	   !is_square_attacked(position, from_col, from_row, opponents_colour) &&
	   !is_square_attacked(position, from_col - 1, from_row, opponents_colour) &&
	   !is_square_attacked(position, from_col - 2, from_row, opponents_colour)) {

	    add_move_to_list(&move_list, from_col, from_row, from_col - 2, from_row);
	}
      }
    }
  }

  return move_list;
}

RESULT get_result(const POSITION *position)
{
  MOVE_LIST_NODE* move_list = get_legal_moves(position);
  POSITION test_position = *position;

  /* if there are legal moves left to play then the game has not finished */
  if(move_list) {
    delete_move_list(move_list);
    return NO_RESULT;
  }

  /* if in check, must be check mate, else stalemate */
  test_position.turn = (test_position.turn == WHITE) ? BLACK : WHITE;
  if(is_legal_position(&test_position)) {
    return DRAW;
  }
  else {
    return (test_position.turn == WHITE) ? WHITE_WIN : BLACK_WIN;
  }
}

RESULT_TYPE get_result_type(const POSITION *position)
{
  MOVE_LIST_NODE* move_list = get_legal_moves(position);
  POSITION test_position = *position;

  /* if there are legal moves left to play then the game has not finished */
  if(move_list) {
    delete_move_list(move_list);
    return NO_RESULT_TYPE;
  }

  /* if in check, must be check mate, else stalemate */
  test_position.turn = (test_position.turn == WHITE) ? BLACK : WHITE;
  if(is_legal_position(&test_position)) {
    return STALEMATE;
  }
  else {
    return CHECKMATE;
  }
}

bool is_legal_position(const POSITION *position)
{
  MOVE_LIST_NODE* move_list = get_pseudo_legal_moves(position);
  MOVE_LIST_NODE* current = move_list;
  bool legal = true;
    
  /* loop through moves checking if a king is captured */
  while(current) {
    if(piece_to_piece_type(position->board[current->move.to_col][current->move.to_row]) == KING) {
      legal = false;
      break;
    }
    current = current->next;
  }
  
  delete_move_list(move_list);
  return legal;
}

bool is_legal_move(const POSITION *position, MOVE const *move)
{
  POSITION test_position;
  MOVE_LIST_NODE* move_list;
  MOVE_LIST_NODE* test_move;
  bool legal = false;

  /* check the move is in our move list */
  move_list = get_pseudo_legal_moves(position);
  test_move = move_list;

  while(test_move) {
    if(test_move->move.from_col == move->from_col &&
       test_move->move.from_row == move->from_row &&
       test_move->move.to_col == move->to_col &&
       test_move->move.to_row == move->to_row &&
       test_move->move.promotion_piece == move->promotion_piece) {
      
      legal = true;
      break;
    }

    test_move = test_move->next;
  }

  delete_move_list(move_list);

  /* make sure no-one is left in check */
  test_position = *position;
  make_move(&test_position, move);
  legal = legal && is_legal_position(&test_position); 

  return legal;
}

int is_square_attacked(const POSITION *position, int col, int row, COLOUR attacking_colour)
{
  int attacked = 0;
  int vector, range;
  int attacker_col, attacker_row;
  bool n, ne, e, se, s, sw, w, nw;
  PIECE bishop = piece_type_and_colour_to_piece(BISHOP, attacking_colour);
  PIECE rook = piece_type_and_colour_to_piece(ROOK, attacking_colour);
  PIECE queen = piece_type_and_colour_to_piece(QUEEN, attacking_colour);
  
  /* check for pawn attacks */
  vector = (attacking_colour == WHITE) ? -1 : 1;

  if(col > 0 && position->board[col - 1][row + vector] == piece_type_and_colour_to_piece(PAWN, attacking_colour)) {
    attacked++;
  }

  if(col < 7 && position->board[col + 1][row + vector] == piece_type_and_colour_to_piece(PAWN, attacking_colour)) {
    attacked++;
  }

  /* check for knight attacks */
  for(vector = 0; vector < 8; vector++) {
    attacker_col = col + MOVEVECTORS[KNIGHT - 2][vector].col_vector;
    attacker_row = row + MOVEVECTORS[KNIGHT - 2][vector].row_vector;
    
    /* make sure square is on board */
    if(attacker_col >= 0 && attacker_col < 8 && attacker_row >= 0 && attacker_row < 8) {
     
      /* check if square contains any enemy knight */
      if(position->board[attacker_col][attacker_row] == piece_type_and_colour_to_piece(KNIGHT, attacking_colour)) {
	attacked++;
      }
    }
  }

  /* check for attacks from long range pieces */

  n = ne = e = se = s = sw = w = nw = true; /* keeps track of which directons are not blocked by a piece yet */

  for(range = 1; range <= 7; range++) {

    /* north */
    if(n && row + range < 8) {
      if(position->board[col][row + range] != NO_PIECE) {
	if(position->board[col][row + range] == rook || position->board[col][row + range] == queen) {
	  attacked++;
	}
	else {
	  n = false;
	}
      }
    }
    
    /* north east */
    if(ne && col + range < 8 && row + range < 8) {
      if(position->board[col + range][row + range] != NO_PIECE) {
	if(position->board[col + range][row + range] == bishop || position->board[col + range][row + range] == queen) {
	  attacked++;
	}
	else {
	  ne = false;
	}
      }
    }

    /* east */
    if(e && col + range < 8) {
      if(position->board[col + range][row] != NO_PIECE) {
	if(position->board[col + range][row] == rook || position->board[col + range][row] == queen) {
	  attacked++;
	}
	else {
	  e = false;
	}
      }
    }

    /* south east */
    if(se && col + range < 8 && row - range >= 0) {
      if(position->board[col + range][row - range] != NO_PIECE) {
	if(position->board[col + range][row - range] == bishop || position->board[col + range][row - range] == queen) {
	  attacked++;
	}
	else {
	  se = false;
	}
      }
    }

  
    /* south */
    if(s && row - range >= 0) {
      if(position->board[col][row - range] != NO_PIECE) {
	if(position->board[col][row - range] == rook || position->board[col][row - range] == queen) {
	  attacked++;
	}
	else {
	  s = false;
	}
      }
    }
    
    /* south west */
    if(sw && col - range >= 0 && row - range >= 0) {
      if(position->board[col - range][row - range] != NO_PIECE) {
	if(position->board[col - range][row - range] == bishop || position->board[col - range][row - range] == queen) {
	  attacked++;
	}
	else {
	  sw = false;
	}
      }
    }

    /* west */
    if(w && col - range >= 0) {
      if(position->board[col - range][row] != NO_PIECE) {
	if(position->board[col - range][row] == rook || position->board[col - range][row] == queen) {
	  attacked++;
	}
	else {
	  w = false;
	}
      }
    }

    /* noth west */
    if(nw && col - range >= 0 && row + range < 8) {
      if(position->board[col - range][row + range] != NO_PIECE) {
	if(position->board[col - range][row + range] == bishop || position->board[col - range][row + range] == queen) {
	  attacked++;
	}
	else {
	  nw = false;
	}
      }
    }
    
  }

  return attacked;
}

void make_move(POSITION* position, const MOVE* move)
{
  PIECE piece;
  PIECE_TYPE piece_type;

  piece = position->board[move->from_col][move->from_row];
  piece_type = piece_to_piece_type(piece);

  if(piece_type == PAWN) {
    /* if double pawn move, set ep. column */
    if((move->to_row - move->from_row) == 2 || (move->from_row - move->to_row) == 2) {
      position->ep_col = move->to_col;
    }
    else {
      position->ep_col = -1;
    }

    /* if e.p. capture remove captured pawn */
    if(move->to_col != move->from_col && position->board[move->to_col][move->to_row] == NO_PIECE) {
      position->board[move->to_col][move->from_row] = NO_PIECE;
    }
  }

  /* move piece */
  if(move->promotion_piece == NO_PIECE_TYPE) {
    position->board[move->to_col][move->to_row] = piece;
  }
  else {
    position->board[move->to_col][move->to_row] = piece_type_and_colour_to_piece(move->promotion_piece, position->turn);
  }
  position->board[move->from_col][move->from_row] = NO_PIECE;

  /* castling move corresponding rook also */
  if(piece_type == KING && ((move->to_col - move->from_col) == 2 || (move->from_col - move->to_col) == 2)) {
    if(move->to_col > move->from_col) {
      position->board[move->to_col - 1][move->to_row] = position->board[7][move->to_row];
      position->board[7][move->to_row] = NO_PIECE;
    }
    else {
      position->board[move->to_col + 1][move->to_row] = position->board[0][move->to_row];
      position->board[0][move->to_row] = NO_PIECE;
    }
  }

  /* update castling rights */
  if(piece_type == KING) {
    if(position->turn == WHITE) {
      position->wkcr = position->wqcr = false;
    }
    else {
      position->bkcr = position->bqcr = false;
    }
  }

  if(piece_type == ROOK) {
    if(move->from_row == 0) {
      if(move->from_col == 0) {
	position->wqcr = false;
      }
      if(move->from_col == 7) {
	position->wkcr = false;
      }
    }

    if(move->from_row == 7) {
      if(move->from_col == 0) {
	position->bqcr = false;
      }
      if(move->from_col == 7) {
	position->bkcr = false;
      }
    }
  }    

  /* next player's turn */
  position->turn = (position->turn == WHITE) ? BLACK : WHITE;
}

/* !!! Allocates memory, free string when done with it !!! */
char* move_to_string(const MOVE *move)
{
  char* string;

  if(move->promotion_piece == NO_PIECE) {
    string = (char*)calloc(5, sizeof(char));
  }
  else {
    string = (char*)calloc(6, sizeof(char));
  }

  string[0] = 'a' + move->from_col;
  string[1] = '1' + move->from_row;
  string[2] = 'a' + move->to_col;
  string[3] = '1' + move->to_row;

  if(move->promotion_piece == NO_PIECE) {
    string[4] = '\0';
  }
  else {
    string[4] = piece_to_char(piece_type_and_colour_to_piece(move->promotion_piece, BLACK));
    string[5] = '\0';
  }

  return string;
}

char piece_to_char(PIECE piece)
{
  switch(piece) {
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
    return ' ';
  }
}

COLOUR piece_to_colour(PIECE piece)
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
    return NO_COLOUR;
  }
}

PIECE_TYPE piece_to_piece_type(PIECE piece)
{
  switch(piece) {
  case WPAWN:
  case BPAWN:
    return PAWN;
  case WKNIGHT:
  case BKNIGHT:
    return KNIGHT;
  case WBISHOP:
  case BBISHOP:
    return BISHOP;
  case WROOK:
  case BROOK:
    return ROOK;
  case WQUEEN:
  case BQUEEN:
    return QUEEN;
  case WKING:
  case BKING:
    return KING;
  default:
    return NO_PIECE_TYPE;
  }
}

PIECE piece_type_and_colour_to_piece(PIECE_TYPE piece_type, COLOUR colour)
{
  return (colour == WHITE) ? piece_type : piece_type + WKING;
}


void setup_board(POSITION* position, const char* FEN)
{
  int col = 0;
  int row = 7;
  int fill;

  /* parse placement data */
  while(row >= 0) {
    if(*FEN == '/') {
      col = 0;
      row--;
      FEN++;
      continue;
    }

    if(isalpha(*FEN)) {
      position->board[col][row] = char_to_piece(*FEN);
      col++;
      FEN++;
    }
    else {
      for(fill = 0; fill < (*FEN - '0'); fill++) {
	position->board[col][row] = NO_PIECE;
	col++;
      }
      FEN++;
    }

    if(col == 8) {
      col = 0;
      row--;
      FEN++;
    }
  }

  /* active colour */
  position->turn = (*FEN == 'w') ? WHITE : BLACK;
  FEN += 2;

  /* castling availability */
  position->wkcr = position->wqcr = position->bkcr = position->bqcr = false;
  while(*FEN != ' ') {
    switch(*FEN) {
    case 'K':
      position->wkcr = true;
      break;
    case 'Q':
      position->wqcr = true;
      break;
    case 'k':
      position->bkcr = true;
      break;
    case 'q':
      position->bqcr = true;
      break;
    default:
      break;
    }
    FEN++;
  }
  FEN++;

  /* en passant square */
  position->ep_col = -1;
  if(*FEN != '-') {
    position->ep_col = *FEN - 'a';
    FEN++;
  }
  FEN += 2;

  /* reversable moves made (for 50-move rule) */
  sscanf(FEN, "%d, %*d", &position->reversable_moves);
}

MOVE string_to_move(const char* notation)
{
  MOVE move;
  move.from_col = notation[0] - 'a';
  move.from_row = notation[1] - '1';
  move.to_col = notation[2] - 'a';
  move.to_row = notation[3] - '1';

  if(notation[4] != '\0') {
    move.promotion_piece = piece_to_piece_type(char_to_piece(notation[4]));
  }
  else {
    move.promotion_piece = NO_PIECE;
  }

  return move;
}
