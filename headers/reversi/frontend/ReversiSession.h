#ifndef REVERSI_FRONTEND_REVERSI_SESSION_H_
#define REVERSI_FRONTEND_REVERSI_SESSION_H_

#include "reversi/frontend/base.h"
#include <atomic>

namespace Reversi::Frontend {

  class ReversiSession {
   public:
    virtual ~ReversiSession() = default;
    virtual const State &getState() const = 0;
    virtual void onClick(Position) = 0;
  };

  class DefaultReversiSession : public ReversiSession {
   public:
    DefaultReversiSession();
    DefaultReversiSession(const State &);

    GameEngine &getEngine();
    const State &getState() const override;
   protected:
    DefaultGameEngine engine;
  };

  class ReversiHumanHumanSession : public DefaultReversiSession {
   public:
    ReversiHumanHumanSession();
    ReversiHumanHumanSession(const State &);
    void onClick(Position) override;
  };

  class ReversiHumanAISession : public DefaultReversiSession {
   public:
    ReversiHumanAISession(Player);
    ReversiHumanAISession(Player, const State &);
    void onClick(Position) override;
   private:
    Player human;
    AIPlayer ai;
  };

  class ReversiAIAISession : public DefaultReversiSession {
   public:
    ReversiAIAISession();
    ReversiAIAISession(const State &);
    void onClick(Position) override;
   private:
    AIPlayer aiWhite;
    AIPlayer aiBlack;
  };

  class ReversiSessionFactory {
   public:
    static std::unique_ptr<DefaultReversiSession> createHumanHumanSession();
    static std::unique_ptr<DefaultReversiSession> createHumanHumanSession(const State &);
    static std::unique_ptr<DefaultReversiSession> createHumanAISession(Player);
    static std::unique_ptr<DefaultReversiSession> createHumanAISession(Player, const State &);
    static std::unique_ptr<DefaultReversiSession> createAIAISession();
    static std::unique_ptr<DefaultReversiSession> createAIAISession(const State &);
  };
}

#endif