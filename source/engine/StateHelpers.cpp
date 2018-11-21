/*
  Copyright 2018 Jevgenijs Protopopovs

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
  in the documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "reversi/engine/Engine.h"

namespace Reversi {

  bool StateHelpers::hasMoves(const State &state, Player player) {
    std::vector<Position> moves;
    state.getBoard().getMoves(moves, player);
    return !moves.empty();
  }

  bool StateHelpers::isMovePossible(const State &state, Player player, Position position) {
    std::vector<Position> moves;
    state.getBoard().getMoves(moves, player);
    return std::count(moves.begin(), moves.end(), position) > 0;
  }

  bool StateHelpers::isGameFinished(const State &state) {
    return !(StateHelpers::hasMoves(state, Player::White) || StateHelpers::hasMoves(state, Player::Black));
  }

  State StateHelpers::getDefaultInitialState() {
    Board board;
    board.putDisc(Position('E', 5), Player::White);
    board.putDisc(Position('D', 4), Player::White);
    board.putDisc(Position('D', 5), Player::Black);
    board.putDisc(Position('E', 4), Player::Black);
    return State(board, Player::Black);
  }

  int32_t StateHelpers::assessState(const State &state) {
    const static int WEIGHTS[8][8] = {
        { 500,  -3, 6, 4, 4, 6,  -3, 500 },
        { -3, -250, 3, 1, 1, 3, -250, -3 },
        { 6,   3, 5, 3, 3, 5,   3,  6 },
        { 4,   1, 3, 1, 1, 3,   1,  4 },
        { 4,   1, 3, 1, 1, 3,   1,  4 },
        { 6,   3, 5, 3, 3, 5,   3,  6 },
        { -3, -250, 3, 1, 1, 3, -250, -3 },
        { 500,  -3, 6, 4, 4, 6,  -3, 500 }
    };
    const int32_t END_OF_GAME_THRESHOLD = 59;
	const int32_t NEAR_END_OF_GAME = 54;

    int32_t white_discs =  state.getBoard().getMetric([](int32_t sum, CellState state, Position position) {
      if (state == CellState::White) {
        return sum + 1;
      } else {
        return sum;
      }
    });
    int32_t black_discs =  state.getBoard().getMetric([](int32_t sum, CellState state, Position position) {
      if (state == CellState::White) {
        return sum + 1;
      } else {
        return sum;
      }
    });
    int32_t total_sum = white_discs + black_discs;
    int32_t disc_diff = white_discs - black_discs;
	
	if (total_sum >= END_OF_GAME_THRESHOLD) {
		return disc_diff;
    } else if (total_sum >= NEAR_END_OF_GAME) {
	  int32_t corner_heuristic = StateHelpers::cornerHeuristic(state);
      return disc_diff + corner_heuristic;
    } else {
      int32_t weight_heuristic = state.getBoard().getMetric([&](int32_t sum, CellState state, Position position) {
        return sum + static_cast<int>(state) * WEIGHTS[position.getRow() - 1][position.getColumn() - 'A'];
      });
      int32_t around_heuristic = state.getBoard().getMetric([&](int32_t sum, CellState cellState, Position position) {
        if (cellState == CellState::Empty) {
          return sum;
        } else {
          return sum + StateHelpers::calculateEmptyDiscsAround(state, position);
        }
      });
      return disc_diff + weight_heuristic + around_heuristic;
    }
  }

  int32_t StateHelpers::calculateEmptyDiscsAround(const State &state, Position position) {
    std::function<int32_t (char, unsigned int)> empty = [state](char column, unsigned int row) {
      if (Position::isPossible(column, row) && state.getBoard().getCellState(Position(column, row)) == CellState::Empty) {
        return 1;
      } else {
        return 0;
      }
    };
    unsigned int row = position.getRow();
    char col = position.getColumn();
    int32_t empty_cells = -(empty(col, row - 1) +
      empty(col, row + 1) +
      empty(col - 1, row) +
      empty(col + 1, row) +
      empty(col - 1, row - 1) +
      empty(col - 1, row + 1) +
      empty(col + 1, row - 1) +
      empty(col + 1, row + 1));
    if (empty_cells == 0) {
      empty_cells = 2;
    }
    return empty_cells * static_cast<int>(state.getBoard().getCellState(position));     
  }
  
  int32_t StateHelpers::cornerHeuristic(const State &state) {
	const int CORNER_WEIGHT = 20;
    int32_t corners = static_cast<int>(state.getBoard().getCellState(Position('A', 1))) +
		static_cast<int>(state.getBoard().getCellState(Position('A', 8))) +
		static_cast<int>(state.getBoard().getCellState(Position('H', 1))) +
		static_cast<int>(state.getBoard().getCellState(Position('H', 8)));
	return corners * CORNER_WEIGHT;
  }
}