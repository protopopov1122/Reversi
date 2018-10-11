#ifndef REVERSI_FRONTEND_FRAME_H_
#define REVERSI_FRONTEND_FRAME_H_

#include "reversi/frontend/base.h"
#include "reversi/frontend/ReversiSession.h"
#include <functional>

namespace Reversi::Frontend {

  class ReversiFrame : public wxFrame {
   public:
    ReversiFrame(std::string);
   private:
    ReversiAIAISession session;
    FunctionEventListener<State> updateListener;
  };
}

#endif