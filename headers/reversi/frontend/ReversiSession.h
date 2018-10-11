#ifndef REVERSI_FRONTEND_REVERSI_SESSION_H_
#define REVERSI_FRONTEND_REVERSI_SESSION_H_

#include "reversi/frontend/base.h"

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
    FixedThreadPool threads;
  };

  class ReversiHumanHumanSession : public DefaultReversiSession {
   public:
    void onClick(Position) override;
  };

  class ReversiHumanAISession : public DefaultReversiSession {
   public:
    ReversiHumanAISession(Player);
    void onClick(Position) override;
   private:
    void aiTurn(const State &);

    FunctionEventListener<State> listener;
    Player human;
  };
}

#endif