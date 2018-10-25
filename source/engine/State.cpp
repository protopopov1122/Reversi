#include "reversi/engine/State.h"

namespace Reversi {

  State::State()
    : board(), player(Player::Black) {}

  State::State(const Board &board, Player player)
    : board(board), player(player) {}

  State::State(const State &state)
    : board(state.board), player(state.player) {}

  State &State::operator=(const State &state) {
    this->board = state.board;
    this->player = state.player;
    return *this;
  }

  const Board &State::getBoard() const {
    return this->board;
  }

  Player State::getPlayer() const {
    return this->player;
  }

  bool State::apply(Position pos) {
    bool res = this->board.putDisc(pos, this->player);
    if (res) {
      this->player = invertPlayer(this->player);
    }
    return res;
  }

  void State::next() {
    this->player = invertPlayer(this->player);
  }

  bool State::operator<(const State &other) const {
    if (this->board < other.board) {
      return true;
    } else if (this->board == other.board) {
      return static_cast<int>(this->player) < static_cast<int>(other.player);
    } else {
      return false;
    }
  }

  bool State::operator==(const State &other) const {
    return this->board == other.board && this->player == other.player;
  }
}