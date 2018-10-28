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

  const std::vector<PlayerMove> &GameEngine::getMoves() const {
    return this->moves;
  }

  void GameEngine::stateUpdated() {
    this->triggerEvent(this->state);
  }

  DefaultGameEngine::DefaultGameEngine() {}

  DefaultGameEngine::DefaultGameEngine(const State &state)
    : GameEngine::GameEngine(state) {}

  void DefaultGameEngine::triggerEvent() {
    this->stateUpdated();
  }

  void DefaultGameEngine::receiveEvent(const PlayerMove &move) {
    Position position('A', 1);
    Player player;
    std::tie(player, position) = move;
    if (this->state.getPlayer() == player && StateHelpers::isMovePossible(this->state, player, position)) {
      this->moves.push_back(move);
      this->state.apply(position);
      if (!StateHelpers::hasMoves(this->state, this->state.getPlayer()) && StateHelpers::hasMoves(this->state, invertPlayer(this->state.getPlayer()))) {
        this->state.next();
      }
      this->stateUpdated();
    }
  }

  bool StateHelpers::hasMoves(const State &state, Player player) {
    std::vector<Position> moves;
    state.getBoard().getMoves(moves, player);
    return !moves.empty();
  }

  bool StateHelpers::isMovePossible(const State &state, Player player, Position position) {
    std::vector<Position> moves;
    state.getBoard().getMoves(moves, player);
    return std::count(moves.begin(), moves.end(), position) > 0;
  }

  bool StateHelpers::isGameFinished(const State &state) {
    return !(StateHelpers::hasMoves(state, Player::White) || StateHelpers::hasMoves(state, Player::Black));
  }

  State StateHelpers::getDefaultInitialState() {
    Board board;
    board.putDisc(Position('E', 5), Player::White);
    board.putDisc(Position('D', 4), Player::White);
    board.putDisc(Position('D', 5), Player::Black);
    board.putDisc(Position('E', 4), Player::Black);
    return State(board, Player::Black);
  }
}