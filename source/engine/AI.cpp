#include "reversi/engine/Engine.h"

namespace Reversi {

  static const std::size_t THREAD_POOL_CAPACITY = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 1;

  AIPlayer::AIPlayer(Player player, unsigned int difficulty, GameEngine &engine, bool wait)
    : player(player), difficulty(difficulty), threads(THREAD_POOL_CAPACITY), engine(engine), active(false), wait(wait) {
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
      auto move = root.build(this->difficulty, strat, this->threads);
      this->active = false;
      if (move && move.value().first) {
        this->engine.receiveEvent(PlayerMove(state.getPlayer(), move.value().first.value()));
      } else {
        std::vector<Position> moves;
        state.getBoard().getMoves(moves, this->player);
      }
    });
    thread.detach();
  }

  int32_t AIPlayer::assessState(const State &state) {
    const int WEIGHTS[8][8] = {
      {20, -3, 11, 8, 8, 11, -3, 20},
    	{-3, -7, -4, 1, 1, -4, -7, -3},
    	{11, -4, 2, 2, 2, 2, -4, 11},
    	{8, 1, 2, -3, -3, 2, 1, 8},
    	{8, 1, 2, -3, -3, 2, 1, 8},
    	{11, -4, 2, 2, 2, 2, -4, 11},
    	{-3, -7, -4, 1, 1, -4, -7, -3},
      {20, -3, 11, 8, 8, 11, -3, 20}
    };
    int32_t total_sum = state.getBoard().getMetric([&](int32_t sum, CellState state, Position position) {
      return sum + static_cast<int>(state) * WEIGHTS[position.getColumn() - 'A'][position.getRow() - 1];
    });

    int32_t white_discs = state.getBoard().getMetric([](int32_t sum, CellState state, Position position) {
      if (state == CellState::White) {
        return sum + 1;
      } else {
        return sum;
      }
    });
    int32_t black_discs = state.getBoard().getMetric([](int32_t sum, CellState state, Position position) {
      if (state == CellState::Black) {
        return sum + 1;
      } else {
        return sum;
      }
    });
    float parity = 100.0f * (white_discs - black_discs) / (white_discs + black_discs);
    std::vector<Position> moves;
    state.getBoard().getMoves(moves, Player::White);
    int32_t white_moves = moves.size();
    moves.clear();
    state.getBoard().getMoves(moves, Player::Black);
    int32_t black_moves = moves.size();
    float mobility = 0;
    if (white_moves != black_moves) {
      mobility = 100.0f * (white_moves - black_moves) / (white_moves + black_moves);
    }

    int32_t white_corners = state.getBoard().getMetric([](int32_t sum, CellState state, Position position) {
      if (state == CellState::White &&
        (position.getColumn() == 'A' || position.getColumn() == 'H') &&
        (position.getRow() == 1 || position.getRow() == 8)) {
        return sum + 1;
      } else {
        return sum;
      }
    });
    int32_t black_corners = state.getBoard().getMetric([](int32_t sum, CellState state, Position position) {
      if (state == CellState::Black &&
        (position.getColumn() == 'A' || position.getColumn() == 'H') &&
        (position.getRow() == 1 || position.getRow() == 8)) {
        return sum + 1;
      } else {
        return sum;
      }
    });

    float corner_heuristic = 0;
    if (white_corners != black_corners) {
      corner_heuristic = 100.0f * (white_corners - black_corners) / (white_corners + black_corners);
    }

    float total = (10 * parity) + (802 * corner_heuristic) + (79 * mobility) + (10 * total_sum);
    return static_cast<int32_t>(total);
  }
}