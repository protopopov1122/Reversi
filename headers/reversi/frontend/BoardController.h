#ifndef REVERSI_BOARD_CONTROLLER_H_
#define REVERSI_BOARD_CONTROLLER_H_

#include "reversi/frontend/base.h"

namespace Reversi::Frontend {

  class BoardController {
   public:
    virtual ~BoardController() = default;
    virtual void onClick(Position) = 0;
  };
}

#endif