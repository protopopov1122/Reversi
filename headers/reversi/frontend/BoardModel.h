#ifndef REVERSI_FRONTEND_BOARD_MODEL_H_
#define REVERSI_FRONTEND_BOARD_MODEL_H_

#include "reversi/frontend/base.h"


namespace Reversi::Frontend {

  class BoardModel {
   public:
    virtual ~BoardModel() = default;
    virtual const Board &getBoard() = 0;
  };
}


#endif