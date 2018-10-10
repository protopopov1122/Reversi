#include "reversi/engine/Engine.h"
#include <iostream>

namespace Reversi {

  GameEngine::GameEngine()
    : state() {}
  
  GameEngine::GameEngine(const State &state)
    : state(state) {}

  void GameEngine::setState(const State &state) {
    this->state = state;
  }

  const State &GameEngine::getState() const {
    return this->state;
  }

  void GameEngine::stateUpdated() {
    this->triggerEvent(this->state);
  }

  DefaultGameEngine::DefaultGameEngine() {}

  DefaultGameEngine::DefaultGameEngine(const State &state)
    : GameEngine::GameEngine(state) {}

  void DefaultGameEngine::receiveEvent(const PlayerMove &move) {
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

  bool DefaultGameEngine::hasMoves(Player player) const {
    std::vector<Position> moves;
    this->state.getBoard().getMoves(moves, player);
    return !moves.empty();
  }

  bool DefaultGameEngine::isMovePossible(Player player, Position position) const {
    std::vector<Position> moves;
    this->state.getBoard().getMoves(moves, player);
    return std::count(moves.begin(), moves.end(), position) > 0;
  }

  bool DefaultGameEngine::isGameFinished() const {
    return this->hasMoves(Player::White) || this->hasMoves(Player::Black);
  }
}