#include "reversi/engine/Player.h"

namespace Reversi {

  Player invertPlayer(Player player) {
    if (player == Player::White) {
      return Player::Black;
    } else {
      return Player::White;
    }
  }
}