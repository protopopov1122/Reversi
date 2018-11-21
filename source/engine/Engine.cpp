/*
  Copyright 2018 Jevgenijs Protopopovs

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
  in the documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "reversi/engine/Engine.h"
#include "reversi/engine/Logging.h"
#include <iostream>
#include <cmath>

namespace Reversi {

  GameEngine::GameEngine()
    : state(), baseState() {}
  
  GameEngine::GameEngine(const State &state)
    : state(state), baseState(state) {}

  const State &GameEngine::getState() const {
    return this->state;
  }

  const std::vector<PlayerMoveDiff> &GameEngine::getMoves() const {
    return this->moves;
  }

  void GameEngine::undoMove(std::size_t count) {
    Logger::log("Game", [&](auto &out) {
      out << "Undoing " << count << " moves";
    });
    while (count-- > 0 && !this->moves.empty()) {
      this->moves.pop_back();
    }
    this->state = baseState;
    for (PlayerMoveDiff move : this->moves) {
      this->state.apply(move.getMove());
    }
    this->stateUpdated();
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
      Logger::log("Game", [&](auto &out) {
        out << (this->state.getPlayer() == Player::White ? "White" : "Black") << " move - " << position;
      });
      if constexpr (DISPLAY_MOVE_METRIC) {
        int player = static_cast<int>(this->state.getPlayer());
        int32_t old_metric = this->state.getBoard().getPlainMetric();
        this->state.apply(position);
        int32_t new_metric = this->state.getBoard().getPlainMetric();
        float move_metric = INFINITY;
        if (old_metric != 0) {
          move_metric = player * static_cast<float>(new_metric - old_metric) / abs(old_metric) * 100;
        }
        this->moves.push_back(PlayerMoveDiff(move.first, move.second, move_metric));
      } else {
        this->state.apply(position);
        this->moves.push_back(PlayerMoveDiff(move.first, move.second));
      }
      if (!StateHelpers::hasMoves(this->state, this->state.getPlayer()) && StateHelpers::hasMoves(this->state, invertPlayer(this->state.getPlayer()))) {
        this->state.next();
      }
      this->stateUpdated();
    }
  }
}