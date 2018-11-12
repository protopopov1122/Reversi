#include "reversi/engine/Engine.h"
#include <iostream>
#include <cmath>

namespace Reversi {

  GameEngine::GameEngine()
    : state(), baseState() {}
  
  GameEngine::GameEngine(const State &state)
    : state(state), baseState(state) {}

  const State &GameEngine::getState() const {
    return this->state;
  }

  const std::vector<PlayerMoveDiff> &GameEngine::getMoves() const {
    return this->moves;
  }

  void GameEngine::undoMove(std::size_t count) {
    while (count-- > 0 && !this->moves.empty()) {
      this->moves.pop_back();
    }
    this->state = baseState;
    for (PlayerMoveDiff move : this->moves) {
      this->state.apply(move.move);
    }
    this->stateUpdated();
  }

  void GameEngine::stateUpdated() {
    this->triggerEvent(this->state);
  }

  DefaultGameEngine::DefaultGameEngine() {}

  DefaultGameEngine::DefaultGameEngine(const State &state)
    : GameEngine::GameEngine(state) {}

  void DefaultGameEngine::triggerEvent() {
    this->stateUpdated();
  }

  void DefaultGameEngine::receiveEvent(const PlayerMove &move) {
    Position position('A', 1);
    Player player;
    std::tie(player, position) = move;
    if (this->state.getPlayer() == player && StateHelpers::isMovePossible(this->state, player, position)) {
      if constexpr (DISPLAY_MOVE_METRIC) {
        int player = static_cast<int>(this->state.getPlayer());
        int32_t old_metric = this->state.getBoard().getPlainMetric();
        this->state.apply(position);
        int32_t new_metric = this->state.getBoard().getPlainMetric();
        float move_metric = INFINITY;
        if (old_metric != 0) {
          move_metric = player * static_cast<float>(new_metric - old_metric) / abs(old_metric) * 100;
        }
        this->moves.push_back(PlayerMoveDiff(move.first, move.second, move_metric));
      } else {
        this->state.apply(position);
        this->moves.push_back(PlayerMoveDiff(move.first, move.second));
      }
      if (!StateHelpers::hasMoves(this->state, this->state.getPlayer()) && StateHelpers::hasMoves(this->state, invertPlayer(this->state.getPlayer()))) {
        this->state.next();
      }
      this->stateUpdated();
    }
  }

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
        { 65,  -3, 6, 4, 4, 6,  -3, 65 },
        { -3, -29, 3, 1, 1, 3, -29, -3 },
        { 6,   3, 5, 3, 3, 5,   3,  6 },
        { 4,   1, 3, 1, 1, 3,   1,  4 },
        { 4,   1, 3, 1, 1, 3,   1,  4 },
        { 6,   3, 5, 3, 3, 5,   3,  6 },
        { -3, -29, 3, 1, 1, 3, -29, -3 },
        { 65,  -3, 6, 4, 4, 6,  -3, 65 }
    };
    const int32_t END_OF_GAME_THRESHOLD = 54;

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
}