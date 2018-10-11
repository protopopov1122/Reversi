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

  ReversiHumanHumanSession::ReversiHumanHumanSession() {}

  ReversiHumanHumanSession::ReversiHumanHumanSession(const State &state)
    : DefaultReversiSession::DefaultReversiSession(state) {}

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

  ReversiHumanAISession::ReversiHumanAISession(Player human, const State &state)
    : DefaultReversiSession::DefaultReversiSession(state), human(human) {
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

  ReversiAIAISession::ReversiAIAISession()
    : aiRunning(false) {}

  ReversiAIAISession::ReversiAIAISession(const State &state)
    : DefaultReversiSession::DefaultReversiSession(state), aiRunning(false) {}

  void ReversiAIAISession::onClick(Position position) {
    if (!this->aiRunning.load()) {
      this->aiTurn(this->engine.getState());
    }
  }

  void ReversiAIAISession::aiTurn(const State &state) {
    this->aiRunning = true;
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
      this->aiRunning = false;
    });
    thread.detach();
  }

  std::unique_ptr<DefaultReversiSession> ReversiSessionFactory::createHumanHumanSession() {
    return std::make_unique<ReversiHumanHumanSession>();
  }

  std::unique_ptr<DefaultReversiSession> ReversiSessionFactory::createHumanHumanSession(const State &state) {
    return std::make_unique<ReversiHumanHumanSession>(state);
  }

  std::unique_ptr<DefaultReversiSession> ReversiSessionFactory::createHumanAISession(Player human) {
    return std::make_unique<ReversiHumanAISession>(human);
  }

  std::unique_ptr<DefaultReversiSession> ReversiSessionFactory::createHumanAISession(Player human, const State &state) {
    return std::make_unique<ReversiHumanAISession>(human, state);
  }

  std::unique_ptr<DefaultReversiSession> ReversiSessionFactory::createAIAISession() {
    return std::make_unique<ReversiAIAISession>();
  }

  std::unique_ptr<DefaultReversiSession> ReversiSessionFactory::createAIAISession(const State &state) {
    return std::make_unique<ReversiAIAISession>(state);
  }

}