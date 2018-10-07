#ifndef REVERSI_ENGINE_PLAYER_H_
#define REVERSI_ENGINE_PLAYER_H_

namespace Reversi {

  enum class Player {
    White = 1,
    Black = -1
  };

  Player invertPlayer(Player);
}

#endif