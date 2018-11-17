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
#include "reversi/engine/Time.h"

namespace Reversi {

  static const std::size_t THREAD_POOL_CAPACITY = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 1;

  AIPlayer::AIPlayer(Player player, unsigned int difficulty, GameEngine &engine, bool wait)
    : player(player), difficulty(difficulty), threads(THREAD_POOL_CAPACITY), engine(engine), active(false), wait(wait), randomized(false) {
    this->engine.addEventListener(*this);
  }

  AIPlayer::~AIPlayer() {
    this->engine.removeEventListener(*this);
  }

  Player AIPlayer::getPlayer() const {
    return this->player;
  }

  unsigned int AIPlayer::getDifficulty() const {
    return this->difficulty;
  }

  void AIPlayer::setDifficulty(unsigned int diff) {
    this->difficulty = diff;
  }

  bool AIPlayer::isActive() const {
    return this->active.load();
  }

  void AIPlayer::setRandomized(bool rand) {
    this->randomized = rand;
  }

  void AIPlayer::makeMove() {
    const State &state = this->engine.getState();
    if (state.getPlayer() == this->player) {
      this->aiTurn(state);
    }
  }

  void AIPlayer::receiveEvent(const State &state) {
    if (state.getPlayer() == this->player && !wait) {
      this->aiTurn(state);
    }
  }

  void AIPlayer::aiTurn(const State &state) {
    this->active = true;
    Logger::log("AI", [&](auto &out)  {
      out << "AI starts lookup. Maximal search depth is " << this->difficulty;
    });
    auto duration = TimeUtils::stopwatches();
    std::thread thread([this, duration, state]() {
      std::function<int32_t (const State &)> reduce = StateHelpers::assessState;
      Strategy strat = {reduce, reduce};
      Node root(state);
      auto move = root.build(this->difficulty, strat, this->threads, this->randomized);
      this->active = false;
      duration();
      auto realDuration = duration().count();
      Logger::log("AI", [&](auto &out) {
        std::size_t node_count = root.getSubNodeCount();
        out << "AI finished lookup after " << realDuration << " microseconds; traversed " << node_count << " nodes";
      });
      if (move && move.value().move) {
        this->engine.receiveEvent(PlayerMove(state.getPlayer(), move.value().move.value()));
      } else {
        std::vector<Position> moves;
        state.getBoard().getMoves(moves, this->player);
      }
    });
    thread.detach();
  }
}