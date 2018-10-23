#ifndef REVERSI_FRONTEND_REVERSI_SESSION_H_
#define REVERSI_FRONTEND_REVERSI_SESSION_H_

#include "reversi/frontend/base.h"
#include <atomic>

namespace Reversi::Frontend {

  class ReversiSession {
   public:
    virtual ~ReversiSession() = default;
    virtual const State &getState() const = 0;
    virtual const std::vector<PlayerMove> &getMoves() const = 0;
    virtual void onClick(Position) = 0;
  };

  class DefaultReversiSession : public ReversiSession {
   public:
    DefaultReversiSession();
    DefaultReversiSession(const State &);

    GameEngine &getEngine();
    const State &getState() const override;
    const std::vector<PlayerMove> &getMoves() const override;
    virtual wxWindow *getSettings(wxWindow *, wxWindowID) = 0;
    virtual bool isCurrentlyProcessing() = 0;

    static const unsigned int DEFAULT_AI_DIFFICULTY;
   protected:
    DefaultGameEngine engine;
  };

  class ReversiSessionFactory {
   public:
    virtual ~ReversiSessionFactory() = default;
    virtual std::unique_ptr<DefaultReversiSession> createSession(const State &) = 0;

    static std::unique_ptr<ReversiSessionFactory> Human_Human;
    static std::unique_ptr<ReversiSessionFactory> Human_AI;
    static std::unique_ptr<ReversiSessionFactory> AI_Human;
    static std::unique_ptr<ReversiSessionFactory> AI_AI;
  };
}

#endif