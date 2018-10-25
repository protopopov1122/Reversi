#ifndef REVERSI_ENGINE_STATE_H_
#define REVERSI_ENGINE_STATE_H_

#include "reversi/engine/Board.h"

namespace Reversi {

  class State {
   public:
    State();
    State(const Board &, Player);
    State(const State &);
    State &operator=(const State &);

    const Board &getBoard() const;
    Player getPlayer() const;
    bool apply(Position);
    void next();
    bool operator<(const State &) const;
    bool operator==(const State &) const;
   private:
    Board board;
    Player player;
  };
}

#endif