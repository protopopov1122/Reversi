#include "reversi/engine/Engine.h"

namespace Reversi {

  static const std::size_t THREAD_POOL_CAPACITY = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 1;

  AIPlayer::AIPlayer(Player player, unsigned int difficulty, GameEngine &engine, bool wait)
    : player(player), difficulty(difficulty), threads(THREAD_POOL_CAPACITY), engine(engine), active(false), wait(wait), randomized(false) {
    this->engine.addEventListener(*this);
  }

  AIPlayer::~AIPlayer() {
    this->engine.removeEventListener(*this);
  }

  Player AIPlayer::getPlayer() const {
    return this->player;
  }

  unsigned int AIPlayer::getDifficulty() const {
    return this->difficulty;
  }

  void AIPlayer::setDifficulty(unsigned int diff) {
    this->difficulty = diff;
  }

  bool AIPlayer::isActive() const {
    return this->active.load();
  }

  void AIPlayer::setRandomized(bool rand) {
    this->randomized = rand;
  }

  void AIPlayer::makeMove() {
    const State &state = this->engine.getState();
    if (state.getPlayer() == this->player) {
      this->aiTurn(state);
    }
  }

  void AIPlayer::receiveEvent(const State &state) {
    if (state.getPlayer() == this->player && !wait) {
      this->aiTurn(state);
    }
  }

  void AIPlayer::aiTurn(const State &state) {
    this->active = true;
    std::thread thread([&]() {
      std::function<int32_t (const State &)> reduce = AIPlayer::assessState;
      Strategy strat = {reduce, reduce};
      Node root(state);
      auto move = root.build(this->difficulty, strat, this->threads, this->randomized);
      this->active = false;
      if (move && move.value().move) {
        this->engine.receiveEvent(PlayerMove(state.getPlayer(), move.value().move.value()));
      } else {
        std::vector<Position> moves;
        state.getBoard().getMoves(moves, this->player);
      }
    });
    thread.detach();
  }

  int32_t AIPlayer::assessState(const State &state) {
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
          return sum + AIPlayer::calculateEmptyDiscsAround(state, position);
        }
      });
      return disc_diff + weight_heuristic + around_heuristic;
    }
  }

  int32_t AIPlayer::calculateEmptyDiscsAround(const State &state, Position position) {
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