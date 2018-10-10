#include "reversi/frontend/ReversiSession.h"
#include <algorithm>

namespace Reversi::Frontend {

  DefaultReversiSession::DefaultReversiSession() {}

  DefaultReversiSession::DefaultReversiSession(const State &state)
    : engine(state) {}

  GameEngine &DefaultReversiSession::getEngine() {
    return this->engine;
  }

  const State &DefaultReversiSession::getState() const {
    return this->engine.getState();
  }

  void ReversiHumanHumanSession::onClick(Position position) {
    this->engine.receiveEvent(PlayerMove(this->getState().getPlayer(), position));
  }

  ReversiHumanAISession::ReversiHumanAISession(Player human)
    : human(human) {
    this->engine.addEventListener(this->listener);
    this->listener.setCallback([&](const State &state) {
      if (state.getPlayer() == invertPlayer(this->human)) {
        this->aiTurn(state);
      }
    });
  }

  void ReversiHumanAISession::onClick(Position position) {
    if (this->getState().getPlayer() == this->human) {
      this->engine.receiveEvent(PlayerMove(this->getState().getPlayer(), position));
    }
  }

  void ReversiHumanAISession::aiTurn(const State &state) {
    BoardReduceFunction reduce = [](int32_t sum, CellState state, Position position) {
      return sum + static_cast<int>(state);
    };
    Strategy strat = {reduce, reduce};
    Node root(state);
    auto move = root.build(7, strat);
    if (move && move.value().first) {
      this->engine.receiveEvent(PlayerMove(state.getPlayer(), move.value().first.value()));
    }
  }
}