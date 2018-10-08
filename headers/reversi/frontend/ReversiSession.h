#ifndef REVERSI_FRONTEND_REVERSI_SESSION_H_
#define REVERSI_FRONTEND_REVERSI_SESSION_H_

#include "reversi/frontend/BoardController.h"
#include "reversi/frontend/BoardModel.h"

namespace Reversi::Frontend {

  class ReversiSession {
   public:
    ReversiSession();
    ReversiSession(const State &);
    virtual ~ReversiSession() = default;

    void setState(const State &);
    const State &getState() const;
    void onStateUpdate(std::function<void()>);
    
    virtual void onClick(Position) = 0;
   protected:
    void stateUpdated() const;

    State state;
   private:
    std::vector<std::function<void()>> update;
  };

  class ReversiSessionBoard : public BoardController, public BoardModel {
   public:
    ReversiSessionBoard(ReversiSession &);

    const Board &getBoard() override;
    void onClick(Position) override;
   private:
    ReversiSession &session;
  };

  class ReversiHumanHumanSession : public ReversiSession {
   public:
    ReversiHumanHumanSession();
    ReversiHumanHumanSession(const State &);

    void onClick(Position) override;
  };

  class ReversiHumanAISession : public ReversiSession {
   public:
    ReversiHumanAISession();
    ReversiHumanAISession(const State &);

    void onClick(Position) override;
   private:
    FixedThreadPool threads;
  };
}

#endif