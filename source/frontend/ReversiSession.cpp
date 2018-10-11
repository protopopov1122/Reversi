#include "reversi/frontend/ReversiSession.h"
#include <algorithm>

namespace Reversi::Frontend {

  static const unsigned int AI_DIFFICULTY = 5;

  DefaultReversiSession::DefaultReversiSession() {}

  DefaultReversiSession::DefaultReversiSession(const State &state)
    : engine(state) {}

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
    : human(human), ai(invertPlayer(human), 5, this->engine) {}

  ReversiHumanAISession::ReversiHumanAISession(Player human, const State &state)
    : DefaultReversiSession::DefaultReversiSession(state), human(human), ai(invertPlayer(human), AI_DIFFICULTY, this->engine) {}

  void ReversiHumanAISession::onClick(Position position) {
    if (this->getState().getPlayer() == this->human) {
      this->engine.receiveEvent(PlayerMove(this->getState().getPlayer(), position));
    }
  }

  ReversiAIAISession::ReversiAIAISession()
    : aiWhite(Player::White, AI_DIFFICULTY, this->engine, true), aiBlack(Player::Black, AI_DIFFICULTY, this->engine, true) {}

  ReversiAIAISession::ReversiAIAISession(const State &state)
    : DefaultReversiSession::DefaultReversiSession(state),
      aiWhite(Player::White, AI_DIFFICULTY, this->engine, true), aiBlack(Player::Black, AI_DIFFICULTY, this->engine, true) {}

  void ReversiAIAISession::onClick(Position position) {
    if (!this->aiWhite.isActive() && !this->aiBlack.isActive()) {
      if (this->engine.getState().getPlayer() == Player::White) {
        this->aiWhite.makeMove();
      } else {
        this->aiBlack.makeMove();
      }
    }
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