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

#ifndef REVERSI_ENGINE_ENGINE_H_
#define REVERSI_ENGINE_ENGINE_H_

#include "reversi/config.h"
#include "reversi/engine/Event.h"
#include "reversi/engine/AI.h"
#include "reversi/engine/Tree.h"
#include <algorithm>
#include <vector>


namespace Reversi {

  using PlayerMove = std::pair<Player, Position>;

  template <bool M>
  class PlayerMoveDiffImpl {};

  class PlayerMoveDiffBase {
   public:
    PlayerMoveDiffBase(Player player, Position move)
      : player(player), move(move) {}

    Player getPlayer () const {
      return this->player;
    }

    Position getMove () const {
      return this->move;
    }

   private:
    Player player;
    Position move;
  };

  template <>
  class PlayerMoveDiffImpl<true> : public PlayerMoveDiffBase {
   public:
    PlayerMoveDiffImpl(Player player, Position move, float metric = 0.0f)
      : PlayerMoveDiffBase(player, move), metric(metric) {}

    float getMetric() const {
      return this->metric;
    }

   private:
    float metric;
  };

  template <>
  class PlayerMoveDiffImpl<false> : public PlayerMoveDiffBase {
   public:
    PlayerMoveDiffImpl(Player player, Position move, float metric = 0.0f)
      : PlayerMoveDiffBase(player, move) {}

    float getMetric() const {
      return 0;
    }
  };

  using PlayerMoveDiff = PlayerMoveDiffImpl<DISPLAY_MOVE_METRIC>;

  class GameEngine : public EventListener<PlayerMove>, public EventSource<State> {
   public:
    GameEngine();
    GameEngine(const State &);
    virtual ~GameEngine() = default;
    const State &getState() const;
    const std::vector<PlayerMoveDiff> &getMoves() const;
    void undoMove(std::size_t = 0);
   protected:
    void stateUpdated();

    State baseState;
    State state;
    std::vector<PlayerMoveDiff> moves;
  };
  
  class StateHelpers {
   public:
    static bool hasMoves(const State &, Player);
    static bool isMovePossible(const State &, Player, Position);
    static bool isGameFinished(const State &);
    static State getDefaultInitialState();
    static int32_t assessState(const State &);
   private:
    static int32_t calculateEmptyDiscsAround(const State &, Position);
	static int32_t cornerHeuristic(const State &);
  };

  class DefaultGameEngine : public GameEngine {
   public:
    DefaultGameEngine();
    DefaultGameEngine(const State &);
    void triggerEvent();
    void receiveEvent(const PlayerMove &) override;
  };
}

#endif