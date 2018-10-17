#include "reversi/frontend/ReversiSession.h"
#include <algorithm>

namespace Reversi::Frontend {

  const unsigned int DefaultReversiSession::DEFAULT_AI_DIFFICULTY = 5;

  DefaultReversiSession::DefaultReversiSession() {}

  DefaultReversiSession::DefaultReversiSession(const State &state)
    : engine(state) {}
  
  GameEngine &DefaultReversiSession::getEngine() {
    return this->engine;
  }

  const State &DefaultReversiSession::getState() const {
    return this->engine.getState();
  }

  class ReversiHumanHumanSession : public DefaultReversiSession {
   public:
    ReversiHumanHumanSession() {}

    ReversiHumanHumanSession(const State & state)
      : DefaultReversiSession::DefaultReversiSession(state) {}

    void onClick(Position position) override {
      this->engine.receiveEvent(PlayerMove(this->getState().getPlayer(), position));
    }

    bool isCurrentlyProcessing() override {
      return false;
    }

    bool isAI(Player player) override {
      return false;
    }

    bool setAIDifficulty(Player player, unsigned int difficulty) override {
      return false;
    }
  };

  class ReversiHumanAISession : public DefaultReversiSession {
   public:
    ReversiHumanAISession(Player human, unsigned int difficulty)
      : human(human), ai(invertPlayer(human), difficulty, this->engine) {}

    ReversiHumanAISession(Player human, const State &state, unsigned int difficulty)
      : DefaultReversiSession::DefaultReversiSession(state), human(human), ai(invertPlayer(human), difficulty, this->engine) {}

    void onClick(Position position) override {
      if (this->getState().getPlayer() == this->human) {
        this->engine.receiveEvent(PlayerMove(this->getState().getPlayer(), position));
      }
      this->engine.triggerEvent();
    }

    bool isCurrentlyProcessing() override {
      return this->getState().getPlayer() != human && this->ai.isActive();
    }

    bool isAI(Player player) override {
      return player != this->human;
    }

    bool setAIDifficulty(Player player, unsigned int difficulty) override {
      if (player != this->human) {
        this->ai.setDifficulty(difficulty);
        return true;
      } else {
        return false;
      }
    }
   private:
    Player human;
    AIPlayer ai;
  };

  class ReversiAIAISession : public DefaultReversiSession {
   public:
    ReversiAIAISession(unsigned int whiteDifficulty, unsigned int blackDifficulty)
      : aiWhite(Player::White, whiteDifficulty, this->engine, true), aiBlack(Player::Black, blackDifficulty, this->engine, true) {}
   
    ReversiAIAISession(const State &state, unsigned int whiteDifficulty, unsigned int blackDifficulty)
      : DefaultReversiSession::DefaultReversiSession(state),
        aiWhite(Player::White, whiteDifficulty, this->engine, true), aiBlack(Player::Black, blackDifficulty, this->engine, true) {}
   
    void onClick(Position position) override {
      if (!this->aiWhite.isActive() && !this->aiBlack.isActive()) {
        if (this->engine.getState().getPlayer() == Player::White) {
          this->aiWhite.makeMove();
        } else {
          this->aiBlack.makeMove();
        }
        this->engine.triggerEvent();
      }
    }

    bool isCurrentlyProcessing() override {
      return (this->getState().getPlayer() == Player::White && this->aiWhite.isActive()) ||
        (this->getState().getPlayer() == Player::Black && this->aiBlack.isActive());
    }

    bool isAI(Player player) override {
      return true;
    }

    bool setAIDifficulty(Player player, unsigned int difficulty) override {
      if (player == Player::White) {
        this->aiWhite.setDifficulty(difficulty);
      } else {
        this->aiBlack.setDifficulty(difficulty);
      }
      return true;
    }
   private:
    AIPlayer aiWhite;
    AIPlayer aiBlack;
  };

  class LambdaReversiSessionFactory : public ReversiSessionFactory {
   public:
    LambdaReversiSessionFactory(std::function<std::unique_ptr<DefaultReversiSession> (const State &)> build)
      : build(build) {}
    
    std::unique_ptr<DefaultReversiSession> createSession(const State &state) override {
      return this->build(state);
    }
   private:
    std::function<std::unique_ptr<DefaultReversiSession> (const State &)> build;
  };

  std::unique_ptr<ReversiSessionFactory> ReversiSessionFactory::Human_Human = std::make_unique<LambdaReversiSessionFactory>([](const State &state) {
    return std::make_unique<ReversiHumanHumanSession>(state);
  });

  std::unique_ptr<ReversiSessionFactory> ReversiSessionFactory::Human_AI = std::make_unique<LambdaReversiSessionFactory>([](const State &state) {
    return std::make_unique<ReversiHumanAISession>(state.getPlayer(), state, DefaultReversiSession::DEFAULT_AI_DIFFICULTY);
  });

  std::unique_ptr<ReversiSessionFactory> ReversiSessionFactory::AI_AI = std::make_unique<LambdaReversiSessionFactory>([](const State &state) {
    return std::make_unique<ReversiAIAISession>(state, DefaultReversiSession::DEFAULT_AI_DIFFICULTY, DefaultReversiSession::DEFAULT_AI_DIFFICULTY);
  });

}