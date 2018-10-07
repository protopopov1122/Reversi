#ifndef REVERSI_FRONTEND_FRAME_H_
#define REVERSI_FRONTEND_FRAME_H_

#include "reversi/frontend/base.h"
#include "reversi/frontend/BoardModel.h"
#include "reversi/frontend/BoardController.h"
#include <functional>

namespace Reversi::Frontend {

  class ReversiModelController : public BoardModel, public BoardController {
   public:
    ReversiModelController();

    void onUpdate(std::function<void()>);
    State &getState();
    void setState(const State &);

    void onClick(Position) override;
    const Board &getBoard() override;
   private:
    std::function<void()> update;
    State state;
  };

  class ReversiFrame : public wxFrame {
   public:
    ReversiFrame(std::string);
   private:
    ReversiModelController model;
  };
}

#endif