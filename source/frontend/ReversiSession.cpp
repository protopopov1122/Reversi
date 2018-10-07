#include "reversi/frontend/ReversiSession.h"
#include <algorithm>

namespace Reversi::Frontend {

  ReversiSession::ReversiSession()
    : state() {}

  ReversiSession::ReversiSession(const State &state)
    : state(state) {}

  void ReversiSession::setState(const State &state) {
    this->state = state;
    this->stateUpdated();
  }

  const State &ReversiSession::getState() const {
    return this->state;
  }

  void ReversiSession::onStateUpdate(std::function<void()> listener) {
    this->update.push_back(listener);
  }

  void ReversiSession::stateUpdated() const {
    for (const auto &listener : this->update) {
      listener();
    }
  }

  ReversiSessionBoard::ReversiSessionBoard(ReversiSession &session)
    : session(session) {}

  const Board &ReversiSessionBoard::getBoard() {
    return this->session.getState().getBoard();
  }

  void ReversiSessionBoard::onClick(Position position) {
    this->session.onClick(position);
  }

  ReversiHumanAISession::ReversiHumanAISession()
    : ReversiSession::ReversiSession() {}

  ReversiHumanAISession::ReversiHumanAISession(const State &state)
    : ReversiSession::ReversiSession(state) {}

  void ReversiHumanAISession::onClick(Position pos) {
    if (this->state.getBoard().getCellState(pos) != CellState::Empty) {
      return;
    }
    BoardReduceFunction reduce = [](int32_t sum, CellState state, Position position) {
      return sum + static_cast<int>(state);
    };
    Strategy strat = {reduce, reduce};
    std::vector<Position> moves;
    this->state.getBoard().getMoves(moves, this->state.getPlayer());
    if (std::count(moves.begin(), moves.end(), pos) > 0 && state.apply(pos)) {
      Node root(state);
      auto move = root.build(6, strat);
      // std::cout << root << std::endl;
      if (move && move.value().first) {
        this->state.apply(move.value().first.value());
      } else {
        this->state.next();
      }
      this->stateUpdated();
    }
  }

  ReversiHumanHumanSession::ReversiHumanHumanSession()
    : ReversiSession::ReversiSession() {}

  ReversiHumanHumanSession::ReversiHumanHumanSession(const State &state)
    : ReversiSession::ReversiSession(state) {}

  void ReversiHumanHumanSession::onClick(Position pos) {
    if (this->state.getBoard().getCellState(pos) != CellState::Empty) {
      return;
    }
    std::vector<Position> moves;
    this->state.getBoard().getMoves(moves, this->state.getPlayer());
    if (std::count(moves.begin(), moves.end(), pos) > 0 && state.apply(pos)) {
      this->stateUpdated();
    }
  }
}