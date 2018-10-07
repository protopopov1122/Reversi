#ifndef REVERSI_ENGINE_PLAYER_H_
#define REVERSI_ENGINE_PLAYER_H_

namespace Reversi {

  enum class Player {
    White,
    Black
  };

  Player invertPlayer(Player);
}

#endif