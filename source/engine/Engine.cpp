#include "reversi/engine/Engine.h"

namespace Reversi {
  AbstractGameEngine::AbstractGameEngine()
    : state() {}
  
  AbstractGameEngine::AbstractGameEngine(const State &state)
    : state(state) {}

  void AbstractGameEngine::setState(const State &state) {
    this->state = state;
  }

  const State &AbstractGameEngine::getState() const {
    return this->state;
  }

  void AbstractGameEngine::stateUpdated() {
    this->triggerEvent(this->state);
  }

  void GameEngine::receiveEvent(const PlayerMove &move) {
    Position position('A', 1);
    Player player;
    std::tie(player, position) = move;
    if (this->state.getPlayer() == player && this->isMovePossible(player, position)) {
      this->state.apply(position);
      if (!this->hasMoves(this->state.getPlayer()) && this->hasMoves(invertPlayer(this->state.getPlayer()))) {
        this->state.next();
      }
      this->stateUpdated();
    }
  }

  bool GameEngine::hasMoves(Player player) const {
    std::vector<Position> moves;
    this->state.getBoard().getMoves(moves, player);
    return moves.empty();
  }

  bool GameEngine::isMovePossible(Player player, Position position) const {
    std::vector<Position> moves;
    this->state.getBoard().getMoves(moves, player);
    return std::count(moves.begin(), moves.end(), position) > 0;
  }

  bool GameEngine::isGameFinished() const {
    return this->hasMoves(Player::White) || this->hasMoves(Player::Black);
  }
}