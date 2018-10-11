#include "reversi/frontend/ReversiSession.h"
#include <algorithm>
#include <thread>
#include <iostream>

namespace Reversi::Frontend {

  static std::size_t THREAD_POOL_CAPACITY = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 1;

  DefaultReversiSession::DefaultReversiSession()
    : threads(THREAD_POOL_CAPACITY) {}

  DefaultReversiSession::DefaultReversiSession(const State &state)
    : engine(state), threads(THREAD_POOL_CAPACITY) {}

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
    std::thread thread([&]() {
      BoardReduceFunction reduce = [](int32_t sum, CellState state, Position position) {
        return sum + static_cast<int>(state);
      };
      Strategy strat = {reduce, reduce};
      Node root(state);
      auto move = root.build(5, strat, this->threads);
      if (move && move.value().first) {
        this->engine.receiveEvent(PlayerMove(state.getPlayer(), move.value().first.value()));
      }
    });
    thread.detach();
  }
}